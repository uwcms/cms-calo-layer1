/*
 * Minimal test of inter-FPGA UART communication on the oRSC backend.
 *
 * Author: Tapas R. Sarangi, UW Madison
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
#include "i2cutils.h"
#include "xio.h"

//#include <stdio.h>

/*  STDOUT functionality  */

#define printf xil_printf
#define print xil_printf

#define UARTLITE_DEVICE_ID      XPAR_UARTLITE_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define UARTLITE_INT_IRQ_ID     XPAR_INTC_0_UARTLITE_0_VEC_ID

#define TEST_BUFFER_SIZE        100

/************************** Variable Definitions *****************************/

XUartLite UartLite;            /* The instance of the UartLite Device */

XIntc InterruptController;     /* The instance of the Interrupt Controller */


// application input/output buffers 
/* static CircularBuffer* tx_buffer; */
/* static CircularBuffer* rx_buffer; */
/* static volatile uint32_t rx_tmp_buffer; */
/* static volatile int currently_sending = 0; */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following buffers are used in this example to send and receive data
 * with the UartLite.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];
u8 ReceiveBuffer[TEST_BUFFER_SIZE];

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
static volatile int TotalReceivedCount;
static volatile int TotalSentCount;


int SetupInterruptSystem(XUartLite *UartLitePtr);

void SendHandler(void *CallBackRef, unsigned int EventData) 
{
  TotalSentCount = EventData;
}

void RecvHandler(void *CallBackRef, unsigned int EventData) 
{
  TotalReceivedCount = EventData;
	
}

int main(void) {

  XIo_Out32(0x10008044,0x1);  // UnReset Clock A
  XIo_Out32(0x10008048,0x1);  // UnReset Clock C

  init_platform();
  init_DS25CP104();

  /// Delay the signals to sync them later
  int i1;
  for(i1=0; i1<1000000; i1++);

  init_SI5324A();
  check_SI5324A();
  init_SI5324C();
  check_SI5324C();
  

  setup_tracer((uint32_t*)0x7FF0, 3); 
  
  int Status;
  u16 DeviceId = UARTLITE_DEVICE_ID;     

  LOG_INFO("wtf1\n");
  set_trace_flag(1);

  /*
   * Initialize the UartLite driver so that it's ready to use.
   */
  Status = XUartLite_Initialize(&UartLite, DeviceId);
  if (Status != XST_SUCCESS) {
    LOG_ERROR ("Error: could not initialize UART\n");
      return XST_FAILURE;
  }
  LOG_INFO("wtf2\n");
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
  LOG_INFO("wtf3 - help\n");
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
  LOG_INFO("wtf4\n");
  set_trace_flag(4);

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

  int Index;
  for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
    SendBuffer[Index] = Index;
    ReceiveBuffer[Index] = 0;
  }

  set_trace_flag(7);

  XUartLite_Recv(&UartLite, ReceiveBuffer, TEST_BUFFER_SIZE);
  XUartLite_Send(&UartLite, SendBuffer, TEST_BUFFER_SIZE);

  set_trace_flag(8);
  while ((TotalReceivedCount != TEST_BUFFER_SIZE) ||
	 (TotalSentCount != TEST_BUFFER_SIZE)) {
    xil_printf("TRCount= %d : TSCount= %d \r\n", TotalReceivedCount, TotalSentCount);
  }
  set_trace_flag(9);

  for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
    if (ReceiveBuffer[Index] != SendBuffer[Index]) {
      xil_printf("XST_FAILURE... \r\n");
      return XST_FAILURE;
    }
  }
  set_trace_flag(10);
  xil_printf("XST_SUCCESS... \r\n");
  return XST_SUCCESS;

}

int SetupInterruptSystem(XUartLite *UartLitePtr) {

  int Status;


  /*
   * Initialize the interrupt controller driver so that it is ready to
   * use.
   */
  LOG_INFO("shiiit 1\n");
  Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
  if (Status != XST_SUCCESS) {
      LOG_INFO("shiiit\n");
    return XST_FAILURE;
  }

  LOG_INFO("wtf5\n");


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

  LOG_INFO("wtf6\n");

  /*
   * Start the interrupt controller such that interrupts are enabled for
   * all devices that cause interrupts, specific real mode so that
   * the UartLite can cause interrupts through the interrupt controller.
   */
  Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }
  LOG_INFO("wtf6\n");

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
