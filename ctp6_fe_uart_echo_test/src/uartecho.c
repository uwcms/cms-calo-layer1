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

#include <stdio.h>

/*  STDOUT functionality  */
void print(char *str);

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
  print("send interrupt\n");
  printf("sent %i\n", EventData);
  if (EventData % sizeof(uint32_t)) {
    print("ERROR: sent data not word aligned!!!\n");
  }
  cbuffer_deletefront(tx_buffer, EventData / sizeof(uint32_t));
  if (cbuffer_size(tx_buffer)) {
    unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
    printf("sending %i\n", to_send);
    XUartLite_Send(&UartLite, (u8*)&(tx_buffer->data[tx_buffer->pos]), to_send);
    currently_sending = 1;
  } else {
    print("sending completed");
    currently_sending = 0;
  }
}

void RecvHandler(void *CallBackRef, unsigned int EventData) {
  printf("receive interrupt %i\n", EventData);
  if (EventData != sizeof(uint32_t)) {
    print("ERROR: did not receive a whole word!\n");
  }
  cbuffer_push_back(rx_buffer, rx_tmp_buffer);
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));
}

int main(void) {

  print("UART CTP FE echo test\n");

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
    print ("Error: could not initialize UART\n");
      return XST_FAILURE;
  }

  //XUartLite_ResetFifos(&UartLite);

  /*
   * Perform a self-test to ensure that the hardware was built correctly.
   */
  Status = XUartLite_SelfTest(&UartLite);
  if (Status != XST_SUCCESS) {
    print ("Error: self test failed\n");
      return XST_FAILURE;
  }

  /*
   * Connect the UartLite to the interrupt subsystem such that interrupts can
   * occur. This function is application specific.
   */
  Status = SetupInterruptSystem(&UartLite);
  if (Status != XST_SUCCESS) {
    print ("Error: could not setup interrupts\n");
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
  print("Bootstrapping READ\n");
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));

  print("Starting loop\n");

  cbuffer_push_back(tx_buffer, 5);

  print("Sending 'wtf!'\n");
  currently_sending = 1;
  char help[4] = "wtf!";
  unsigned int ret = XUartLite_Send(&UartLite, (u8*)help, 4);
  printf("WTF send complete return: %i\n", ret);

  /* echo received data forever */
  while (1) {
    while (cbuffer_size(rx_buffer) && cbuffer_freespace(tx_buffer)) {
      cbuffer_push_back(tx_buffer, cbuffer_pop_front(rx_buffer));
    }
    if (!currently_sending && cbuffer_size(tx_buffer)) {
      print("Reinitializing SEND\n");
      currently_sending = 1;

      if (XUartLite_IsSending(&UartLite)) {
        print("UART is currently sending\n");
      } else {
        print("UART is not sending\n");
      }

      unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
      printf("Sending bytes %u\n", to_send);
      int ret = XUartLite_Send(&UartLite, 
          (u8*)&(tx_buffer->data[tx_buffer->pos]), to_send);
      printf("Sent bootstrap, return code: %i\n", ret);
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

  print("setup interrupts okay\n");

  return XST_SUCCESS;
}
