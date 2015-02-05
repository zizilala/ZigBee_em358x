/** @file hal/micro/cortexm3/uart.c
 *  @brief EM3XX UART Library.
 *
 * <!-- Copyright 2014 Silicon Laboratories, Inc.                        *80*-->
 */

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "stack/include/error.h"
#include "hal/hal.h"
#include "hal/micro/micro-types.h"

#if (! defined(EMBER_STACK_IP))
#include "stack/include/packet-buffer.h"
#endif

#include "app/util/serial/serial.h"

// Allow some code to be disabled (and flash saved) if
//  a port is unused or in low-level driver mode
// port 0 is VUART
#if (EMBER_SERIAL0_MODE == EMBER_SERIAL_UNUSED)
  #define EM_SERIAL0_ENABLED 0
  #define EM_SER0_PORT_EN(port) (FALSE)
#else
  #define EM_SERIAL0_ENABLED 1
  #define EM_SER0_PORT_EN(port) ((port) == 0)
#endif
#if (EMBER_SERIAL0_MODE == EMBER_SERIAL_LOWLEVEL)
  #error Serial 0 (Virtual Uart) does not support LOWLEVEL mode
#endif

// port 1 is SC1
#if !defined(EMBER_MICRO_HAS_SC1) \
    || (EMBER_SERIAL1_MODE == EMBER_SERIAL_UNUSED) \
    || (EMBER_SERIAL1_MODE == EMBER_SERIAL_LOWLEVEL)
  #define EM_SERIAL1_ENABLED 0
  #define EM_SER1_PORT_EN(port)     (FALSE)
  #define EM_SER1_PORT_FIFO(port)   (FALSE)
  #define EM_SER1_PORT_BUFFER(port) (FALSE)
#else
  #define EM_SERIAL1_ENABLED 1
  #define EM_SER1_PORT_EN(port) ((port) == 1)
  #if     (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
    #define EM_SER1_PORT_FIFO(port)   EM_SER1_PORT_EN(port)
    #define EM_SER1_PORT_BUFFER(port) (FALSE)
  #else//Must be EMBER_SERIAL_BUFFER
    #define EM_SER1_PORT_FIFO(port)   (FALSE)
    #define EM_SER1_PORT_BUFFER(port) EM_SER1_PORT_EN(port)
  #endif
  #ifndef SOFTUART
    #define EM_PHYSICAL_UART
  #endif
#endif

// port 2 is SC3
#if !defined(EMBER_MICRO_HAS_SC3) \
    || (EMBER_SERIAL2_MODE == EMBER_SERIAL_UNUSED) \
    || (EMBER_SERIAL2_MODE == EMBER_SERIAL_LOWLEVEL)
  #define EM_SERIAL2_ENABLED 0
  #define EM_SER2_PORT_EN(port)     (FALSE)
  #define EM_SER2_PORT_FIFO(port)   (FALSE)
  #define EM_SER2_PORT_BUFFER(port) (FALSE)
#else
  #define EM_SERIAL2_ENABLED 1
  #define EM_SER2_PORT_EN(port) ((port) == 2)
  #if     (EMBER_SERIAL2_MODE == EMBER_SERIAL_FIFO)
    #define EM_SER2_PORT_FIFO(port)   EM_SER2_PORT_EN(port)
    #define EM_SER2_PORT_BUFFER(port) (FALSE)
  #else//Must be EMBER_SERIAL_BUFFER
    #define EM_SER2_PORT_FIFO(port)   (FALSE)
    #define EM_SER2_PORT_BUFFER(port) EM_SER2_PORT_EN(port)
  #endif
  #define EM_PHYSICAL_UART
#endif

// port 3 is USB
#if !defined(CORTEXM3_EM35X_USB) \
    || (EMBER_SERIAL3_MODE == EMBER_SERIAL_UNUSED)
  #define EM_SERIAL3_ENABLED 0
  #define EM_SER3_PORT_EN(port) (FALSE)
#else
  #define EM_SERIAL3_ENABLED 1
  #define EM_SER3_PORT_EN(port) ((port) == 3)

  #include "hal/micro/cortexm3/usb/em_usb.h"
  #include "hal/micro/cortexm3/usb/cdc/usbconfig.h"
  #include "hal/micro/cortexm3/usb/cdc/descriptors.h"
  sernum iSerialNumber =
  {
    .len  = 32,
    .type = USB_STRING_DESCRIPTOR,
    .name = {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'},
    .name[ 16 ] = '\0'
  };
#endif




#if defined(EM_ENABLE_SERIAL_FIFO) && defined(EM_ENABLE_SERIAL_BUFFER)
  #define EM_SER_MULTI(expr) (expr)
#else // None/Single/Same UARTs -- no need to runtime check their mode at all
  #define EM_SER_MULTI(expr) (TRUE)
#endif

// TODO
#if EM_SERIAL1_ENABLED && (defined(EMBER_SERIAL1_RTSCTS) || defined(EMBER_SERIAL1_XONXOFF)) && EM_SERIAL2_ENABLED
  #error Flow control is not currently supported when using both physical UARTs
#endif

//State information for RX DMA Buffer operation
typedef struct EmSerialBufferState {
  const int16u fifoSize;
  const int16u rxStartIndexB;
  int16u prevCountA;
  int16u prevCountB;
  boolean waitingForTailA;
  boolean waitingForTailB;
  boolean waitingForInputToB;
  EmberMessageBuffer holdBuf[2];
} EmSerialBufferState;

// to save flash and speed, if there is only one UART in use, make SCx_REG
// and friends decay into a simple register access
#if !EM_SERIAL1_ENABLED || !EM_SERIAL2_ENABLED
  #if EM_SERIAL1_ENABLED
    #define SCx_REG(port, reg) SC1_##reg
    #define INT_SCxCFG(port) INT_SC1CFG
    #define INT_SCxFLAG(port) INT_SC1FLAG
    #define INT_SCx(port) INT_SC1
  #else
    #define SCx_REG(port, reg) SC3_##reg
    #define INT_SCxCFG(port) INT_SC3CFG
    #define INT_SCxFLAG(port) INT_SC3FLAG
    #define INT_SCx(port) INT_SC3
  #endif
#else
  const int32u serialControllerBlockAddresses[] = { SC1_RXBEGA_ADDR, SC3_RXBEGA_ADDR };
  // index into the above array.
  // keep in mind port 1 is SC1 and port 2 is SC3 (this is the tricky one)
  #define SCx_REG(port, reg) (*((volatile int32u *)(                          \
                              (SC1_##reg##_ADDR - SC1_RXBEGA_ADDR)            \
                              + serialControllerBlockAddresses[(port) - 1])))

  const int32u serialControllerIntCfgAddresses[] = { INT_SC1CFG_ADDR, INT_SC3CFG_ADDR };
  #define INT_SCxCFG(port) (*((volatile int32u *)serialControllerIntCfgAddresses[(port) - 1]))

  const int32u serialControllerIntFlagAddresses[]  = { INT_SC1FLAG_ADDR, INT_SC3FLAG_ADDR };
  #define INT_SCxFLAG(port) (*((volatile int32u *)serialControllerIntFlagAddresses[(port) - 1]))

  const int32u serialControllerNvecIntValues[] = { INT_SC1, INT_SC3 };
  #define INT_SCx(port) serialControllerNvecIntValues[(port) - 1]
#endif

#if defined(EZSP_UART) && \
    !defined(EMBER_SERIAL1_RTSCTS) && \
    !defined(EMBER_SERIAL1_XONXOFF)
  #error EZSP-UART requires either RTS/CTS or XON/XOFF flow control!
#endif

#ifdef EMBER_SERIAL1_RTSCTS
  #if EMBER_SERIAL1_MODE != EMBER_SERIAL_BUFFER
  #error "Illegal serial port 1 configuration"
  #endif
#endif

#ifdef EMBER_SERIAL2_RTSCTS
  #if EMBER_SERIAL2_MODE != EMBER_SERIAL_BUFFER
  #error "Illegal serial port 2 configuration"
  #endif
#endif

#ifdef EMBER_SERIAL1_XONXOFF
  #if EMBER_SERIAL1_MODE != EMBER_SERIAL_FIFO
  #error "Illegal serial port 1 configuration"
  #endif

  static void halInternalUart1ForceXon(void); // forward declaration

  static int8s xcmdCount;     // num XONs sent to host, written only by tx isr
                              //-1 means an XOFF was sent last
                              // 0 means ready to rx, but no XON has been sent
                              // n>0 means ready to rx, and n XONs have been sent
  static int8u xonXoffTxByte; // if non-zero, an XON or XOFF byte to send ahead
                              // of tx queue - cleared when byte is sent
  static int8u xonTimer;      // time when last data rx'ed from host, or when
                              // an XON was sent (in 1/4 ticks)

  #define ASCII_XON         0x11  // requests host to pause sending
  #define ASCII_XOFF        0x13  // requests host to resume sending
  #define XON_REFRESH_TIME  8     // delay between repeat XONs (1/4 sec units)
  #define XON_REFRESH_COUNT 3     // max number of repeat XONs to send after 1st

  // Define thresholds for XON/XOFF flow control in terms of queue used values
  // Take into account the 4 byte transmit FIFO
  #if (EMBER_SERIAL1_RX_QUEUE_SIZE == 128)
    #define XON_LIMIT       16    // send an XON
    #define XOFF_LIMIT      96    // send an XOFF
  #elif (EMBER_SERIAL1_RX_QUEUE_SIZE == 64)
    #define XON_LIMIT       8
    #define XOFF_LIMIT      36
  #elif (EMBER_SERIAL1_RX_QUEUE_SIZE == 32)
    #define XON_LIMIT       2
    #define XOFF_LIMIT      8
  #elif (EMBER_SERIAL1_RX_QUEUE_SIZE > 32)
    #define XON_LIMIT       (EMBER_SERIAL1_RX_QUEUE_SIZE/8)
    #define XOFF_LIMIT      (EMBER_SERIAL1_RX_QUEUE_SIZE*3/4)
  #else
    #error "Serial port 1 receive buffer too small!"
  #endif
#endif  // EMBER_SERIAL1_XONXOFF

#ifdef EMBER_SERIAL2_XONXOFF
  #error "XON/XOFF is not supported on port 2"
#endif

#ifdef EMBER_SERIAL3_XONXOFF
  #error "XON/XOFF is not supported on port 3"
#endif

////////////////////// SOFTUART Pin and Speed definitions //////////////////////
//use a logic analyzer and trial and error to determine these values if
//the SysTick time changes or you want to try a different baud
//These were found using EMU 0x50
#define FULL_BIT_TIME_SCLK  0x9C0UL  //9600 baud with FLKC @ SCLK(24MHz)
#define START_BIT_TIME_SCLK 0x138UL  //9600 baud with FLKC @ SCLK(24MHz)
//USE PB6 (GPIO22) for TXD
#define CONFIG_SOFT_UART_TX_BIT() \
  GPIO_PCCFGH = (GPIO_PCCFGH&(~PC6_CFG_MASK)) | (1 << PC6_CFG_BIT)
#define SOFT_UART_TX_BIT(bit)  GPIO_PCOUT = (GPIO_PCOUT&(~PC6_MASK))|((bit)<<PC6_BIT)
//USE PB7 (GPIO23) for RXD
#define CONFIG_SOFT_UART_RX_BIT() \
  GPIO_PCCFGH = (GPIO_PCCFGH&(~PC7_CFG_MASK)) | (4 << PC7_CFG_BIT)
#define SOFT_UART_RX_BIT  ((GPIO_PCIN&PC7)>>PC7_BIT)
////////////////////// SOFTUART Pin and Speed definitions //////////////////////

#if defined(EMBER_SERIAL1_RTSCTS) || defined(EMBER_SERIAL2_RTSCTS)
  void halInternalUartRxCheckRts(int8u port);
#else
  #define halInternalUartRxCheckRts(x) do {} while(0)
#endif

#if defined(EMBER_SERIAL1_RTSCTS)
  // define this for backwards compatibility
  void halInternalUart1RxCheckRts( void )
  {
    halInternalUartRxCheckRts(1);
  }
#endif

// Save flash if ports are undefined
#if defined(EM_PHYSICAL_UART)

  const int8u baudSettings[] = {
    // This table is indexed by the supported BAUD_xxx enum from serial.h.
    // The actual baud rate is encoded in a byte and converted algorithmically
    // into the needed SCx register values based on system clock frequency
    // Here each byte is divided into two 4-bit nibbles 0x<mul><exp> where:
    // baud = <mul> * 100 * 2^<exp> when <exp> is <=10
    //  and = <mul> * 100 * 10^(<exp>-10) when <exp> is >10
    // This allows all supported baud rates (and many others) to be represented.
    0x30, //  0 - BAUD_300    =  3 * 100 * 2^0
    0x60, //  1 - BAUD_600    =  6 * 100 * 2^0
    0x90, //  2 - BAUD_900    =  9 * 100 * 2^0
    0xC0, //  3 - BAUD_1200   = 12 * 100 * 2^0
    0xC1, //  4 - BAUD_2400   = 12 * 100 * 2^1
    0xC2, //  5 - BAUD_4800   = 12 * 100 * 2^2
    0xC3, //  6 - BAUD_9600   = 12 * 100 * 2^3
    0x94, //  7 - BAUD_14400  =  9 * 100 * 2^4
    0xC4, //  8 - BAUD_19200  = 12 * 100 * 2^4
    0x95, //  9 - BAUD_28800  =  9 * 100 * 2^5
    0xC5, // 10 - BAUD_38400  = 12 * 100 * 2^5
    0x5C, // 11 - BAUD_50000  =  5 * 100 * 10^2
    0x96, // 12 - BAUD_57600  =  9 * 100 * 2^6
    0xC6, // 13 - BAUD_76800  = 12 * 100 * 2^6
    0xAC, // 14 - BAUD_100000 = 10 * 100 * 10^2
    0x97, // 15 - BAUD_115200 =  9 * 100 * 2^7
    0x98, // 16 - BAUD_230400 =  9 * 100 * 2^8
    0x99, // 17 - BAUD_460800 =  9 * 100 * 2^9
   #ifdef EMBER_SERIAL_BAUD_CUSTOM
    EMBER_SERIAL_BAUD_CUSTOM, //Hook for custom baud rate, see BOARD_HEADER
   #else
    0x9A, // 18 - BAUD_921600 =  9 * 100 * 2^10
   #endif
  };

#endif // defined(EM_PHYSICAL_UART)

#if EM_SERIAL1_ENABLED
  #if (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
  #endif//(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)

  #if    SLEEPY_IP_MODEM_UART
    //This macro is used to manipulate TxD to avoid glitching it across sleep
    //which can lead to spurrious data or framing errors seen by peer
    #define SC1_TXD_GPIO(gpioCfg, state) do {                                   \
      GPIO_PBCFGL = (GPIO_PBCFGL & ~PB1_CFG_MASK) | ((gpioCfg) << PB1_CFG_BIT); \
      (state) ? (GPIO_PBSET = PB1) : (GPIO_PBCLR = PB1);                        \
    } while (0)
  #else//!SLEEPY_IP_MODEM_UART
    #define SC1_TXD_GPIO(gpioCfg, state) do { } while (0)
  #endif//SLEEPY_IP_MODEM_UART
#endif // EM_SERIAL1_ENABLED

// figure out how many buffer state structs we need
#if (EM_SERIAL1_ENABLED                                                        \
    && EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)                              \
    && (EM_SERIAL2_ENABLED                                                     \
    && EMBER_SERIAL2_MODE == EMBER_SERIAL_BUFFER)
  static EmSerialBufferState serialBufferStates[] = {
    { EMBER_SERIAL1_RX_QUEUE_SIZE,
      (EMBER_SERIAL1_RX_QUEUE_SIZE/2),
      0,
      0,
      FALSE,
      FALSE,
      FALSE,
      { EMBER_NULL_MESSAGE_BUFFER, EMBER_NULL_MESSAGE_BUFFER }
    },
    { EMBER_SERIAL2_RX_QUEUE_SIZE,
      (EMBER_SERIAL2_RX_QUEUE_SIZE/2),
      0,
      0,
      FALSE,
      FALSE,
      FALSE,
      { EMBER_NULL_MESSAGE_BUFFER, EMBER_NULL_MESSAGE_BUFFER }
    }
  };

  #define BUFSTATE(port) (serialBufferStates + (port) - 1)
#elif (EM_SERIAL1_ENABLED                                          \
       && EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
  static EmSerialBufferState serialBufferState = {
    EMBER_SERIAL1_RX_QUEUE_SIZE,
    (EMBER_SERIAL1_RX_QUEUE_SIZE/2),
    0,
    0,
    FALSE,
    FALSE,
    FALSE,
    { EMBER_NULL_MESSAGE_BUFFER, EMBER_NULL_MESSAGE_BUFFER }
  };

  #define BUFSTATE(port) (&serialBufferState)
#elif (EM_SERIAL2_ENABLED                                          \
       && EMBER_SERIAL2_MODE == EMBER_SERIAL_BUFFER)
  static EmSerialBufferState serialBufferState = {
    EMBER_SERIAL2_RX_QUEUE_SIZE,
    (EMBER_SERIAL2_RX_QUEUE_SIZE/2),
    0,
    0,
    FALSE,
    FALSE,
    FALSE,
    { EMBER_NULL_MESSAGE_BUFFER, EMBER_NULL_MESSAGE_BUFFER }
  };

  #define BUFSTATE(port) (&serialBufferState)
#endif

// prototypes
static void halInternalUartTxIsr(int8u port);
static void uartErrorMark(int8u port, int8u errors);
static void halInternalRestartUartDma(int8u port);

// init function for soft uart
#ifdef SOFTUART
static EmberStatus halInternalInitSoftUart()
{
  //make sure the TX bit starts at idle high
  SOFT_UART_TX_BIT(1);
  CONFIG_SOFT_UART_TX_BIT();
  CONFIG_SOFT_UART_RX_BIT();

  return EMBER_SUCCESS;
}
#endif

#ifdef EM_PHYSICAL_UART
static void halInternalInitUartInterrupts(int8u port)
{
  #if defined(EM_ENABLE_SERIAL_FIFO)
    if (EM_SER_MULTI(EM_SER1_PORT_FIFO(port) || EM_SER2_PORT_FIFO(port))) {
      // Make the RX Valid interrupt level sensitive (instead of edge)
      // SC1_INTMODE = SC_SPIRXVALMODE;
      // Enable just RX interrupts; TX interrupts are controlled separately
      INT_SCxCFG(port) |= (INT_SCRXVAL   |
                           INT_SCRXOVF   |
                           INT_SC1FRMERR |
                           INT_SC1PARERR);
      INT_SCxFLAG(port) = 0xFFFF; // Clear any stale interrupts
      INT_CFGSET = INT_SCx(port);
    }
  #endif
  #if defined(EM_ENABLE_SERIAL_BUFFER)
    if (EM_SER_MULTI(EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port))) {
      halInternalRestartUartDma(port);

      // don't do this for port 1 if it's being used for EZSP
      #ifdef EZSP_UART
        if (port != 1) {
      #endif
          INT_SCxCFG(port) |= (INT_SCRXOVF   |
                               INT_SC1FRMERR |
                               INT_SC1PARERR);
      #ifdef EZSP_UART
        }
      #endif

      // The receive side of buffer mode does not require any interrupts.
      // The transmit side of buffer mode requires interrupts, which
      // will be configured on demand in halInternalStartUartTx(), so just
      // enable the top level interrupt for the transmit side.
      INT_SCxFLAG(port) = 0xFFFF; // Clear any stale interrupts
      INT_CFGSET = INT_SCx(port); // Enable top-level interrupt

      #ifdef EMBER_SERIAL1_RTSCTS
        // TODO refactor this into a variable that can be queried at runtime
        if (EM_SER1_PORT_EN(port)) {
          // Software-based RTS/CTS needs interrupts on DMA buffer unloading.
          INT_SCxCFG(port) |= (INT_SCRXULDA | INT_SCRXULDB);
          SCx_REG(port, UARTCFG) |= (SC_UARTFLOW | SC_UARTRTS);
        }
      #endif
      #ifdef EMBER_SERIAL2_RTSCTS
        if (EM_SER2_PORT_EN(port)) {
          // Software-based RTS/CTS needs interrupts on DMA buffer unloading.
          INT_SCxCFG(port) |= (INT_SCRXULDA | INT_SCRXULDB);
          SCx_REG(port, UARTCFG) |= (SC_UARTFLOW | SC_UARTRTS);
        }
      #endif
    }
  #endif
}

// init function for physical UART
static EmberStatus halInternalInitPhysicalUart(int8u port,
                                               SerialBaudRate rate,
                                               SerialParity parity,
                                               int8u stopBits)
{
  int32u tempcfg;

  // set baud rate
  // If rate is one of the BAUD_ settings from serial.h then use
  // its baudSetting[] value from above, otherwise interpret it
  // as a custom baudSetting[] encoded value.
  if (rate < sizeof(baudSettings)/sizeof(*baudSettings)) {
    rate = baudSettings[rate];
  }
  // Convert encoded rate into baud by extracting the <mul> and <exp>
  // nibbles.  <mul> is always multiplied by 100.  For <exp> <= 10,
  // that result is multipled by 2^<exp>; for <exp> > 10 that result
  // is multipled by 10^(<exp>-10).
  tempcfg = (int32u)(rate >> 4) * 100; // multiplier
  rate &= 0x0F; // exponent
  if (rate <= 10) {
    tempcfg <<= rate;
  } else {
    while (rate-- > 10) {
      tempcfg *= 10;
    }
  }
  EmberStatus status = halInternalUartSetBaudRate(port, tempcfg);
  if (status != EMBER_SUCCESS) {
    return status;
  }

  // Default is always 8 data bits irrespective of parity setting,
  // according to Lee, but hack overloads high-order nibble of stopBits to
  // allow user to specify desired number of data bits:  7 or 8 (default).
  if (((stopBits & 0xF0) >> 4) == 7) {
    tempcfg = 0;
  } else {
    tempcfg = SC_UART8BIT;
  }

  // parity bits
  if (parity == PARITY_ODD) {
    tempcfg |= SC_UARTPAR | SC_UARTODD;
  } else if( parity == PARITY_EVEN ) {
    tempcfg |= SC_UARTPAR;
  }

  // stop bits
  if ((stopBits & 0x0F) >= 2) {
    tempcfg |= SC_UART2STP;
  }

  // set all of the above into the config register
  SCx_REG(port, UARTCFG) = tempcfg;

  // put the peripheral into UART mode
  SCx_REG(port, MODE) = SC1_MODE_UART;

  if (EM_SER1_PORT_EN(port)) { // port 1 special glitch-free case 
    SC1_TXD_GPIO(GPIOCFG_OUT_ALT, 1); // Can Assign TxD glitch-free to UART now
  }

  halInternalInitUartInterrupts(port);

  #ifdef EMBER_SERIAL1_XONXOFF
    if (EM_SER1_PORT_EN(port)) { // port 1 XON/XOFF special case
      halInternalUart1ForceXon();
    }
  #endif

  return EMBER_SUCCESS;
}
#endif // EM_PHYSICAL_UART

// initialize USB Virtual COM Port
#if EM_SERIAL3_ENABLED
static EmberStatus halInternalInitUsbVcp(void)
{
  #if defined(CORTEXM3_EM35X_USB)
    halResetWatchdog();

    tokTypeMfgEui64 tokEui64;
    halCommonGetMfgToken((void *)&tokEui64, TOKEN_MFG_EUI_64);

    int8u i = 0;
    int8u j = 0;
    for(j = 0; j<8; j++) {
      iSerialNumber.name[i++] = nibble2Ascii((tokEui64[j]>>4)&0xF);
      iSerialNumber.name[i++] = nibble2Ascii((tokEui64[j]>>0)&0xF);
    }

    USBD_Init(&initstruct);

    // USBD_Read(EP_OUT, receiveBuffer, 50, dataReceivedCallback);

    //It is necessary to wait for the COM port on the host to become
    //active before serial port3 can be used.
    int16u startTime = halCommonGetInt16uMillisecondTick();
    while(USBD_GetUsbState()!=USBD_STATE_CONFIGURED) {
      //Give ourselves a healthy 1 second for a COM port to open.
      if(elapsedTimeInt16u(startTime,
                           halCommonGetInt16uMillisecondTick()) > 1000) {
        return EMBER_SERIAL_INVALID_PORT;
      }
    }

    return EMBER_SUCCESS;
  #endif
}
#endif

#if (EM_SERIAL0_ENABLED ||\
     EM_SERIAL1_ENABLED ||\
     EM_SERIAL2_ENABLED ||\
     EM_SERIAL3_ENABLED)
EmberStatus halInternalUartInit(int8u port,
                                SerialBaudRate rate,
                                SerialParity parity,
                                int8u stopBits)
{
  #if EM_SERIAL0_ENABLED
    if (EM_SER0_PORT_EN(port)) {
      // Nothing special to do since the debug channel handles this
      return EMBER_SUCCESS;
    }
  #endif

  #ifdef SOFTUART
    if (EM_SER1_PORT_EN(port)) {
      return halInternalInitSoftUart();
    }
  #endif

  #ifdef EM_PHYSICAL_UART
    if (EM_SER1_PORT_EN(port) || EM_SER2_PORT_EN(port)) {
      return halInternalInitPhysicalUart(port, rate, parity, stopBits);
    }
  #endif

  #if EM_SERIAL3_ENABLED && defined(CORTEXM3_EM35X_USB)
    if (EM_SER3_PORT_EN(port)) {
      return halInternalInitUsbVcp();
    }
  #endif

  return EMBER_SERIAL_INVALID_PORT;
}
#endif//(!defined(EM_SERIAL0_DISABLED) || !defined(EM_SERIAL1_DISABLED))

#ifdef SOFTUART
//this requires use of the SysTick counter and will destroy interrupt latency!
static void softwareUartTxByte(int8u byte)
{
  int8u i;
  // BIT_TIMEs were determined based on 24 MHz MCU clock.
  // Scale 'em for the actual MCU clock in effect, with rounding.
  // (Because the FCLK might not evenly divide by 1000 or even 500, use quad
  // arithmetic dividing it by 250.)
  int16u fullBitTime = (int16u)(((FULL_BIT_TIME_SCLK
                               * ((int32u)halMcuClockKHz() / 250)) + 48) / 96);

  ATOMIC(
    ST_RVR = fullBitTime; //set the SysTick reload value register
    //enable core clock reference and the counter itself
    ST_CSR = (ST_CSR_CLKSOURCE | ST_CSR_ENABLE);
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time

    //go low for start bit
    SOFT_UART_TX_BIT(0); //go low for start bit
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time

    //loop over all 8 data bits transmitting each
    for (i=0;i<8;i++) {
      SOFT_UART_TX_BIT(byte&0x1); //data bit
      while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time
      byte = (byte>>1);
    }

    SOFT_UART_TX_BIT(1); //stop bit
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time

    //disable SysTick
    ST_CSR = 0;
  )
}
#endif //SOFTUART

void halInternalStartUartTx(int8u port)
{
  #if EM_SERIAL0_ENABLED
    if (EM_SER0_PORT_EN(port)) {
      #if EMBER_SERIAL0_MODE == EMBER_SERIAL_FIFO
        EmSerialFifoQueue *q = (EmSerialFifoQueue *)emSerialTxQueues[0];
        assert(q->tail == 0);
        emDebugSendVuartMessage(q->fifo, q->used);
        q->used = 0;
        q->head = 0;
        return;
      #elif EMBER_SERIAL0_MODE == EMBER_SERIAL_BUFFER
        EmSerialBufferQueue *q = (EmSerialBufferQueue *)emSerialTxQueues[0];
        assert(q->nextByte == NULL);
        emSerialBufferNextMessageIsr(q);
        while (q->nextByte != NULL) {
          emDebugSendVuartMessage(q->nextByte, (q->lastByte-q->nextByte)+1);
          emSerialBufferNextBlockIsr(q,0);
        }
        return;
      #endif
    }
  #endif//!defined(EM_SERIAL0_DISABLED)

  #if EM_SERIAL1_ENABLED && defined(SOFTUART)
    if (EM_SER1_PORT_EN(port)) {
      EmSerialFifoQueue *q = (EmSerialFifoQueue *)emSerialTxQueues[1];
      // Always configure the bit because other operations might have
      // tried to compromise it
      SOFT_UART_TX_BIT(1);
      CONFIG_SOFT_UART_TX_BIT();
      while (q->used > 0) {
        int8u byte = FIFO_DEQUEUE(q, emSerialTxQueueWraps[1]);
        softwareUartTxByte(byte);
      }
      return;
    }
  #endif

  #if defined(EM_PHYSICAL_UART)
    // If the port is configured, go ahead and start transmit
    #if defined(EM_ENABLE_SERIAL_FIFO)
      if ((EM_SER1_PORT_FIFO(port) || EM_SER2_PORT_FIFO(port)) &&
          (SCx_REG(port, MODE) == SC1_MODE_UART)) {
        // Ensure UART TX interrupts are enabled,
        // and call the ISR to send any pending output
        ATOMIC(
          // Enable TX interrupts
          INT_SCxCFG(port) |= (INT_SCTXFREE | INT_SCTXIDLE);
          // Pretend we got a tx interrupt
          halInternalUartTxIsr(port);
        )
        return;
      }
    #endif // defined(EM_ENABLE_SERIAL_FIFO)
    #if defined(EM_ENABLE_SERIAL_BUFFER)
      if ((EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port)) &&
          (SCx_REG(port, MODE) == SC1_MODE_UART)) {
        // Ensure UART TX interrupts are enabled,
        // and call the ISR to send any pending output
        ATOMIC(
          INT_SCxCFG(port) |= (INT_SCTXULDA | INT_SCTXULDB | INT_SCTXIDLE);
          // Pretend we got a tx interrupt
          halInternalUartTxIsr(port);
        )
        return;
      }
    #endif // defined(EM_ENABLE_SERIAL_BUFFER)
  #endif // EM_PHYSICAL_UART

  // why not buffered mode? --RDM
  #if EM_SERIAL3_ENABLED && EMBER_SERIAL3_MODE == EMBER_SERIAL_FIFO
    if (EM_SER3_PORT_EN(port)) {
      #if defined(CORTEXM3_EM35X_USB)
        //Call into the usb.c driver which will operate on serial
        //port 3's Q to transmit data.
        usbTxData();
        return;
      #endif
    }
  #endif // EM_SERIAL3_ENABLED && EMBER_SERIAL3_MODE == EMBER_SERIAL_FIFO
}

void halInternalStopUartTx(int8u port)
{
  // Nothing for port 0 (virtual uart)

  #if defined(EM_PHYSICAL_UART)
    #if defined(EM_ENABLE_SERIAL_FIFO)
      if (EM_SER1_PORT_FIFO(port) || EM_SER2_PORT_FIFO(port)) {
        // Disable TX Interrupts
        INT_SCxCFG(port) &= ~(INT_SCTXFREE | INT_SCTXIDLE);
      }
    #endif // defined(EM_ENABLE_SERIAL_FIFO)
    #if defined(EM_ENABLE_SERIAL_BUFFER)
      if (EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port)) {
        // Ensure DMA operations are complete before shutting off interrupts,
        // otherwise we might miss an important interrupt and cause a
        // packet buffer leak, e.g.
        while (SCx_REG(port, DMACTRL) & (SC_TXLODA | SC_TXLODB)) {}
        while ( !(SCx_REG(port, UARTSTAT) & SC_UARTTXIDLE) ) {}
        // Disable TX Interrupts
        INT_SCxCFG(port) &= ~(INT_SCTXULDA | INT_SCTXULDB | INT_SCTXIDLE);
      }
    #endif // defined(EM_ENABLE_SERIAL_BUFFER)
  #endif // defined(EM_PHYSICAL_UART)
}


//full blocking, no queue overflow issues, can be used in or out of int context
//does not return until character is transmitted.
EmberStatus halInternalForceWriteUartData(int8u port, int8u *data, int8u length)
{
  #if EM_SERIAL0_ENABLED
    if (EM_SER0_PORT_EN(port)) {
      emDebugSendVuartMessage(data, length);
      return EMBER_SUCCESS;
    }
  #endif

  #ifdef SOFTUART
    if (EM_SER1_PORT_EN(port)) {
      //always configure the bit because other operations might have
      //tried to compromise it
      SOFT_UART_TX_BIT(1);
      CONFIG_SOFT_UART_TX_BIT();
      while (length--) {
        SC1_DATA = *data; // why is the soft UART using the physical UART? --RDM
        softwareUartTxByte(*data);
        data++;
      }
      return EMBER_SUCCESS;
    }
  #endif

  #if defined(EM_PHYSICAL_UART)
    //if the port is configured, go ahead and transmit
    if ((EM_SER1_PORT_EN(port) || EM_SER2_PORT_EN(port)) &&
        (SCx_REG(port, MODE) == SC1_MODE_UART)) {
      while (length--) {
        //spin until data register has room for more data
        while (!(SCx_REG(port, UARTSTAT) & SC_UARTTXFREE)) {}
        SCx_REG(port, DATA) = *data;
        data++;
      }

      //spin until TX complete (TX is idle)
      while (!(SCx_REG(port, UARTSTAT) & SC_UARTTXIDLE)) {}

      return EMBER_SUCCESS;
    }
  #endif // defined(EM_PHYSICAL_UART)

  #if EM_SERIAL3_ENABLED
    if (EM_SER3_PORT_EN(port)) {
      #if defined(CORTEXM3_EM35X_USB)
        //This function will block until done sending all the data.
        usbForceTxData(data, length);
        return EMBER_SUCCESS;
      #endif
    }
  #endif // EM_SERIAL3_ENABLED

  return EMBER_SERIAL_INVALID_PORT;
}

// Useful for waiting on serial port characters before interrupts have been
// turned on.
EmberStatus halInternalForceReadUartByte(int8u port, int8u* dataByte)
{
  EmberStatus err = EMBER_SUCCESS;

  #if EM_SERIAL0_ENABLED
    if (EM_SER0_PORT_EN(port)) {
      EmSerialFifoQueue *q = emSerialRxQueues[0];
      ATOMIC(
        if (q->used == 0) {
          WAKE_CORE = WAKE_CORE_FIELD;
        }
        if (q->used > 0) {
          *dataByte = FIFO_DEQUEUE(q, emSerialRxQueueWraps[0]);
        } else {
          err = EMBER_SERIAL_RX_EMPTY;
        }
      )
    }
  #endif // EM_SERIAL0_ENABLED

  #if defined(EM_PHYSICAL_UART)
    #if defined(EM_ENABLE_SERIAL_FIFO)
      if (EM_SER1_PORT_FIFO(port) || EM_SER2_PORT_FIFO(port)) {
        if (SCx_REG(port, UARTSTAT) & SC_UARTRXVAL) {
          *dataByte = (int8u) SCx_REG(port, DATA);
        } else {
          err = EMBER_SERIAL_RX_EMPTY;
        }
      }
    #endif
    #if defined(EM_ENABLE_SERIAL_BUFFER)
      if (EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port)) {
        //When in buffer mode, the DMA channel is active and the RXVALID bit (as
        //used above in FIFO mode) will never get set.  To maintain the DMA/Buffer
        //model of operation, we need to break the conceptual model in this function
        //and make a function call upwards away from the hardware.  The ReadByte
        //function calls back down into halInternalUartRxPump and forces the
        //sequencing of the serial queues and the DMA buffer, resulting in a forced
        //read byte being returned if it is there.
        if (emberSerialReadByte(port, dataByte) != EMBER_SUCCESS) {
          err = EMBER_SERIAL_RX_EMPTY;
        }
      }
    #endif // defined(EM_ENABLE_SERIAL_BUFFER)
  #endif // defined(EM_PHYSICAL_UART)

  return err;
}

// blocks until the text actually goes out
void halInternalWaitUartTxComplete(int8u port)
{
  halResetWatchdog();

  // Nothing to do for port 0 (virtual uart)

  #if defined(EM_PHYSICAL_UART)
  if (EM_SER1_PORT_EN(port) || EM_SER2_PORT_EN(port)) {
      while ( !(SCx_REG(port, UARTSTAT) & SC_UARTTXIDLE) ) {}
      return;
    }
  #endif // defined(EM_PHYSICAL_UART)
}

// Debug Channel calls this ISR to push up data it has received
void halStackReceiveVuartMessage(int8u *data, int8u length)
{
  #if EM_SERIAL0_ENABLED
    EmSerialFifoQueue *q = emSerialRxQueues[0];

    while (length--) {
      //Use (emSerialRxQueueSizes - 1) so that the FIFO never completely fills
      //and the head never wraps around to the tail
      if ((q->used < (emSerialRxQueueSizes[0] - 1))) {
        FIFO_ENQUEUE(q,*data++,emSerialRxQueueWraps[0]);
      } else {
        uartErrorMark(0, EMBER_SERIAL_RX_OVERFLOW);
        return;  // no sense in trying to enqueue the rest
      }
    }
  #endif // EM_SERIAL0_ENABLED
}

#if defined(EM_ENABLE_SERIAL_BUFFER)
static void halInternalRestartUartDma(int8u port)
{
  //Reset the DMA software and restart it.
  EmSerialFifoQueue *q = emSerialRxQueues[port];
  int32u startAddress = (int32u)q->fifo;
  int8u head;
  int8u tail;
  int8u loadA = 0;
  int8u loadB = 0;
  BUFSTATE(port)->prevCountA = 0;
  BUFSTATE(port)->prevCountB = 0;
  BUFSTATE(port)->waitingForTailA = FALSE;
  BUFSTATE(port)->waitingForTailB = FALSE;
  BUFSTATE(port)->waitingForInputToB = FALSE;
  //reload all defaults addresses - they will be adjusted below if needed
  SCx_REG(port, DMACTRL) = SC_RXDMARST;
  SCx_REG(port, RXBEGA) =  startAddress;
  SCx_REG(port, RXENDA) = (startAddress + BUFSTATE(port)->fifoSize/2 - 1);
  SCx_REG(port, RXBEGB) =  (startAddress + BUFSTATE(port)->fifoSize/2);
  SCx_REG(port, RXENDB) = (startAddress + BUFSTATE(port)->fifoSize - 1);

  //adjust buffer addresses as needed and reload available buffers
  if ( q->used != BUFSTATE(port)->fifoSize ) {
    //we can only reload if the FIFO isn't full!
    //the FIFO is not empty or full, figure out what to do:
    //at this point we know we always have to adjust ST_ADDR to the head
    //we need to know which buffer the head is in, and always load that buff
    if ((q->head) < BUFSTATE(port)->rxStartIndexB) {
      SCx_REG(port, RXBEGA) = startAddress + (q->head);
      loadA++;
    } else {
      SCx_REG(port, RXBEGB) = startAddress + (q->head);
      loadB++;
    }
    //check to see if the head and the tail are not in the same buffer
    if((q->tail)/(BUFSTATE(port)->rxStartIndexB)) {
      tail = TRUE;  //Tail in B buffer
    } else {
      tail = FALSE; //Tail in A buffer
    }

    if((q->head)/(BUFSTATE(port)->rxStartIndexB)) {
      head = TRUE;  //Head in B buffer
    } else {
      head = FALSE; //Head in A buffer
    }

    if ( tail != head ) {
      //the head and the tail are in different buffers
      //we need to flag the buffer the tail is in so the Pump function does
      //not try to reenable it until it has been drained like normal.
      if ((q->tail)<BUFSTATE(port)->rxStartIndexB) {
        BUFSTATE(port)->waitingForTailA = TRUE;
      } else {
        BUFSTATE(port)->waitingForTailB = TRUE;
      }
    } else {
      //the head and the tail are in the same buffers
      if (q->used <= BUFSTATE(port)->rxStartIndexB) {
        //The serial FIFO is less no more than half full!
        if (!loadB) {
          //the head is in B, and we're capable of loading A
          //BUT: we can't activate A because the DMA defaults to A first,
          //  and it needs to start using B first to fill from the head
          //  SO, only load A if B hasn't been marked yet for loading.
          loadA++;
        } else {
          //B is loaded and waiting for data, A is being supressed until
          //B receives at least one byte so A doesn't prematurely load and
          //steal bytes meant for B first.
          BUFSTATE(port)->waitingForTailA = TRUE;
          BUFSTATE(port)->waitingForInputToB = TRUE;
        }
        //We can always loadB at this point thanks to our waiting* flags.
        loadB++;
      } else {
        //The serial FIFO is more than half full!
        //Since this case requires moving an end address of a buffer, which
        //severely breaks DMA'ing into a FIFO, we cannot do anything.
        //Doing nothing is ok because we are more than half full anyways,
        //and under normal operation we would only load a buffer when our
        //used count is less than half full.
        //Configure so the Pump function takes over when the serial FIFO drains
        SCx_REG(port, RXBEGA) =  startAddress;
        SCx_REG(port, RXBEGB) =  (startAddress + BUFSTATE(port)->fifoSize/2);
        loadA = 0;
        loadB = 0;
        BUFSTATE(port)->waitingForTailA = TRUE;
        BUFSTATE(port)->waitingForTailB = TRUE;
      }
    }

    //Address are set, flags are set, DMA is ready, so now we load buffers
    if (loadA) {
      SCx_REG(port, DMACTRL) = SC_RXLODA;
    }
    if (loadB) {
      SCx_REG(port, DMACTRL) = SC_RXLODB;
    }
  } else {
    //we're full!!  doh!  have to wait for the FIFO to drain
    BUFSTATE(port)->waitingForTailA = TRUE;
    BUFSTATE(port)->waitingForTailB = TRUE;
  }
}
#endif // defined(EM_ENABLE_SERIAL_BUFFER)

#ifdef EM_PHYSICAL_UART
void halInternalUartRxIsr(int8u port, int16u causes)
{
  #if defined(EM_ENABLE_SERIAL_FIFO)
    if (EM_SER_MULTI(EM_SER1_PORT_FIFO(port) || EM_SER2_PORT_FIFO(port))) {
      EmSerialFifoQueue *q = emSerialRxQueues[port];

      // At present we really don't care which interrupt(s)
      // occurred, just that one did.  Loop reading RXVALID
      // data (loop is necessary for bursty data otherwise
      // we could leave with RXVALID and not get another
      // RXVALID interrupt), processing any errors noted
      // along the way.
      while ( SCx_REG(port, UARTSTAT) & SC_UARTRXVAL ) {
        int8u errors = SCx_REG(port, UARTSTAT) & (SC_UARTFRMERR |
                                       SC_UARTRXOVF  |
                                       SC_UARTPARERR );
        int8u incoming = (int8u) SCx_REG(port, DATA);

        if ( (errors == 0) && (q->used < (emSerialRxQueueSizes[port]-1)) ) {
#ifdef EMBER_SERIAL1_XONXOFF
          if (EM_SER1_PORT_FIFO(port)) {
            // Discard any XON or XOFF bytes received
            if ( (incoming != ASCII_XON) && (incoming != ASCII_XOFF) ) {
              FIFO_ENQUEUE(q, incoming, emSerialRxQueueWraps[port]);
            }
          } else {
            FIFO_ENQUEUE(q, incoming, emSerialRxQueueWraps[port]);
          }
#else
          FIFO_ENQUEUE(q, incoming, emSerialRxQueueWraps[port]);
#endif
        } else {
          // Translate error code
          if ( errors == 0 ) {
            errors = EMBER_SERIAL_RX_OVERFLOW;
            HANDLE_ASH_ERROR(EMBER_COUNTER_ASH_OVERFLOW_ERROR);
          } else if ( errors & SC_UARTRXOVF ) {
            errors = EMBER_SERIAL_RX_OVERRUN_ERROR;
            HANDLE_ASH_ERROR(EMBER_COUNTER_ASH_OVERRUN_ERROR);
          } else if ( errors & SC_UARTFRMERR ) {
            errors = EMBER_SERIAL_RX_FRAME_ERROR;
            HANDLE_ASH_ERROR(EMBER_COUNTER_ASH_FRAMING_ERROR);
          } else if ( errors & SC_UARTPARERR ) {
            errors = EMBER_SERIAL_RX_PARITY_ERROR;
          } else { // unknown
            errors = EMBER_ERR_FATAL;
          }
          uartErrorMark(port, errors);
        }
#ifdef EMBER_SERIAL1_XONXOFF
        if (EM_SER1_PORT_FIFO(port) &&
            (q->used >= XOFF_LIMIT) && (xcmdCount >= 0))  {
          xonXoffTxByte = ASCII_XOFF;
          halInternalStartUartTx(1);
        }
#endif
      } // end of while ( SC1_UARTSTAT & SC1_UARTRXVAL )
    }
  #endif

  #if defined(EM_ENABLE_SERIAL_BUFFER)
    if (EM_SER_MULTI(EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port))) {
    #ifdef EMBER_SERIAL1_RTSCTS
      // TODO this flow control will fail if port 2 is active
      // If RTS is controlled by sw, this ISR is called when a buffer unloads.
      if (causes & (INT_SCRXULDA | INT_SCRXULDB)) {
        // Deassert RTS if the rx queue tail is not in an active DMA buffer:
        // if it is, then there's at least one empty DMA buffer
        if ( !( (emSerialRxQueues[port]->tail < emSerialRxQueueSizes[port]/2) &&
               (SCx_REG(port, DMASTAT) & SC_RXACTA) ) &&
             !( (emSerialRxQueues[port]->tail >= emSerialRxQueueSizes[port]/2)
                && (SCx_REG(port, DMASTAT) & SC_RXACTB) ) ) {
          SCx_REG(port, UARTCFG) &= ~SC_UARTRTS;        // deassert RTS
        }
      #ifdef EZSP_UART
        // TODO fix EZSP_UART
        if ( ( (causes & INT_SCRXULDA) && (SC1_DMASTAT & SC_RXOVFA) ) ||
             ( (causes & INT_SCRXULDB) && (SC1_DMASTAT & SC_RXOVFB) ) ) {
          HANDLE_ASH_ERROR(EMBER_COUNTER_ASH_OVERFLOW_ERROR);
        }
        if ( ( (causes & INT_SCRXULDA) && (SC1_DMASTAT & SC_RXFRMA) ) ||
             ( (causes & INT_SCRXULDB) && (SC1_DMASTAT & SC_RXFRMB) ) ) {
          HANDLE_ASH_ERROR(EMBER_COUNTER_ASH_FRAMING_ERROR);
        }
      #else//!EZSP_UART
        causes &= ~(INT_SCRXULDA | INT_SCRXULDB);
        if (causes == 0) { // if no errors in addition, all done
          return;
        }
      #endif//EZSP_UART
      }
    #endif  //#ifdef EMBER_SERIAL1_RTSCTS
    #ifndef EZSP_UART
    //Load all of the hardware status, then immediately reset so we can process
    //what happened without worrying about new data changing these values.
    //We're in an error condition anyways, so it is ok to have the DMA disabled
    //for a while (less than 80us, while 4 bytes @ 115.2kbps is 350us)
    {
      EmSerialFifoQueue *q = emSerialRxQueues[port];
      int16u status  = SCx_REG(port, DMASTAT);
      int16u errCntA = SCx_REG(port, RXERRA);
      int16u errCntB = SCx_REG(port, RXERRB);
      int32u errorIdx = emSerialRxQueueSizes[port]*2;
      int32u tempIdx;
      int32u startAddress = (int32u)q->fifo;

      //interrupts acknowledged at the start of the master SC1 ISR
      int16u intSrc  = causes;
      int8u errorType = EMBER_SUCCESS;

      SCx_REG(port, DMACTRL) = SC_RXDMARST;  //to clear error
      //state fully captured, DMA reset, now we process error and restart

      if ( intSrc & INT_SCRXOVF ) {
        //Read the data register four times to clear
        //the RXOVERRUN condition and empty the FIFO, giving us 4 bytes
        //worth of time (from this point) to reenable the DMA.
        (void) SCx_REG(port, DATA);
        (void) SCx_REG(port, DATA);
        (void) SCx_REG(port, DATA);
        (void) SCx_REG(port, DATA);

        if ( status & ( SC_RXFRMA
                     | SC_RXFRMB
                     | SC_RXPARA
                     | SC_RXPARB ) ) {
          //We just emptied hardware FIFO so the overrun condition is cleared.
          //Byte errors require special handling to roll back the serial FIFO.
          goto dealWithByteError;
        }

      //record the error type
      emSerialRxError[port] = EMBER_SERIAL_RX_OVERRUN_ERROR;

      //check for a retriggering of the Rx overflow, don't advance FIFO if so
      if ( !(BUFSTATE(port)->waitingForTailA && BUFSTATE(port)->waitingForTailB) ) {
        //first, move head to end of buffer head is in
        //second, move head to end of other buffer if tail is not in other buffer
        if ((q->head)<BUFSTATE(port)->rxStartIndexB) {
          //head inside A
          q->used += (BUFSTATE(port)->rxStartIndexB - q->head);
          q->head = (BUFSTATE(port)->rxStartIndexB);
          if ((q->tail)<BUFSTATE(port)->rxStartIndexB) {
            //tail not inside of B
            q->used += BUFSTATE(port)->rxStartIndexB;
            q->head = 0;
          }
        } else {
          //head inside B
          q->used += (BUFSTATE(port)->fifoSize - q->head);
          q->head = 0;
          if ((q->tail)>=BUFSTATE(port)->rxStartIndexB) {
            //tail is not inside of A
            q->used += BUFSTATE(port)->rxStartIndexB;
            q->head = BUFSTATE(port)->rxStartIndexB;
          }
        }
      }

      //Record the error position in the serial FIFO
      if (q->used != BUFSTATE(port)->fifoSize) {
        //mark the byte at q->head as the error
        emSerialRxErrorIndex[port] = q->head;
      } else {
        //Since the FIFO is full, the error index needs special handling
        //so there is no conflict between the head and tail looking at the same
        //index which needs to be marked as an error.
        emSerialRxErrorIndex[port] = RX_FIFO_FULL;
      }

      //By now the error is accounted for and the DMA hardware is reset.
      //By definition, the overrun error means we have no room left, therefore
      //we can't reenable the DMA.  Reset the previous counter states, and set
      //the waitingForTail flags to TRUE - this tells the Pump function we have
      //data to process.  The Pump function will reenable the buffers as they
      //become available, just like normal.
      BUFSTATE(port)->prevCountA = 0;
      BUFSTATE(port)->prevCountB = 0;
      BUFSTATE(port)->waitingForInputToB = FALSE;
      BUFSTATE(port)->waitingForTailA = TRUE;
      BUFSTATE(port)->waitingForTailB = TRUE;
      //from this point we fall through to the end of the Isr and return.

      } else {
      dealWithByteError:
        //We have a byte error to deal with and possibly more than one byte error,
        //of different types in different DMA buffers, so check each error flag.
        //All four error checks translate the DMA buffer's error position to their
        //position in the serial FIFO, and compares the error locations to find
        //the first error to occur after the head of the FIFO.  This error is the
        //error condition that is stored and operated on.
        if ( status & SC_RXFRMA ) {
          tempIdx = errCntA;
          if (tempIdx < q->head) {
            tempIdx += BUFSTATE(port)->fifoSize;
          }
          if (tempIdx<errorIdx) {
            errorIdx = tempIdx;
          }
          errorType = EMBER_SERIAL_RX_FRAME_ERROR;
        }
        if ( status & SC_RXFRMB ) {
          tempIdx = (errCntB + SCx_REG(port, RXBEGB)) - startAddress;
          if (tempIdx < q->head) {
            tempIdx += BUFSTATE(port)->fifoSize;
          }
          if (tempIdx<errorIdx) {
            errorIdx = tempIdx;
          }
          errorType = EMBER_SERIAL_RX_FRAME_ERROR;
        }
        if ( status & SC_RXPARA ) {
          tempIdx = errCntA;
          if (tempIdx < q->head) {
            tempIdx += BUFSTATE(port)->fifoSize;
          }
          if (tempIdx<errorIdx) {
            errorIdx = tempIdx;
          }
          errorType = EMBER_SERIAL_RX_PARITY_ERROR;
        }
        if ( status & SC_RXPARB ) {
          tempIdx = (errCntB + SCx_REG(port, RXBEGB)) - startAddress;
          if (tempIdx < q->head) {
            tempIdx += BUFSTATE(port)->fifoSize;
          }
          if (tempIdx<errorIdx) {
            errorIdx = tempIdx;
          }
          errorType = EMBER_SERIAL_RX_PARITY_ERROR;
        }

        //We now know the type and location of the first error.
        //Move up to the error location and increase the used count.
        q->head = (errorIdx % BUFSTATE(port)->fifoSize);
        if (q->head < q->tail) {
          q->used = ((q->head + BUFSTATE(port)->fifoSize) - q->tail);
        } else {
          q->used = (q->head - q->tail);
        }

        //Mark the byte at q->head as the error
        emSerialRxError[port] = errorType;
        if (q->used != BUFSTATE(port)->fifoSize) {
          //mark the byte at q->head as the error
          emSerialRxErrorIndex[port] = q->head;
        } else {
          //Since the FIFO is full, the error index needs special handling
          //so there is no conflict between the head and tail looking at the same
          //index which needs to be marked as an error.
          emSerialRxErrorIndex[port] = RX_FIFO_FULL;
        }

        //By now the error is accounted for and the DMA hardware is reset.
        halInternalRestartUartDma(port);
      }
    }
    #endif // #ifndef EZSP_UART
    }
  #endif //(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)
}
#endif//!defined(EM_SERIAL1_DISABLED)


#if EM_SERIAL3_ENABLED
  void halInternalUart3RxIsr(int8u *rxData, int8u length)
  {
    EmSerialFifoQueue *q = emSerialRxQueues[3];

    while(length--) {
      if(q->used < (EMBER_SERIAL3_RX_QUEUE_SIZE-1)) {
        FIFO_ENQUEUE(q, *rxData, emSerialRxQueueWraps[3]);
        rxData++;
      } else {
        uartErrorMark(3, EMBER_SERIAL_RX_OVERFLOW);
        return;
      }
    }
  }
#endif


#ifdef SOFTUART
//this requires use of the SysTick counter and will destroy interrupt latency!
static int8u softwareUartRxByte(void)
{
  int8u i;
  int8u bit;
  int8u byte = 0;
  // BIT_TIMEs were determined based on 24 MHz MCU clock.
  // Scale 'em for the actual MCU clock in effect, with rounding.
  // (Because the FCLK might not evenly divide by 1000 or even 500, use quad
  // arithmetic dividing it by 250.)
  int16u fullBitTime  = (int16u)((( FULL_BIT_TIME_SCLK
                                      * (halMcuClockKHz() / 250)) + 48) / 96);
  int16u startBitTime = (int16u)(((START_BIT_TIME_SCLK
                                      * (halMcuClockKHz() / 250)) + 48) / 96);
  ATOMIC(
    INTERRUPTS_ON();
    //we can only begin receiveing if the input is idle high
    while (SOFT_UART_RX_BIT != 1) {}
    //now wait for our start bit
    while (SOFT_UART_RX_BIT != 0) {}
    INTERRUPTS_OFF();

    //set reload value such that move to the center of an incoming bit
    ST_RVR = startBitTime;
    ST_CVR = 0; //writing the current value will cause it to reset to zero
    //enable core clock reference and the counter itself
    ST_CSR = (ST_CSR_CLKSOURCE | ST_CSR_ENABLE);
    while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 0.5bit time
    //set reload value such that move 1bit time
    ST_RVR = fullBitTime;
    ST_CVR = 0; //writing the current value will cause it to reset to zero

    //loop 8 times recieving all 8 bits and building up the byte
    for (i=0;i<8;i++) {
      while ((ST_CSR&ST_CSR_COUNTFLAG) != ST_CSR_COUNTFLAG) {} //wait 1bit time
      bit = SOFT_UART_RX_BIT; //get the data bit
      bit = ((bit&0x1)<<7);
      byte = (byte>>1)|(bit);
    }

    //disable SysTick
    ST_CSR = 0;
  )
  return byte;
}
#endif //SOFTUART

void halInternalUartRxPump(int8u port)
{
  #ifdef SOFTUART
    if (EM_SER1_PORT_EN(port)) {
      EmSerialFifoQueue *q = emSerialRxQueues[1];
      int8u errors;
      int8u byte;

      //always configure the bit because other operations might have
      //tried to compromise it
      CONFIG_SOFT_UART_RX_BIT();

      //this will block waiting for a start bit!
      byte = softwareUartRxByte();

      if (q->used < (EMBER_SERIAL1_RX_QUEUE_SIZE-1)) {
          FIFO_ENQUEUE(q, byte, emSerialRxQueueWraps[1]);
      } else {
        errors = EMBER_SERIAL_RX_OVERFLOW;
        uartErrorMark(1, errors);
      }
      return;
    }
  #endif

  #ifdef EM_ENABLE_SERIAL_BUFFER
    if (EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port)) {
      EmSerialFifoQueue *q = emSerialRxQueues[port];
      int8u tail,head;
      int16u count=0;
      int8u loadA;
      int8u loadB;
      //Load all of the hardware status, so we can process what happened
      //without worrying about new data changing these values.
      int8u dmaStatus = SCx_REG(port, DMACTRL);
      int16u currCountA = SCx_REG(port, RXCNTA);
      int16u currCountB = SCx_REG(port, RXCNTB);

      //Normal check to see if A has any data
      if (BUFSTATE(port)->prevCountA != currCountA) {
        //Update the counters and head location for the new data
        count = (currCountA - BUFSTATE(port)->prevCountA);
        q->used += count;
        q->head = (q->head + count) % emSerialRxQueueSizes[port];
        BUFSTATE(port)->prevCountA = currCountA;
        BUFSTATE(port)->waitingForTailA = TRUE;
      }
      //Normal check to see if B has any data at all
      if (BUFSTATE(port)->prevCountB != currCountB) {
        //Update the counters and head location for the new data
        count = (currCountB - BUFSTATE(port)->prevCountB);
        q->used += count;
        q->head = (q->head + count) % emSerialRxQueueSizes[port];
        BUFSTATE(port)->prevCountB = currCountB;
        BUFSTATE(port)->waitingForTailB = TRUE;
        BUFSTATE(port)->waitingForInputToB = FALSE;
      }


      //if the used count is greater than half the buffer size, nothing can be done
      if (q->used > BUFSTATE(port)->rxStartIndexB) {
        return;
      }
      //if nothing is in the FIFO, we can reload both if needed
      if (q->used == 0) {
        loadA = TRUE;
        loadB = TRUE;
        goto reloadBuffers;
      }
      //0 < used < bufferSize, so figure out where tail and head are
      if((q->tail)/(BUFSTATE(port)->rxStartIndexB)) {
        tail = TRUE;  //Tail in B buffer
      } else {
        tail = FALSE; //Tail in A buffer
      }

      if(((int16u)(q->head - 1))/(BUFSTATE(port)->rxStartIndexB)) {
        head = TRUE;  //Head in B buffer
      } else {
        head = FALSE; //Head in A buffer
      }

      //To load, the tail must be in the same buffer as the head so we don't
      //overwrite any bytes that haven't drained from the serial FIFO yet.
      if (tail!=head) {
        halInternalUartRxCheckRts(port);
        return;
      }
      // Recall tail TRUE means data is inside B
      loadA = tail;
      loadB = !tail;
  reloadBuffers:
      //check if the buffers need to be reloaded
      if ( (loadA) && (!BUFSTATE(port)->waitingForInputToB) ) {
        if ( (dmaStatus&SC_RXLODA)
            != SC_RXLODA) {
          //An error interrupt can move the addresses of the buffer
          //during the flush/reset/reload operation.  At this point the
          //buffer is clear of any usage, so we can reset the addresses
          SCx_REG(port, RXBEGA) = (int32u)q->fifo;
          SCx_REG(port, RXENDA) = (int32u)(q->fifo + BUFSTATE(port)->fifoSize/2 - 1);
          BUFSTATE(port)->prevCountA = 0;
          BUFSTATE(port)->waitingForTailA = FALSE;
          SCx_REG(port, DMACTRL) = SC_RXLODA;
        }
      }
      if (loadB) {
        if ( (dmaStatus&SC_RXLODB)
            != SC_RXLODB) {
          //An error interrupt can move the addresses of the buffer
          //during the flush/reset/reload operation.  At this point the
          //buffer is clear of any usage, so we can reset the addresses
          SCx_REG(port, RXBEGB) = (int32u)(q->fifo + BUFSTATE(port)->fifoSize/2);
          SCx_REG(port, RXENDB) = (int32u)(q->fifo + BUFSTATE(port)->fifoSize - 1);
          BUFSTATE(port)->prevCountB = 0;
          BUFSTATE(port)->waitingForTailB = FALSE;
          SCx_REG(port, DMACTRL) = SC_RXLODB;
        }
      }
      halInternalUartRxCheckRts(port);
    }
  #endif // EM_ENABLE_SERIAL_BUFFER
}

#if defined(EMBER_SERIAL1_RTSCTS) || defined(EMBER_SERIAL2_RTSCTS)
void halInternalUartRxCheckRts(int8u port)
{
  // Verify RTS is controlled by SW (not AUTO mode), and isn't already asserted.
  // (The logic to deassert RTS is in halInternalUart1RxIsr().)
  if ((SCx_REG(port, UARTCFG) & (SC_UARTFLOW | SC_UARTAUTO | SC_UARTRTS)) == SC_UARTFLOW) {
    // Assert RTS if the rx queue tail is in an active (or pending) DMA buffer,
    // because this means the other DMA buffer is empty.
    ATOMIC (
      if ( ( (emSerialRxQueues[port]->tail < emSerialRxQueueSizes[port]/2) &&
             (SCx_REG(port, DMACTRL) & SC_RXLODA) ) ||
           ( (emSerialRxQueues[port]->tail >= emSerialRxQueueSizes[port]/2)
              && (SCx_REG(port, DMACTRL) & SC_RXLODB) ) ) {
          SCx_REG(port, UARTCFG) |= SC_UARTRTS;          // assert RTS
      }
    )
  }
}
#endif

#ifdef EMBER_SERIAL1_RTSCTS
boolean halInternalUartFlowControlRxIsEnabled(int8u port)
{
  return ( (SCx_REG(port, UARTCFG) & (SC_UARTFLOW | SC_UARTAUTO | SC_UARTRTS)) ==
           (SC_UARTFLOW | SC_UARTRTS) );
}
#endif
#ifdef EMBER_SERIAL1_XONXOFF
boolean halInternalUartFlowControlRxIsEnabled(int8u port)
{
  xonTimer = halCommonGetInt16uQuarterSecondTick(); //FIXME move into new func?
  return ( (xonXoffTxByte == 0) && (xcmdCount > 0) );
}

boolean halInternalUartXonRefreshDone(int8u port)
{
  return (xcmdCount == XON_REFRESH_COUNT);
}
#endif

boolean halInternalUartTxIsIdle(int8u port)
{
  // TODO how do we determine idle for the VUART or USB?
  #if defined(EM_PHYSICAL_UART)
    if (EM_SER1_PORT_EN(port) || EM_SER2_PORT_EN(port)) {
      return ( (SCx_REG(port, MODE) == SC1_MODE_UART) &&
               ((SCx_REG(port, UARTSTAT) & SC_UARTTXIDLE) != 0) );
    }
  #endif

  return TRUE;
}

#if defined(EM_PHYSICAL_UART)
// If called outside of an ISR, it should be from within an ATOMIC block.
static void halInternalUartTxIsr(int8u port)
{
  #if defined(EM_ENABLE_SERIAL_FIFO)
    if (EM_SER_MULTI(EM_SER1_PORT_FIFO(port) || EM_SER2_PORT_FIFO(port))) {
      EmSerialFifoQueue *q = (EmSerialFifoQueue *)emSerialTxQueues[port];

      // At present we really don't care which interrupt(s)
      // occurred, just that one did.  Loop while there is
      // room to send more data and we've got more data to
      // send.  For UART there is no error detection.

#ifdef EMBER_SERIAL1_XONXOFF
      // Sending an XON or XOFF takes priority over data in the tx queue.
      if (xonXoffTxByte && (SCx_REG(port, UARTSTAT) & SC_UARTTXFREE) ) {
        SCx_REG(port, DATA) = xonXoffTxByte;
        if (xonXoffTxByte == ASCII_XOFF) {
          xcmdCount = -1;
          HANDLE_ASH_ERROR(EMBER_COUNTER_ASH_XOFF);
        } else {
          xcmdCount = (xcmdCount < 0) ? 1: xcmdCount + 1;
        }
        xonXoffTxByte = 0;    // clear to indicate XON/XOFF was sent
      }
#endif
      while ( (q->used > 0) && (SCx_REG(port, UARTSTAT) & SC_UARTTXFREE) ) {
        SCx_REG(port, DATA) = FIFO_DEQUEUE(q, emSerialTxQueueWraps[port]);
      }
    }
  #endif
  #if defined(EM_ENABLE_SERIAL_BUFFER)
    if (EM_SER_MULTI(EM_SER1_PORT_BUFFER(port) || EM_SER2_PORT_BUFFER(port))) {
      EmSerialBufferQueue *q = (EmSerialBufferQueue *)emSerialTxQueues[port];

      // The only interrupts we care about here are UNLOAD's and IDLE.
      // Our algorithm doesn't really care which interrupt occurred,
      // or even if one really didn't.  If there is data to send and
      // a DMA channel available to send it, then out it goes.

      assert( !((q->used == 0) && (q->nextByte != NULL)) );
      while ( q->used > 0 ) {
        if ( q->nextByte == NULL ) {
          // new message pending, but nextByte not set up yet
          emSerialBufferNextMessageIsr(q);
        }

        // Something to send: do we have a DMA channel to send it on?
        // Probe for an available channel by checking the channel's
        // SC1_DMACTRL.TX_LOAD   == 0 (channel unloaded) &&
        // SC1_DMASTAT.TX_ACTIVE == 0 (channel not active)
        // The latter check should be superfluous but is a safety mechanism.
        if ( !(SCx_REG(port, DMACTRL) & SC_TXLODA) &&
             !(SCx_REG(port, DMASTAT) & SC_TXACTA) ) {
          // Channel A is available
          SCx_REG(port, TXBEGA)  = (int32u)q->nextByte;
          SCx_REG(port, TXENDA) = (int32u)q->lastByte;
          INT_SCxFLAG(port) = INT_SCTXULDA; // Ack if pending
          SCx_REG(port, DMACTRL) = SC_TXLODA;
          // Release previously held buffer and hold the newly-loaded one
          // so we can safely use emSerialBufferNextBlockIsr() to check for
          // more data to send without the risk of reusing a buffer we're
          // in the process of DMA-ing.
          if (BUFSTATE(port)->holdBuf[0] != EMBER_NULL_MESSAGE_BUFFER)
            emberReleaseMessageBuffer(BUFSTATE(port)->holdBuf[0]);
          BUFSTATE(port)->holdBuf[0] = q->currentBuffer;
          emberHoldMessageBuffer(BUFSTATE(port)->holdBuf[0]);
          emSerialBufferNextBlockIsr(q, port);
        } else
        if ( !(SCx_REG(port, DMACTRL) & SC_TXLODB) &&
             !(SCx_REG(port, DMASTAT) & SC_TXACTB) ) {
          // Channel B is available
          SCx_REG(port, TXBEGB)  = (int32u)q->nextByte;
          SCx_REG(port, TXENDB) = (int32u)q->lastByte;
          INT_SCxFLAG(port) = INT_SCTXULDB; // Ack if pending
          SCx_REG(port, DMACTRL) = SC_TXLODB;
          // Release previously held buffer and hold the newly-loaded one
          // so we can safely use emSerialBufferNextBlockIsr() to check for
          // more data to send without the risk of reusing a buffer we're
          // in the process of DMA-ing.
          if (BUFSTATE(port)->holdBuf[1] != EMBER_NULL_MESSAGE_BUFFER)
            emberReleaseMessageBuffer(BUFSTATE(port)->holdBuf[1]);
          BUFSTATE(port)->holdBuf[1] = q->currentBuffer;
          emberHoldMessageBuffer(BUFSTATE(port)->holdBuf[1]);
          emSerialBufferNextBlockIsr(q, port);
        } else {
          // No channels available; can't send anything now so break out of loop
          break;
        }

      } // while ( q->used > 0 )

      // Release previously-held buffer(s) from an earlier DMA operation
      // if that channel is now free (i.e. it's completed the DMA and we
      // didn't need to use that channel for more output in this call).
      if ( (BUFSTATE(port)->holdBuf[0] != EMBER_NULL_MESSAGE_BUFFER) &&
           !(SCx_REG(port, DMACTRL) & SC_TXLODA) &&
           !(SCx_REG(port, DMASTAT) & SC_TXACTA) ) {
        emberReleaseMessageBuffer(BUFSTATE(port)->holdBuf[0]);
        BUFSTATE(port)->holdBuf[0] = EMBER_NULL_MESSAGE_BUFFER;
      }
      if ( (BUFSTATE(port)->holdBuf[1] != EMBER_NULL_MESSAGE_BUFFER) &&
           !(SCx_REG(port, DMACTRL) & SC_TXLODB) &&
           !(SCx_REG(port, DMASTAT) & SC_TXACTB) ) {
        emberReleaseMessageBuffer(BUFSTATE(port)->holdBuf[1]);
        BUFSTATE(port)->holdBuf[1] = EMBER_NULL_MESSAGE_BUFFER;
      }
    }
  #endif
}
#endif // defined(EM_PHYSICAL_UART)


#if EM_SERIAL1_ENABLED
  //The following registers are the only SC1-UART registers that need to be
  //saved across deep sleep cycles.  All other SC1-UART registers are
  //reenabled or restarted using more complex init or restart algorithms.
  static int32u  SC1_UARTPER_SAVED;
  static int32u  SC1_UARTFRAC_SAVED;
  static int32u  SC1_UARTCFG_SAVED;
#endif // EM_SERIAL1_ENABLED
#if EM_SERIAL2_ENABLED
  //The following registers are the only SC3-UART registers that need to be
  //saved across deep sleep cycles.  All other SC3-UART registers are
  //reenabled or restarted using more complex init or restart algorithms.
  static int32u  SC3_UARTPER_SAVED;
  static int32u  SC3_UARTFRAC_SAVED;
  static int32u  SC3_UARTCFG_SAVED;
#endif // EM_SERIAL2_ENABLED

void halInternalPowerDownUart(void)
{
  #if EM_SERIAL1_ENABLED
    SC1_UARTPER_SAVED = SC1_UARTPER;
    SC1_UARTFRAC_SAVED = SC1_UARTFRAC;
    SC1_UARTCFG_SAVED = SC1_UARTCFG;
    SC1_TXD_GPIO(GPIOCFG_OUT, 1); // Avoid gitching TxD going down
  #endif // EM_SERIAL1_ENABLED
  #if EM_SERIAL2_ENABLED
    SC3_UARTPER_SAVED = SC3_UARTPER;
    SC3_UARTFRAC_SAVED = SC3_UARTFRAC;
    SC3_UARTCFG_SAVED = SC3_UARTCFG;
    // TODO SC3_TXD_GPIO(GPIOCFG_OUT, 1); // Avoid gitching TxD going down
  #endif // EM_SERIAL1_ENABLED
}

void halInternalPowerUpUart(void)
{
  #if EM_SERIAL1_ENABLED
    SC1_UARTPER = SC1_UARTPER_SAVED;
    SC1_UARTFRAC = SC1_UARTFRAC_SAVED;
    SC1_UARTCFG = SC1_UARTCFG_SAVED;

    SC1_MODE = SC1_MODE_UART;
    SC1_TXD_GPIO(GPIOCFG_OUT_ALT, 1); // Can Assign TxD glitch-free to UART now

    halInternalInitUartInterrupts(1);
  #endif

  #if EM_SERIAL2_ENABLED
    SC3_UARTPER = SC3_UARTPER_SAVED;
    SC3_UARTFRAC = SC3_UARTFRAC_SAVED;
    SC3_UARTCFG = SC3_UARTCFG_SAVED;

    SC3_MODE = SC3_MODE_UART;
    //SC3_TXD_GPIO(GPIOCFG_OUT_ALT, 1); // Can Assign TxD glitch-free to UART now

    halInternalInitUartInterrupts(2);
  #endif

  #if EM_SERIAL3_ENABLED
    //Remember, halInternalPowerUpUart does not return anything.  Powering
    //up the USB requires going through its normal configuration and
    //enumeration process.
    #if defined(CORTEXM3_EM35X_USB)
      USBD_Init(&initstruct);
    #endif
  #endif
}


void halInternalRestartUart(void)
{
  // This is no longer needed and should be removed as a dinosaur --DMM
}

#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO) && defined(EMBER_SERIAL1_XONXOFF)
// TODO XON/XOFF on port 2
void halInternalUartFlowControl(int8u port)
{
  if (EM_SER1_PORT_EN(port)) {
    int16u used = emSerialRxQueues[1]->used;
    int8u time = halCommonGetInt16uQuarterSecondTick();

    if (used) {
      xonTimer = time;
    }
    // Send an XON if the rx queue is below the XON threshold
    // and an XOFF had been sent that needs to be reversed
    ATOMIC(
      if ( (xcmdCount == -1) && (used <= XON_LIMIT) ) {
        halInternalUart1ForceXon();
      } else if ( (used == 0) &&
                  ((int8u)(time - xonTimer) >= XON_REFRESH_TIME) &&
                  (xcmdCount < XON_REFRESH_COUNT) ) {
        halInternalUart1ForceXon();
      }
    )
  }
}
#endif

#ifdef EMBER_SERIAL1_XONXOFF
// Must be called from within an ATOMIC block.
static void halInternalUart1ForceXon(void)
{
  if (xonXoffTxByte == ASCII_XOFF) {  // if XOFF waiting to be sent, cancel it
    xonXoffTxByte = 0;
    xcmdCount = 0;
  } else {                            // else, send XON and record the time
    xonXoffTxByte = ASCII_XON;
    halInternalStartUartTx(1);
  }
  xonTimer = halCommonGetInt16uQuarterSecondTick();
}
#endif

#if EM_SERIAL1_ENABLED
void halSc1Isr(void)
{
  int32u interrupt;

  //this read and mask is performed in two steps otherwise the compiler
  //will complain about undefined order of volatile access
  interrupt = INT_SC1FLAG;
  interrupt &= INT_SC1CFG;

  #if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
    while (interrupt != 0) {
  #endif // (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)

      INT_SC1FLAG = interrupt; // acknowledge the interrupts early

      // RX events
      if ( interrupt & (INT_SCRXVAL   | // RX has data
                        INT_SCRXOVF   | // RX Overrun error
                        INT_SCRXFIN   | // RX done [TWI]
                        INT_SCNAK     | // RX Nack [TWI]
                        INT_SCRXULDA  | // RX DMA A has data
                        INT_SCRXULDB  | // RX DMA B has data
                        INT_SC1FRMERR | // RX Frame error
                        INT_SC1PARERR ) // RX Parity error
         ) {
        halInternalUartRxIsr(1, interrupt);
      }

      // TX events
      if ( interrupt & (INT_SCTXFREE | // TX has room
                        INT_SCTXIDLE | // TX idle (more room)
                        INT_SCTXUND  | // TX Underrun [SPI/TWI]
                        INT_SCTXFIN  | // TX complete [TWI]
                        INT_SCCMDFIN | // TX Start/Stop done [TWI]
                        INT_SCTXULDA | // TX DMA A has room
                        INT_SCTXULDB ) // TX DMA B has room
         ) {
        halInternalUartTxIsr(1);
      }

  #if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
      interrupt = INT_SC1FLAG;
      interrupt &= INT_SC1CFG;
    }
  #endif // (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO)
}
#endif // EM_SERIAL1_ENABLED

#if EM_SERIAL2_ENABLED
void halSc3Isr(void)
{
  int32u interrupt;

  //this read and mask is performed in two steps otherwise the compiler
  //will complain about undefined order of volatile access
  interrupt = INT_SC3FLAG;
  interrupt &= INT_SC3CFG;

  #if (EMBER_SERIAL2_MODE == EMBER_SERIAL_FIFO)
    while (interrupt != 0) {
  #endif // (EMBER_SERIAL2_MODE == EMBER_SERIAL_FIFO)

      INT_SC3FLAG = interrupt; // acknowledge the interrupts early

      // RX events
      if ( interrupt & (INT_SCRXVAL   | // RX has data
                        INT_SCRXOVF   | // RX Overrun error
                        INT_SCRXFIN   | // RX done [TWI]
                        INT_SCNAK     | // RX Nack [TWI]
                        INT_SCRXULDA  | // RX DMA A has data
                        INT_SCRXULDB  | // RX DMA B has data
                        INT_SC1FRMERR | // RX Frame error
                        INT_SC1PARERR ) // RX Parity error
         ) {
        halInternalUartRxIsr(2, interrupt);
      }

      // TX events
      if ( interrupt & (INT_SCTXFREE | // TX has room
                        INT_SCTXIDLE | // TX idle (more room)
                        INT_SCTXUND  | // TX Underrun [SPI/TWI]
                        INT_SCTXFIN  | // TX complete [TWI]
                        INT_SCCMDFIN | // TX Start/Stop done [TWI]
                        INT_SCTXULDA | // TX DMA A has room
                        INT_SCTXULDB ) // TX DMA B has room
         ) {
        halInternalUartTxIsr(2);
      }

  #if (EMBER_SERIAL2_MODE == EMBER_SERIAL_FIFO)
      interrupt = INT_SC3FLAG;
      interrupt &= INT_SC3CFG;
    }
  #endif // (EMBER_SERIAL2_MODE == EMBER_SERIAL_FIFO)
}
#endif // EM_SERIAL2_ENABLED

static void uartErrorMark(int8u port, int8u errors)
{
  EmSerialFifoQueue *q = emSerialRxQueues[port];

  // save error code & location in queue
  if ( emSerialRxError[port] == EMBER_SUCCESS ) {
    emSerialRxErrorIndex[port] = q->head;
    emSerialRxError[port] = errors;
  } else {
    // Flush back to previous error location & update value
    q->head = emSerialRxErrorIndex[port];
    emSerialRxError[port] = errors;
    if(q->head < q->tail) {
      q->used = (emSerialRxQueueSizes[port] - q->tail) + q->head;
    } else {
      q->used = q->head - q->tail;
    }
  }
}
