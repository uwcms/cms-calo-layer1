#include <stdio.h>
#include "platform.h"

#include "xspi.h"		/* SPI device driver */
#include "xintc.h"		/* Interrupt controller device driver */
#include "xil_exception.h"

#include "spi_stream.h"

/*  STDOUT functionality  */
void print(char *str);

/*  SPI device driver plumbing  */

#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_IRPT_INTR		XPAR_INTC_0_SPI_0_VEC_ID

static XIntc IntcInstance;	 /* The instance of the Interrupt Controller */
static XSpi  SpiInstance;	 /* The instance of the SPI device */

static int SpiSetupIntrSystem(XIntc *IntcInstancePtr, XSpi *SpiInstancePtr,
    u16 SpiIntrId);

/* SPI stream functionality */

static SPIStream* spi_stream = NULL;
/* Input and output buffers */
static CircularBuffer* tx_buffer;
static CircularBuffer* rx_buffer;

void SpiIntrHandler(void *CallBackRef, u32 StatusEvent, u32 ByteCount) {
  u32 error = StatusEvent != XST_SPI_TRANSFER_DONE ? StatusEvent : 0;
  if (spi_stream != NULL) {
    spi_stream_transfer_data(spi_stream, error);
  }
}

void DoSpiTransfer(u8* tx, u8* rx, u16 nbytes) {
  XSpi_Transfer(&SpiInstance, tx, rx, nbytes);
}

int main() {
  // initialize stdout.
  init_platform();

  tx_buffer = cbuffer_new();
  rx_buffer = cbuffer_new();

  spi_stream = spi_stream_init(
      tx_buffer, rx_buffer, 
      DoSpiTransfer, // callback which triggers a SPI transfer
      0);

  print("Master SPI oRSC echo test\n");

  int Status;
  XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

  /*
   * Initialize the SPI driver so that it is  ready to use.
   */
  ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
  if (ConfigPtr == NULL) {
    print ("Error: could not lookup SPI configuration\n");
    return XST_DEVICE_NOT_FOUND;
  }

  Status = XSpi_CfgInitialize(&SpiInstance, ConfigPtr,
      ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
    printf("Error: could not initialize the SPI device\n");
    return XST_FAILURE;
  }

  Status = XSpi_SelfTest(&SpiInstance);
  if (Status != XST_SUCCESS) {
    printf("Error: The SPI self test failed.\n");
    return XST_FAILURE;
  }

  /*
   * Connect the Spi device to the interrupt subsystem such that
   * interrupts can occur. This function is application specific.
   */
  Status = SpiSetupIntrSystem(&IntcInstance, &SpiInstance, SPI_IRPT_INTR);
  if (Status != XST_SUCCESS) {
    print("Error: Could not setup interrupt system.\n");
    return XST_FAILURE;
  }

  /*
   * Set the Spi device as a master.
   */
  Status = XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION);
  if (Status != XST_SUCCESS) {
    print("Error: Could not set as master\n");
    return XST_FAILURE;
  }

  // Go!
  XSpi_Start(&SpiInstance);

  // Note: to disable, do: XIntc_Disconnect(&IntcInstance, SPI_IRPT_INTR);

  return 0;
}

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system such that interrupts can occur
 * for the Spi device. This function is application specific since the actual
 * system may or may not have an interrupt controller. The Spi device could be
 * directly connected to a processor without an interrupt controller.  The
 * user should modify this function to fit the application.
 *
 * @param	IntcInstancePtr is a pointer to the instance of the Intc device.
 * @param	SpiInstancePtr is a pointer to the instance of the Spi device.
 * @param	SpiIntrId is the interrupt Id and is typically
 *		XPAR_<INTC_instance>_<SPI_instance>_VEC_ID value from
 *		xparameters.h
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None
 *
 ******************************************************************************/
static int SpiSetupIntrSystem(XIntc *IntcInstancePtr, XSpi *SpiInstancePtr,
    u16 SpiIntrId) {
  int Status;

  /*
   * Initialize the interrupt controller driver so that it is ready to
   * use.
   */
  Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
  if (Status != XST_SUCCESS) {
    printf("Could not initialize interrupt controller.\n");
    return XST_FAILURE;
  }

  /*
   * Connect a device driver handler that will be called when an interrupt
   * for the device occurs, the device driver handler performs the
   * specific interrupt processing for the device.
   */
  Status = XIntc_Connect(IntcInstancePtr, SpiIntrId,
      (XInterruptHandler) XSpi_InterruptHandler,
      (void *)SpiInstancePtr);
  if (Status != XST_SUCCESS) {
    printf("Could not connect interrupt controller to SPI.\n");
    return XST_FAILURE;
  }

  /*
   * Start the interrupt controller such that interrupts are enabled for
   * all devices that cause interrupts, specific real mode so that
   * the SPI can cause interrupts through the interrupt controller.
   */
  Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    print("Could not enable interrupts.\n");
    return XST_FAILURE;
  }

  /*
   * Enable the interrupt for the SPI device.
   */
  XIntc_Enable(IntcInstancePtr, SpiIntrId);
  return XST_SUCCESS;
}
