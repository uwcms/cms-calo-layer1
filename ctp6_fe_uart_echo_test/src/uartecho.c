/*
 * Minimal test of inter-FPGA UART communication on the CTP6 frontend.
 *
 * Author: Evan K. Friis, UW Madison
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

//#include <stdio.h>

/*  STDOUT functionality  */

#define printf xil_printf
#define print xil_printf

//#define printf(x, ...)
//#define print(x, ...)

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
    LOG_DEBUG("SendHandler _Send  %x\n", to_send);
    currently_sending = 1;
  } else {
    LOG_DEBUG("SendHandler Idling\n");
    currently_sending = 0;
  }
  LOG_DEBUG("SendHandler SENT INTR %x\n", EventData);
}

void RecvHandler(void *CallBackRef, unsigned int EventData) {
  LOG_DEBUG("RecvHandler %x data %x\n", EventData, rx_tmp_buffer);
  if (EventData != sizeof(uint32_t)) {
    LOG_DEBUG("ERROR: did not receive a whole word!\n");
  }
  cbuffer_push_back(rx_buffer, rx_tmp_buffer);
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));
}

int main(void) {

  LOG_INFO("UART CTP FE echo test\n");

  init_platform();

  tx_buffer = cbuffer_new();
  rx_buffer = cbuffer_new();

  int Status;
  u16 DeviceId = UARTLITE_DEVICE_ID;     

  /*
   * Initialize the UartLite driver so that it's ready to use.
   */
  Status = XUartLite_Initialize(&UartLite, DeviceId);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: could not initialize UART\n");
      return XST_FAILURE;
  }

  XUartLite_ResetFifos(&UartLite);

  /*
   * Perform a self-test to ensure that the hardware was built correctly.
   */
  Status = XUartLite_SelfTest(&UartLite);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: self test failed\n");
      return XST_FAILURE;
  }

  /*
   * Connect the UartLite to the interrupt subsystem such that interrupts can
   * occur. This function is application specific.
   */
  Status = SetupInterruptSystem(&UartLite);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: could not setup interrupts\n");
      return XST_FAILURE;
  }

  /*
   * Setup the handlers for the UartLite that will be called from the
   * interrupt context when data has been sent and received, specify a
   * pointer to the UartLite driver instance as the callback reference so
   * that the handlers are able to access the instance data.
   */
  XUartLite_SetSendHandler(&UartLite, SendHandler, &UartLite);
  XUartLite_SetRecvHandler(&UartLite, RecvHandler, &UartLite);

  /*
   * Enable the interrupt of the UartLite so that interrupts will occur.
   */
  XUartLite_EnableInterrupt(&UartLite);

  // bootstrap the READ
  LOG_DEBUG("Bootstrapping READ\n");
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));

  LOG_INFO("Starting loop\n");

  /*  
  LOG_DEBUG("Sending 'wtf!'\n");
  currently_sending = 1;
  char help[4] = "wtf!";
  unsigned int ret = XUartLite_Send(&UartLite, (u8*)help, 4);
  LOG_DEBUG("WTF send complete return: %x\n", ret);
  */

  /* echo received data forever */
  unsigned int heartbeat = 0;
  while (1) {
    if (heartbeat++ % (1 << 8)) {
      //LOG_DEBUG("bump %x\n", heartbeat);
    }
    while (cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer)) {
      uint32_t data = cbuffer_pop_front(rx_buffer);
      //LOG_DEBUG("Echoing data word %x\n", data);
      cbuffer_push_back(tx_buffer, data);
    }
    if (!currently_sending && cbuffer_size(tx_buffer)) {
      LOG_DEBUG("\nREINT SEND\n");
      currently_sending = 1;

      /* 
      if (XUartLite_IsSending(&UartLite)) {
        LOG_DEBUG("UART STAT: sending\n");
      } else {
        LOG_DEBUG("UART STAT: idle\n");
      }
      */

      unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
      u8* output_ptr = (u8*)&(tx_buffer->data[tx_buffer->pos]);
      //LOG_DEBUG("REINIT %x\n", to_send);
      //LOG_DEBUG("SENDADDR %x\n", output_ptr);
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

  LOG_DEBUG("setup interrupts okay\n");

  return XST_SUCCESS;
}
