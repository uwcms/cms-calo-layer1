/**
 * Forward streamed data from VME to SPIStream
 *
 * Author: D. Austin Belknap, UW Madison
 */

#include "platform.h"

#include "xparameters.h"  /* Defined in BSP */
#include "xspi.h"		      /* SPI device driver */
#include "xintc.h"		    /* Interrupt controller device driver */

#include "VMEStream.h"
#include "VMEStreamAddress.h"
#include "spi_stream.h"

/*  STDOUT functionality  */
void print(char *str);


/* Setup VME and SPI Stream structs */
static SPIStream* spi_stream = NULL;
static VMEStream* vme_stream;

/* Input and output buffers */
static CircularBuffer* tx_buffer_vme;
static CircularBuffer* rx_buffer_vme;
static CircularBuffer* tx_buffer_spi;
static CircularBuffer* rx_buffer_spi;


/*  SPI device driver plumbing  */
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define SPI_IRPT_INTR		XPAR_INTC_0_SPI_0_VEC_ID

static XIntc IntcInstance;	 /* The instance of the Interrupt Controller */
static XSpi  SpiInstance;	 /* The instance of the SPI device */

static int SpiSetupIntrSystem(XIntc *IntcInstancePtr, XSpi *SpiInstancePtr,
    u16 SpiIntrId);

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

  tx_buffer_vme = cbuffer_new();
  rx_buffer_vme = cbuffer_new();
  tx_buffer_spi = cbuffer_new();
  rx_buffer_spi = cbuffer_new();

  /* During a transfer,
   * PC_2_ORSC_DATA -> tx_buffer_vme and
   * rx_buffer_vme -> ORSC_2_PC_DATA
   */
  vme_stream = vmestream_initialize_mem(
          tx_buffer_vme, rx_buffer_vme,
          (uint32_t*)ORSC_2_PC_SIZE,
          (uint32_t*)PC_2_ORSC_SIZE,
          (uint32_t*)ORSC_2_PC_DATA,
          (uint32_t*)PC_2_ORSC_DATA,
          VMERAMSIZE);

  spi_stream = spi_stream_init(
      tx_buffer_spi, rx_buffer_spi, 
      DoSpiTransfer, // callback which triggers a SPI transfer
      0);


  int Status;
  XSpi_Config *ConfigPtr; /* Pointer to Configuration data */


  /*
   * Initialize the SPI driver so that it is  ready to use.
   */
  ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
  if (ConfigPtr == NULL) {
    print("Error: could not lookup SPI configuration\n");
    return XST_DEVICE_NOT_FOUND;
  }

  Status = XSpi_CfgInitialize(&SpiInstance, ConfigPtr,
      ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
    print("Error: could not initialize the SPI device\n");
    return XST_FAILURE;
  }

  Status = XSpi_SelfTest(&SpiInstance);
  if (Status != XST_SUCCESS) {
    print("Error: The SPI self test failed.\n");
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

  // Note: to disable interrupt, do: XIntc_Disconnect(&IntcInstance,
  // SPI_IRPT_INTR);

  /* transfer data between VMEStream and SPIStream */
  while (1) {
    /*
     * Perform vmestream data transfer to populate rx_buffer_vme and to move
     * data out of tx_buffer_vme.
     * Then, move data from rx_buffer_vme to tx_buffer_spi
     * Note: data transfers within SPIStream are handled automatically.
     */
    vmestream_transfer_data(vme_stream);
    cbuffer_transfer_data(rx_buffer_vme, tx_buffer_spi);

    /* Move data from rx_buffer_spi to tx_buffer_vme */
    cbuffer_transfer_data(rx_buffer_spi, tx_buffer_vme);
  }

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
    print("Could not initialize interrupt controller.\n");
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
    print("Could not connect interrupt controller to SPI.\n");
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
