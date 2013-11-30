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
#include "addr.h"
#include "bytebuffer.h"

#define printf xil_printf
#define print xil_printf

#define UARTLITE_DEVICE_ID      XPAR_UARTLITE_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define UARTLITE_INT_IRQ_ID     XPAR_INTC_0_UARTLITE_0_VEC_ID

#define READ_BUFFER_SIZE        512
#define TEST_BUFFER_SIZE        4

/************************** Variable Definitions *****************************/

XUartLite UartLite;            /* The instance of the UartLite Device */

XIntc InterruptController;     /* The instance of the Interrupt Controller */


/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */

/*
 * The following buffers are used in this example to send and receive data
 * with the UartLite.
 */

u8 SendBuffer[TEST_BUFFER_SIZE]={0}; 
/* u8 ReceiveBuffer[TEST_BUFFER_SIZE]; */

static CircularBuffer* SBuf;
static CircularBuffer* RBuf;
static u8 tmpRBuf;
static volatile int currently_sending=0;

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
static volatile int TotalReceivedCount;
static volatile int TotalSentCount;
static volatile int xst_succ=0;


int SetupInterruptSystem(XUartLite *UartLitePtr);

void SendHandler(void *CallBackRef, unsigned int EventData) 
{
  TotalSentCount = EventData;
}

void RecvHandler(void *CallBackRef, unsigned int EventData) 
{
 /*  TotalReceivedCount = EventData; */
  cbuffer_push_back(RBuf, tmpRBuf);
  XUartLite_Recv(&UartLite, (u8*)&tmpRBuf, sizeof(u8));
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
  SBuf=cbuffer_new();
  RBuf=cbuffer_new();

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

  XUartLite_Recv(&UartLite, (u8*)&tmpRBuf, sizeof(u8));

  while(1){ 
    int Index;
    int cbytes=0;
    int btidx=0;
    while(XIo_In32(REG1_ADDR) >0){  
      uint16_t tsize=(XIo_In16(RAM1_ADDR+8)+2)*sizeof(uint32_t);
      xil_printf("Size: %d\n\r", tsize);
      xil_printf("Sending VME Patterns: ");
      while(cbytes<tsize){
	for (Index = 0; Index <4; Index+=2) {
	  int sbit=btidx*sizeof(uint32_t);
	  u8 *v1=(u8 *)&XIo_In16(RAM1_ADDR+sbit);
	  SendBuffer[Index] =  v1[0];
	  SendBuffer[Index+1] =  v1[1];
	  xil_printf("%x%x", SendBuffer[Index], SendBuffer[Index+1]); /*Need this printf for delay*/
	  btidx++;
	}
	xil_printf("\n\r");
	XUartLite_Send(&UartLite, SendBuffer, sizeof(uint32_t));
	cbytes+=sizeof(uint32_t);
      }
      set_trace_flag(7);
      xil_printf("XST_SUCCESS...\n\r");
      XIo_Out32(REG1_ADDR, 0x0);
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
