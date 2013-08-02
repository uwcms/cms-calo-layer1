/*
 * Serve IPBus transactions over UART on the CTP6 front end.
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
#include "xio.h"

// IPBUS functionality
#include "packethandler.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "macrologger.h"

#define UARTLITE_DEVICE_ID      XPAR_UARTLITE_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define UARTLITE_INT_IRQ_ID     XPAR_INTC_0_UARTLITE_0_VEC_ID

#define MIN(a,b) (((a)<(b))?(a):(b))

/************************** Variable Definitions *****************************/

XUartLite UartLite;            /* The instance of the UartLite Device */

XIntc InterruptController;     /* The instance of the Interrupt Controller */


// application input/output buffers 
static CircularBuffer* tx_buffer;
static CircularBuffer* rx_buffer;
static volatile uint32_t rx_tmp_buffer;
static volatile int currently_sending = 0;

int SetupInterruptSystem(XUartLite *UartLitePtr);

// For readout with the debugger
#define TX_SIZE_WORD    0xF0000004
#define RX_SIZE_WORD    0xF0000008
#define BYTES_IN_WORD   0xF000000C
#define BYTES_OUT_WORD  0xF0000010
#define CYCLE_WORD      0xF0000014

void SendHandler(void *CallBackRef, unsigned int EventData) {
  // fix me use         XUartLite_DisableInterrup
  XUartLite_DisableInterrupt(&UartLite);
  // delete the bytes which were sent previously
  if (EventData % sizeof(uint32_t)) {
    LOG_ERROR("ERROR: sent data not word aligned!!!");
  }
  cbuffer_deletefront(tx_buffer, EventData / sizeof(uint32_t));
  if (cbuffer_size(tx_buffer)) {
    unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
    to_send = MIN(to_send, 16);
    XUartLite_Send(&UartLite, (u8*)&(tx_buffer->data[tx_buffer->pos]), to_send);
    currently_sending = 1;
  } else {
    currently_sending = 0;
  }
  uint32_t read = XIo_In32(BYTES_OUT_WORD);
  XIo_Out32(BYTES_OUT_WORD, (EventData >> 2) + read);
  LOG_DEBUG("sent %x", EventData);
  XUartLite_EnableInterrupt(&UartLite);
}

void RecvHandler(void *CallBackRef, unsigned int EventData) {
  XUartLite_DisableInterrupt(&UartLite);
  if (EventData != sizeof(uint32_t)) {
    LOG_ERROR("ERROR: did not receive a whole word!");
  }
  cbuffer_push_back(rx_buffer, rx_tmp_buffer);
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));
  LOG_DEBUG("recv %x", EventData);
  uint32_t written = (EventData >> 2) + XIo_In32(BYTES_IN_WORD);
  XIo_Out32(BYTES_IN_WORD, written);
  XUartLite_EnableInterrupt(&UartLite);
}

static u8* exception_codes;
void ExceptionHandler(void *Data) {
  LOG_ERROR("Caught exception: %x", *((u8*)Data));
}

int main(void) {

  LOG_INFO("UART CTP SPI server");
  XIo_Out32(BYTES_OUT_WORD, 0);
  XIo_Out32(BYTES_IN_WORD, 0);

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
    LOG_ERROR("Error: could not initialize UART");
      return XST_FAILURE;
  }


  /*
   * Perform a self-test to ensure that the hardware was built correctly.
   */
  Status = XUartLite_SelfTest(&UartLite);
  if (Status != XST_SUCCESS) {
    LOG_ERROR("Error: self test failed");
      return XST_FAILURE;
  }

  /*
   * Connect the UartLite to the interrupt subsystem such that interrupts can
   * occur. This function is application specific.
   */
  Status = SetupInterruptSystem(&UartLite);
  if (Status != XST_SUCCESS) {
    LOG_ERROR("Error: could not setup interrupts");
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

  XUartLite_ResetFifos(&UartLite);

  /*
   * Enable the interrupt of the UartLite so that interrupts will occur.
   */
  XUartLite_EnableInterrupt(&UartLite);

  // bootstrap the READ
  XUartLite_Recv(&UartLite, (u8*)&rx_tmp_buffer, sizeof(uint32_t));

  /* serve forever */
  // There can be only one client - the forwarding server demux-es connections.
  Client client;
  client.outputstream = tx_buffer;
  client.inputstream = rx_buffer;
  client.swapbytes = 0;

  LOG_INFO ("Serving memory.");

  LOG_INFO ("Start size: %"PRIx32, cbuffer_size(rx_buffer));

  size_t i = 0;
  while (1) {
    i++;
    XIo_Out32(CYCLE_WORD, i);
    XIo_Out32(TX_SIZE_WORD, cbuffer_size(tx_buffer));
    XIo_Out32(RX_SIZE_WORD, cbuffer_size(rx_buffer));
    ipbus_process_input_stream(&client);
    // if we have data to send and the TX is currently idle, start sending it.
    if (!currently_sending && cbuffer_size(tx_buffer)) {
      XUartLite_DisableInterrupt(&UartLite);
      currently_sending = 1;
      unsigned int to_send = cbuffer_contiguous_data_size(tx_buffer) * sizeof(uint32_t);
      to_send = MIN(to_send, 16);
      XUartLite_Send(&UartLite, (u8*)&(tx_buffer->data[tx_buffer->pos]), to_send);
      XUartLite_EnableInterrupt(&UartLite);
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

  // Give a descriptive error on uncaught exception
  exception_codes = malloc(XIL_EXCEPTION_ID_LAST + 1 - XIL_EXCEPTION_ID_FIRST);

  for (int i = XIL_EXCEPTION_ID_FIRST; i < XIL_EXCEPTION_ID_LAST + 1; ++i) {
    exception_codes[i] = i;
    Xil_ExceptionRegisterHandler(i,
        (Xil_ExceptionHandler)ExceptionHandler,
        &(exception_codes[i]));
  }

  /*
   * Enable exceptions.
   */
  Xil_ExceptionEnable();

  LOG_DEBUG("Setup interrupts okay");

  return XST_SUCCESS;
}
