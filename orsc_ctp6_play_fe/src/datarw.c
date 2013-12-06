/*
 * Minimal test of inter-FPGA UART communication on the oRSC frontend.
 *
 * Author: Tapas R. Sarangi, Evan K. Friis UW Madison
 *
 * Modified from Xilinx xuartlite_intr_example.c
 *
 */
#include "platform.h"

#include "xparameters.h"        
#include "xuartlite.h"
#include "xintc.h"		
#include "xil_exception.h"
#include "circular_buffer.h"
#include "macrologger.h"
#include "tracer.h"
#include "xio.h"

#define printf xil_printf
#define print xil_printf

#define UARTLITE_DEVICE_ID      XPAR_UARTLITE_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define UARTLITE_INT_IRQ_ID     XPAR_INTC_0_UARTLITE_0_VEC_ID

/************************** Variable Definitions *****************************/

XUartLite UartLite;            /* The instance of the UartLite Device */

XIntc InterruptController;     /* The instance of the Interrupt Controller */


// application input/output buffers 
static CircularBuffer* tx_buffer;
static CircularBuffer* rx_buffer;
static volatile uint32_t rx_tmp_buffer;
static volatile int currently_sending = 0;

int SetupInterruptSystem(XUartLite *UartLitePtr);

void SendHandler(void *CallBackRef, unsigned int EventData) {
  // delete the bytes which were sent previously
  if (EventData % sizeof(uint32_t)) {
    LOG_DEBUG("ERROR: sent data not word aligned!!!\n");
  }
  cbuffer_deletefront(tx_buffer, EventData / sizeof(uint32_t));
  if (cbuffer_size(tx_buffer)) {
    unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
    XUartLite_Send(&UartLite, (u8*)&(tx_buffer->data[tx_buffer->pos]), to_send);
    currently_sending = 1;
  } else {
    currently_sending = 0;
  }
}

void RecvHandler(void *CallBackRef, unsigned int EventData) {
  //LOG_DEBUG("RecvHandler %x data %x\n", EventData, rx_tmp_buffer);
  if (EventData != sizeof(uint32_t)) {
    LOG_DEBUG("ERROR: did not receive a whole word!\n");
  }
  cbuffer_push_back(rx_buffer, rx_tmp_buffer);
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));

}

int main(void) {

  init_platform();

  setup_tracer((uint32_t*)0x7FF0, 3); 
  
  LOG_INFO("UART oRSC FE echo test\n\r");

  tx_buffer = cbuffer_new();
  rx_buffer = cbuffer_new();

  int Status;
  u16 DeviceId = UARTLITE_DEVICE_ID;     

  set_trace_flag(1);

  /*
   * Initialize the UartLite driver so that it's ready to use.
   */
  Status = XUartLite_Initialize(&UartLite, DeviceId);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: could not initialize UART\n");
      return XST_FAILURE;
  }
  set_trace_flag(2);

  XUartLite_ResetFifos(&UartLite);

  /*
   * Perform a self-test to ensure that the hardware was built correctly.
   */
  Status = XUartLite_SelfTest(&UartLite);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: self test failed\n");
      return XST_FAILURE;
  }
  set_trace_flag(3);

  /*
   * Connect the UartLite to the interrupt subsystem such that interrupts can
   * occur. This function is application specific.
   */
  Status = SetupInterruptSystem(&UartLite);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: could not setup interrupts\n");
      return XST_FAILURE;
  }
  //set_trace_flag(4);

  /*
   * Setup the handlers for the UartLite that will be called from the
   * interrupt context when data has been sent and received, specify a
   * pointer to the UartLite driver instance as the callback reference so
   * that the handlers are able to access the instance data.
   */
  XUartLite_SetSendHandler(&UartLite, SendHandler, &UartLite);
  XUartLite_SetRecvHandler(&UartLite, RecvHandler, &UartLite);

  set_trace_flag(5);
  /*
   * Enable the interrupt of the UartLite so that interrupts will occur.
   */
  XUartLite_EnableInterrupt(&UartLite);

  set_trace_flag(6);

  // bootstrap the READ
  LOG_DEBUG("Bootstrapping READ\n");
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));

  set_trace_flag(7);

  /* echo received data forever */
  unsigned int badr=0x10000000;
  unsigned int cnt=0;
  unsigned int plen=0;
  uint32_t data;
  while (1) {
    set_trace_flag(8);
    while(cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer) 
	  && cnt==0) {
      data = cbuffer_pop_front(rx_buffer);
      badr=data;
      cnt++;
    }
    while(cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer)
	  && badr <= 0x10016FFc && cnt==1) {
      data = cbuffer_pop_front(rx_buffer);
      cnt=data;
    }
    while(cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer)
	  && badr <= 0x10016FFc && cnt >1 && plen<cnt){
      data = cbuffer_pop_front(rx_buffer);
      cbuffer_push_back(tx_buffer, data);
      XIo_Out32(badr, data);
      badr+=4;
      plen++;
    }
    if(plen==cnt){
      plen=0;
      cnt=0;
    }
    
    set_trace_flag(9);
    
    if (!currently_sending && cbuffer_size(tx_buffer)) {
      LOG_DEBUG("\nREINT SEND\n");
      currently_sending = 1;

      unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
      u8* output_ptr = (u8*)&(tx_buffer->data[tx_buffer->pos]);
      XUartLite_Send(&UartLite, output_ptr, to_send);
    }
  }

}

int SetupInterruptSystem(XUartLite *UartLitePtr) {

  int Status;


  /*
   * Initialize the interrupt controller driver so that it is ready to
   * use.
   */
  Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }


  /*
   * Connect a device driver handler that will be called when an interrupt
   * for the device occurs, the device driver handler performs the
   * specific interrupt processing for the device.
   */
  Status = XIntc_Connect(&InterruptController, UARTLITE_INT_IRQ_ID,
      (XInterruptHandler)XUartLite_InterruptHandler,
      (void *)UartLitePtr);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  /*
   * Start the interrupt controller such that interrupts are enabled for
   * all devices that cause interrupts, specific real mode so that
   * the UartLite can cause interrupts through the interrupt controller.
   */
  Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  /*
   * Enable the interrupt for the UartLite device.
   */
  XIntc_Enable(&InterruptController, UARTLITE_INT_IRQ_ID);

  /*
   * Initialize the exception table.
   */
  Xil_ExceptionInit();

  /*
   * Register the interrupt controller handler with the exception table.
   */
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
      (Xil_ExceptionHandler)XIntc_InterruptHandler,
      &InterruptController);

  /*
   * Enable exceptions.
   */
  Xil_ExceptionEnable();

  LOG_INFO("setup interrupts okay\n");

  return XST_SUCCESS;
}
