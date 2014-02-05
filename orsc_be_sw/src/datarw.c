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
#include "i2cutils.h"
#include "xio.h"
#include "addr.h"
#include "flashled.h"
#include "ecl_utils.h"

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
static volatile uint32_t tmpRBuf;

/*
 * The following counters are used to determine when the entire buffer has
 * been sent and received.
 */
static volatile int TotalReceivedCount;
static volatile int TotalSentCount;
static volatile int xst_succ=0;

typedef struct {
  u8 delayval;
  int fieldID;
} delay_tbl_entry_t;

delay_tbl_entry_t delay_tbl[8] = {
  {0x00, DELAY_CK160IN},
  {0x10, DELAY_RSTIN},
  {0x10, DELAY_BX0IN},
  {0x10, DELAY_L1AIN},
  {0xA0, DELAY_CK80OUT},
  {0x70, DELAY_RSTOUT},
  {0x70, DELAY_BX0OUT},
  {0x70, DELAY_L1AOUT} 
};

inline void playback(int cbytes, int btidx)
{

  int Index;
  uint16_t tsize = XIo_In16(REG4_ADDR)*sizeof(uint32_t);
  xil_printf("\n\rSize: %d\n\r", tsize);
  xil_printf("Sending VME Patterns: \n\r");
  
  u8 *v1= (u8 *)&XIo_In16(REG1_ADDR);
  u8 *v2= (u8 *)&XIo_In16(REG2_ADDR);
  u8 *v3= (u8 *)&XIo_In16(REG3_ADDR);
  u8 *v4= (u8 *)&XIo_In16(REG4_ADDR);

  xil_printf("\n\rRegisters (1-4) : %x %x %x %x %x %x %x %x \n\r", v1[0], v1[1], v2[0], v2[1], v3[0], v3[1], v4[0], v4[1]);
  
  SendBuffer[0] = v2[0];
  SendBuffer[1] = v2[1];
  SendBuffer[2] =  v3[0];
  SendBuffer[3] =  v3[1];
  XUartLite_Send(&UartLite, SendBuffer, sizeof(uint32_t));
  while(XUartLite_IsSending(&UartLite)){}

  SendBuffer[0] = v4[0];
  SendBuffer[1] = v4[1];
  SendBuffer[2] =  v1[0];
  SendBuffer[3] =  v1[1];
  XUartLite_Send(&UartLite, SendBuffer, sizeof(uint32_t));
  while(XUartLite_IsSending(&UartLite)){}

  while(cbytes<tsize){
    int sbit=btidx*sizeof(uint32_t);
    v1=(u8 *)&XIo_In16(RAM1_ADDR+sbit);
    btidx++;
    XIo_Out16(RAM3_ADDR+sbit, XIo_In16(RAM1_ADDR+sbit));
    int sbit2=btidx*sizeof(uint32_t);
    v2=(u8 *)&XIo_In16(RAM1_ADDR+sbit2);
    SendBuffer[0] =  v1[0];
    SendBuffer[1] =  v1[1];
    SendBuffer[2] =  v2[0];
    SendBuffer[3] =  v2[1];
    /* +++ Write to RAM3 on Spartan in order to cross-check via VME */
    XIo_Out16(RAM3_ADDR+sbit, XIo_In16(RAM1_ADDR+sbit));
    XIo_Out16(RAM3_ADDR+sbit2, XIo_In16(RAM1_ADDR+sbit2));
    btidx++;
    /*xil_printf("\n\r");*/
    XUartLite_Send(&UartLite, SendBuffer, sizeof(uint32_t));
    while(XUartLite_IsSending(&UartLite)){}
    cbytes+=sizeof(uint32_t);
  }
  
  xil_printf("XST_SUCCESS...\n\r");
  XIo_Out32(REG1_ADDR, 0x0);

  v1= (u8 *)&XIo_In16(REG1_ADDR);
  v2= (u8 *)&XIo_In16(REG2_ADDR);
  v3= (u8 *)&XIo_In16(REG3_ADDR);
  v4= (u8 *)&XIo_In16(REG4_ADDR);

  xil_printf("\n\rEmd Registers (1-4) : %x %x %x %x %x %x %x %x \n\r", v1[0], v1[1], v2[0], v2[1], v3[0], v3[1], v4[0], v4[1]);

}

int SetupInterruptSystem(XUartLite *UartLitePtr);

void SendHandler(void *CallBackRef, unsigned int EventData) 
{
  TotalSentCount = EventData;
}

void RecvHandler(void *CallBackRef, unsigned int EventData) 
{
 /*  TotalReceivedCount = EventData; */
  cbuffer_push_back(RBuf, tmpRBuf);
  XUartLite_Recv(&UartLite, (u8*)&tmpRBuf, sizeof(uint32_t));
}

int main(void) {

  /* setvbuf(stdout, NULL, _IONBF, 0);  */

  XIo_Out32(CLOCK_READY, 0x0); /* Turn-off Clock READY signal */

  init_platform();
  ecl_config_init_z(); 
  init_DS25CP104();

  /// Delay the signals to sync them later
  int i1;
  for(i1=0; i1<8; i1++)
    ecl_set_delay_z(delay_tbl[i1].delayval, delay_tbl[i1].fieldID);
  
  ecl_set_mux_fields_z(CLK, 2);
  ecl_set_mux_fields_z(RST, 2);
  ecl_set_mux_fields_z(BX0, 2);
  ecl_set_mux_fields_z(L1A, 2);
  /*   reset_clk_synths_z(); */

  XIo_Out32(0x10008044,0x1);  // UnReset Clock A
  XIo_Out32(0x10008048,0x1);  // UnReset Clock C
  for(i1=0; i1<1000000; i1++);
  init_SI5324A();
  check_SI5324A();
  init_SI5324C();
  check_SI5324C(); 

  XIo_Out32(CLOCK_READY, 0x1); /* Turn-on Clock READY signal to THE FE */

  int Status;
  u16 DeviceId = UARTLITE_DEVICE_ID;     
  SBuf=cbuffer_new();
  RBuf=cbuffer_new();

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
  XUartLite_Recv(&UartLite, (u8*)&tmpRBuf, sizeof(uint32_t));

  while(1){ 

    /* LEDs Keep Flashing when transaction is happening */
    red_turnon();
    for(i1=0; i1<1024; i1++)
      /*xil_printf("...")*/;
    green_turnon();
    for(i1=0; i1<1024; i1++)
      /*xil_printf("...")*/;
    /* --------------------- */
    
    int cbt=0;
    int btx=0;
    while(XIo_In32(REG1_ADDR) == 0x0001){  
      playback(cbt, btx);
    }
    
    while(XIo_In32(REG1_ADDR) == 0x0002){
      int Index;
      for (Index=0; Index<4; Index+=2) {
	u8 *v1=(u8 *)&XIo_In16(REG2_ADDR);
	if(Index>=2)
	  v1=(u8 *)&XIo_In16(REG3_ADDR);
	SendBuffer[Index] =  v1[0];
	SendBuffer[Index+1] =  v1[1];
	xil_printf("\n\r StartAddress:%x%x", SendBuffer[Index], SendBuffer[Index+1]);
      }
      XUartLite_Send(&UartLite, SendBuffer, sizeof(uint32_t));
      while(XUartLite_IsSending(&UartLite)){}
      for (Index=0; Index<4; Index+=4) {
	u8 *v1=(u8 *)&XIo_In16(REG4_ADDR);
	SendBuffer[Index] =  v1[0];
	SendBuffer[Index+1] =  v1[1];
	
	u8 *v2=(u8 *)&XIo_In16(REG1_ADDR);
	SendBuffer[Index+2] =  v2[0];  /* CTRL values for READ/WRITE */
	SendBuffer[Index+3] =  v2[1];
	xil_printf("Size:%x%x , CTRL:%x%x", SendBuffer[Index], SendBuffer[Index+1],
		   SendBuffer[Index+2], SendBuffer[Index+3]);
      }
      XUartLite_Send(&UartLite, SendBuffer, sizeof(uint32_t));

      while(XUartLite_IsSending(&UartLite)){}
      xil_printf("DONE SENDING... Now Receiving...\n\r");

      int b=0;
      while(cbuffer_size(RBuf)){ 
	uint32_t recv = cbuffer_pop_front(RBuf);
	/* xil_printf("data: %x\n\r",recv); */
	uint16_t w1 = (uint16_t)(recv&0x0000ffff);
	uint16_t w2 = (uint16_t)((recv&0xffff0000)>>16);
	XIo_Out16(RAM4_ADDR+b,w1);
	XIo_Out16(RAM4_ADDR+b+4,w2);
	b+=(2*sizeof(uint32_t));
      }
      xil_printf("XST_SUCCESS \n\r");
      XIo_Out32(REG1_ADDR, 0x0);
    }
    

    while(XIo_In32(REG1_ADDR) == 0x0003) {  /* Local Write */
      uint32_t startaddr_write = ((uint32_t)((XIo_In16(REG2_ADDR)&0x0000ffff)<<16) | (XIo_In16(REG3_ADDR)));
      uint32_t startvalue = XIo_In16(REG4_ADDR);
      XIo_Out32(startaddr_write, startvalue);
      xil_printf("\n\r StartAddressForLocalWrite:%x%x\n\r", startaddr_write, startvalue);
      
      xil_printf("XST_SUCCESS \n\r");
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
