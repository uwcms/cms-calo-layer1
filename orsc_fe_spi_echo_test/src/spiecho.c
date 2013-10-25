/*
 * Minimal test of inter-FPGA SPI communication on the oRSC frontend.
 *
 * Author: Evan K. Friis, UW Madison
 *
 * Modified from Xilinx xspi_intr_example.c
 *
 * This program sets up the SPI as slave, then echos all received data.   *
 */

#include "platform.h"

#include "xparameters.h"        /* Defined in BSP */
#include "xspi.h"		/* SPI device driver */
#include "xintc.h"		/* Interrupt controller device driver */
#include "xil_exception.h"

#include "spi_stream.h"
#include "macrologger.h"
#include "tracer.h"

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

// Number of SPI transfers
u32 transfers = 0;
u32 transfers_in_error = 0;

// This function is called by the interrupt service routine at the
// conclusion of each SPI transfer.
void SpiIntrHandler(void *CallBackRef, u32 StatusEvent,
        unsigned int ByteCount) {
  u32 error = StatusEvent != XST_SPI_TRANSFER_DONE ? StatusEvent : 0;
  transfers++;
  set_trace_flag(9);
  if (error) {
    transfers_in_error++;
  }
  if (spi_stream != NULL) {
    spi_stream_transfer_data(spi_stream, error);
  }
}

// This is the format for the callback defined by SPIStream.
// We can't use XSpi_Transfer directly as the callback, since
// we need to pass in the &SpiInstance pointer.
void DoSpiTransfer(u8* tx, u8* rx, u16 nbytes) {
  XSpi_Transfer(&SpiInstance, tx, rx, nbytes);
  set_trace_flag(10);
}

int main() {

  LOG_INFO("\n==> main");

  extern void *__tracer_start;
  setup_tracer((uint32_t*)__tracer_start, 3);
  set_trace_flag(0);

  // initialize stdout.
  init_platform();

  tx_buffer = cbuffer_new();
  rx_buffer = cbuffer_new();

  spi_stream = spi_stream_init(
      tx_buffer, rx_buffer,
      DoSpiTransfer, // callback which triggers a SPI transfer
      0);

  int Status;
  XSpi_Config *ConfigPtr;	/* Pointer to Configuration data */

  /*
   * Initialize the SPI driver so that it is  ready to use.
   */
  ConfigPtr = XSpi_LookupConfig(SPI_DEVICE_ID);
  if (ConfigPtr == NULL) {
    LOG_INFO("Error: lookup conf");
    return XST_DEVICE_NOT_FOUND;
  }
  set_trace_flag(1);

  Status = XSpi_CfgInitialize(&SpiInstance, ConfigPtr,
      ConfigPtr->BaseAddress);
  if (Status != XST_SUCCESS) {
    LOG_INFO("Error: init SPI");
    return XST_FAILURE;
  }
  LOG_INFO("SPI init");
  set_trace_flag(2);

  Status = XSpi_SelfTest(&SpiInstance);
  if (Status != XST_SUCCESS) {
    LOG_INFO("Error: selftest");
    return XST_FAILURE;
  }
  LOG_INFO("Selftest");
  set_trace_flag(3);

  /*
   * Connect the Spi device to the interrupt subsystem such that
   * interrupts can occur. This function is application specific.
   */
  Status = SpiSetupIntrSystem(&IntcInstance, &SpiInstance, SPI_IRPT_INTR);
  if (Status != XST_SUCCESS) {
    LOG_INFO("Error: setup intr");
    return XST_FAILURE;
  }
  LOG_INFO("Setup intr");
  set_trace_flag(4);

  /*
   * Configure the interrupt service routine
   */
  XSpi_SetStatusHandler(&SpiInstance, NULL, SpiIntrHandler);

  LOG_INFO("Interrupts configured");
  set_trace_flag(5);

  // Go!
  XSpi_Start(&SpiInstance);

  // Note: to disable interrupt, do: XIntc_Disconnect(&IntcInstance,
  // SPI_IRPT_INTR);
  
  LOG_INFO("Serve forever");
  set_trace_flag(6);
  while (1) {
    // copy things from the RX buffer to the transmit buffer
    while (cbuffer_freespace(tx_buffer) && cbuffer_size(rx_buffer)) {
      cbuffer_push_back(tx_buffer, cbuffer_value_at(rx_buffer, 0));
      cbuffer_deletefront(rx_buffer, 1);
    }
    set_trace_flag(7);
  }

  LOG_INFO("Goodbye");
  set_trace_flag(8);
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
    LOG_INFO("Could not initialize interrupt controller.");
    return XST_FAILURE;
  }
  LOG_INFO("Init intr");

  /*
   * Connect a device driver handler that will be called when an interrupt
   * for the device occurs, the device driver handler performs the
   * specific interrupt processing for the device.
   */
  Status = XIntc_Connect(IntcInstancePtr, SpiIntrId,
      (XInterruptHandler) XSpi_InterruptHandler,
      (void *)SpiInstancePtr);
  if (Status != XST_SUCCESS) {
    LOG_INFO("Could not connect interrupt controller to SPI.");
    return XST_FAILURE;
  }
  LOG_INFO("Conn intr");

  /*
   * Start the interrupt controller such that interrupts are enabled for
   * all devices that cause interrupts, specific real mode so that
   * the SPI can cause interrupts through the interrupt controller.
   */
  Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    LOG_INFO("Could not enable interrupts.");
    return XST_FAILURE;
  }
  LOG_INFO("Enable intr");

  /*
   * Enable the interrupt for the SPI device.
   */
  XIntc_Enable(IntcInstancePtr, SpiIntrId);

  /*
   * Initialize the exception table.
   */
  Xil_ExceptionInit();

  /*
   * Register the interrupt controller handler with the exception table.
   */
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
      (Xil_ExceptionHandler)XIntc_InterruptHandler,
      IntcInstancePtr);

  /*
   * Enable exceptions.
   */
  Xil_ExceptionEnable();


  return XST_SUCCESS;
}
