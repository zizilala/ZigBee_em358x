/** @file hal/micro/serial.h
 * @brief Serial hardware abstraction layer interfaces.
 * See @ref serial for documentation.
 *
 * <!-- Copyright 2013 Silicon Laboratories, Inc.                        *80*-->
 */
 
 /** @addtogroup serial
 * @brief This API contains the HAL interfaces that applications must implement
 * for the high-level serial code.
 *
 * This header describes the interface between the high-level serial APIs in
 * app/util/serial/serial.h and the low level UART implementation.
 *
 * Some functions in this file return an ::EmberStatus value. See 
 * error-def.h for definitions of all ::EmberStatus return values.
 *
 * See hal/micro/serial.h for source code. 
 *@{
 */


#ifndef __HAL_SERIAL_H__
#define __HAL_SERIAL_H__

// Include ember.h to get EmberMessageBuffer definition.
// Why is this necessary for ZIP but not for ZNet?  In fact, including it in
// ZNet breaks things.  I can't figure out how to untangle the
// dependencies, but I'm thinking this is not the right place to include
// this header.  Since it's breaking things for other people, I'll just
// apply the band aid and worry about it later. -- ryan
#ifdef EMBER_STACK_IP
  #include "stack/include/ember.h"
#endif

// EM_NUM_SERIAL_PORTS is inherited from the micro specifc micro.h
#if (EM_NUM_SERIAL_PORTS == 1)
  #define FOR_EACH_PORT(cast,prefix,suffix)  \
    cast(prefix##0##suffix)
#elif (EM_NUM_SERIAL_PORTS == 2)
  #define FOR_EACH_PORT(cast,prefix,suffix)  \
    cast(prefix##0##suffix),                 \
    cast(prefix##1##suffix)
#elif (EM_NUM_SERIAL_PORTS == 3)
  #define FOR_EACH_PORT(cast,prefix,suffix)  \
    cast(prefix##0##suffix),            \
    cast(prefix##1##suffix),            \
    cast(prefix##2##suffix)
#elif (EM_NUM_SERIAL_PORTS == 4)
  #define FOR_EACH_PORT(cast,prefix,suffix)  \
    cast(prefix##0##suffix),            \
    cast(prefix##1##suffix),            \
    cast(prefix##2##suffix),            \
    cast(prefix##3##suffix),
#elif (EM_NUM_SERIAL_PORTS == 5)
  #define FOR_EACH_PORT(cast,prefix,suffix)  \
    cast(prefix##0##suffix),            \
    cast(prefix##1##suffix),            \
    cast(prefix##2##suffix),            \
    cast(prefix##3##suffix),            \
    cast(prefix##4##suffix),
#else
  #error unsupported number of serial ports
#endif

/** @name Serial Mode Definitions
 * These are numerical definitions for the possible serial modes so that code
 * can test for the one being used.  There may be additional modes defined in
 * the micro-specific micro.h.
 *
 *@{
 */
 
/**
 * @brief A numerical definition for a possible serial mode the code
 * can test for.
 */
#define EMBER_SERIAL_UNUSED 0
#define EMBER_SERIAL_FIFO   1
#define EMBER_SERIAL_BUFFER 2
#define EMBER_SERIAL_LOWLEVEL 3
/** @}  END of Serial Mode Definitions */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// The following tests for setting of an invalid mode
#ifdef EMBER_SERIAL0_MODE
#if (EMBER_SERIAL0_MODE != EMBER_SERIAL_FIFO)   && \
    (EMBER_SERIAL0_MODE != EMBER_SERIAL_BUFFER) && \
    (EMBER_SERIAL0_MODE != EMBER_SERIAL_LOWLEVEL) && \
    (EMBER_SERIAL0_MODE != EMBER_SERIAL_UNUSED)
  #error Invalid Serial 0 Mode
#endif
#else
  #define EMBER_SERIAL0_MODE EMBER_SERIAL_UNUSED
#endif
#ifdef EMBER_SERIAL1_MODE
#if (EMBER_SERIAL1_MODE != EMBER_SERIAL_FIFO)   && \
    (EMBER_SERIAL1_MODE != EMBER_SERIAL_BUFFER) && \
    (EMBER_SERIAL1_MODE != EMBER_SERIAL_LOWLEVEL) && \
    (EMBER_SERIAL1_MODE != EMBER_SERIAL_UNUSED)
  #error Invalid Serial 1 Mode
#endif
#else
  #define EMBER_SERIAL1_MODE EMBER_SERIAL_UNUSED
#endif
#ifdef EMBER_SERIAL2_MODE
#if (EMBER_SERIAL2_MODE != EMBER_SERIAL_FIFO)   && \
    (EMBER_SERIAL2_MODE != EMBER_SERIAL_BUFFER) && \
    (EMBER_SERIAL2_MODE != EMBER_SERIAL_LOWLEVEL) && \
    (EMBER_SERIAL2_MODE != EMBER_SERIAL_UNUSED)
  #error Invalid Serial 2 Mode
#endif
#else
  #define EMBER_SERIAL2_MODE EMBER_SERIAL_UNUSED
#endif
#ifdef EMBER_SERIAL3_MODE
#if (EMBER_SERIAL3_MODE != EMBER_SERIAL_FIFO)   && \
    (EMBER_SERIAL3_MODE != EMBER_SERIAL_BUFFER) && \
    (EMBER_SERIAL3_MODE != EMBER_SERIAL_LOWLEVEL) && \
    (EMBER_SERIAL3_MODE != EMBER_SERIAL_UNUSED)
  #error Invalid Serial 3 Mode
#endif
#else
  #define EMBER_SERIAL3_MODE EMBER_SERIAL_UNUSED
#endif
#ifdef EMBER_SERIAL4_MODE
#if (EMBER_SERIAL4_MODE != EMBER_SERIAL_FIFO)   && \
    (EMBER_SERIAL4_MODE != EMBER_SERIAL_BUFFER) && \
    (EMBER_SERIAL4_MODE != EMBER_SERIAL_LOWLEVEL) && \
    (EMBER_SERIAL4_MODE != EMBER_SERIAL_UNUSED)
  #error Invalid Serial 4 Mode
#endif
#else
  #define EMBER_SERIAL4_MODE EMBER_SERIAL_UNUSED
#endif

// Determine if FIFO and/or Buffer modes are being used, so those sections of
//  code may be disabled if not
#if (EMBER_SERIAL0_MODE == EMBER_SERIAL_FIFO) || \
    (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO) || \
    (EMBER_SERIAL2_MODE == EMBER_SERIAL_FIFO) || \
    (EMBER_SERIAL3_MODE == EMBER_SERIAL_FIFO) || \
    (EMBER_SERIAL4_MODE == EMBER_SERIAL_FIFO)
  #define EM_ENABLE_SERIAL_FIFO
#endif
#if (EMBER_SERIAL0_MODE == EMBER_SERIAL_BUFFER) || \
    (EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER) || \
    (EMBER_SERIAL2_MODE == EMBER_SERIAL_BUFFER) || \
    (EMBER_SERIAL3_MODE == EMBER_SERIAL_BUFFER) || \
    (EMBER_SERIAL4_MODE == EMBER_SERIAL_BUFFER)
  #define EM_ENABLE_SERIAL_BUFFER
#endif

/** @name Queue Data Structures 
 *@{
 */

/** @brief A basic FIFO queue that stores individual characters
 *  used for ::EMBER_SERIAL_MODE_FIFO.
 */
typedef struct {
  /** Index of next byte to send.*/
  int16u head;
  /** Index of where to enqueue next message.*/
  int16u tail;
  /** Number of bytes queued.*/
  volatile int16u used;
  /** FIFO of queue data.*/
  int8u fifo[];
} EmSerialFifoQueue;


/** @brief A single element of the ::EmSerialBufferQueue
 *  used for ::EMBER_SERIAL_MODE_BUFFER.
 */
typedef struct {
  /** Number of bytes in this buffer to write.*/
  int8u length;
  /** The first linked buffer.*/
  EmberMessageBuffer buffer;
  /** Starting position within the buffer.*/
  int8u startIndex;
} EmSerialBufferQueueEntry;

/** @brief A basic FIFO queue that stores packet buffer chains
 *  that is used for ::EMBER_SERIAL_MODE_BUFFER.
 */
typedef struct {
  /** Index of next message to send */  
  int8u head;        
  /** Index of where to enqueue new messages.*/
  int8u tail;        
  /** Number of active messages.*/
  volatile int8u used;        
  /** Number of messages sent that need cleanup.*/
  volatile int8u dead;        
  /** Packet buffer currently writing from, if any (used to
  step down a chain of buffers).*/
  EmberMessageBuffer currentBuffer;
  /** Pointer to the next byte to send from current packet buffer.*/
  int8u *nextByte;
  /** Pointer to the last byte to send from current packet buffer.*/
  int8u *lastByte;
  /** FIFO of queued messages.*/
  EmSerialBufferQueueEntry fifo[];
} EmSerialBufferQueue;

/** @}  END of Queue Data Structures */

/** @name Serial Data Structures
 * These data structures maintain the state of serial communication on each
 * port. They are defined in serial.c and may be accessed by the HAL.
 *@{
 */

/** Pointer to the transmit queue data structure for each port.*/
extern void *emSerialTxQueues[EM_NUM_SERIAL_PORTS];
/** Pointer to the receive FIFO queue data structure for each port.*/
extern EmSerialFifoQueue *emSerialRxQueues[EM_NUM_SERIAL_PORTS];
/** Masks used for managing the FIFOs of the transmit queues.*/
extern int16u PGM emSerialTxQueueMasks[EM_NUM_SERIAL_PORTS];
/** Sizes used for managing the FIFOs of the transmit queues.*/
extern int16u PGM emSerialTxQueueSizes[EM_NUM_SERIAL_PORTS];
/** Sizes used for managing the FIFOs of the receive queues.*/
extern int16u PGM emSerialRxQueueSizes[EM_NUM_SERIAL_PORTS];
/** Last UART receive error to occur.*/
extern int8u emSerialRxError[EM_NUM_SERIAL_PORTS];
/** Index in the FIFO queue where the last receive error occurred.*/
extern int16u emSerialRxErrorIndex[EM_NUM_SERIAL_PORTS];
/** Mode in which the ports are operating.*/
extern int8u PGM emSerialPortModes[EM_NUM_SERIAL_PORTS];

//Compatibility code for the AVR Atmega
#ifdef AVR_ATMEGA
extern int8u PGM emSerialTxQueueWraps[EM_NUM_SERIAL_PORTS];
extern int8u PGM emSerialRxQueueWraps[EM_NUM_SERIAL_PORTS];
#else
extern int16u PGM emSerialTxQueueWraps[EM_NUM_SERIAL_PORTS];
extern int16u PGM emSerialRxQueueWraps[EM_NUM_SERIAL_PORTS];
#endif

/** @} END of Serial Data Structures */
 
#ifdef EZSP_UART
  #define HANDLE_ASH_ERROR(type) emCallCounterHandler(type, 0)
#else
  #define HANDLE_ASH_ERROR(type)
#endif

#endif //DOXYGEN_SHOULD_SKIP_THIS


/** @name FIFO Utility Macros
 * These macros manipulate the FIFO queue data structures to add and remove 
 * data.
 *
 *@{
 */
 
/** 
 * @brief Macro that enqueues a byte of data in a FIFO queue.
 *  
 * @param queue  Pointer to the FIFO queue.
 *  
 * @param data   Data byte to be enqueued.
 *  
 * @param size   Size used to control the wrap-around of the FIFO pointers.
 */
//Compatibility code for the AVR Atmega
#ifdef AVR_ATMEGA
#define FIFO_ENQUEUE(queue,data,size) \
  do {                                \
    queue->fifo[queue->head] = data;      \
    queue->head = ((queue->head + 1) & size);  \
    queue->used++;                    \
  } while(0)
#else
#define FIFO_ENQUEUE(queue,data,size) \
  do {                                \
    queue->fifo[queue->head] = data;      \
    queue->head = ((queue->head + 1) % size);  \
    queue->used++;                    \
  } while(0)
#endif
  
/** 
 * @brief Macro that de-queues a byte of data from a FIFO queue.
 *  
 * @param queue Pointer to the FIFO queue.
 *  
 * @param size  Size used to control the wrap-around of the FIFO pointers.
 */
//Compatibility code for the AVR Atmega
#ifdef AVR_ATMEGA
#define FIFO_DEQUEUE(queue,size) \
  queue->fifo[queue->tail];          \
  queue->tail = ((queue->tail + 1) & size);  \
  queue->used--
#else
#define FIFO_DEQUEUE(queue,size) \
  queue->fifo[queue->tail];          \
  queue->tail = ((queue->tail + 1) % size);  \
  queue->used--
#endif
  
/** @}  END of FIFO Utility Macros */
 

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Assign numerical values for variables that hold Baud Rate
 * parameters. 
 */
enum SerialBaudRate
#else
#ifndef DEFINE_BAUD
#define DEFINE_BAUD(num) BAUD_##num
#endif
typedef int8u SerialBaudRate;
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{ 
  DEFINE_BAUD(300) = 0,  // BAUD_300
  DEFINE_BAUD(600) = 1,  // BAUD_600
  DEFINE_BAUD(900) = 2,  // etc...
  DEFINE_BAUD(1200) = 3,
  DEFINE_BAUD(2400) = 4,
  DEFINE_BAUD(4800) = 5,
  DEFINE_BAUD(9600) = 6,
  DEFINE_BAUD(14400) = 7,
  DEFINE_BAUD(19200) = 8,
  DEFINE_BAUD(28800) = 9,
  DEFINE_BAUD(38400) = 10,
  DEFINE_BAUD(50000) = 11,
  DEFINE_BAUD(57600) = 12,
  DEFINE_BAUD(76800) = 13,
  DEFINE_BAUD(100000) = 14,
  DEFINE_BAUD(115200) = 15,
#ifdef AVR_ATMEGA
  DEFINE_BAUD(CUSTOM) = 16
#else
  DEFINE_BAUD(230400) = 16,   /*<! define higher baud rates for the EM2XX and EM3XX */
  DEFINE_BAUD(460800) = 17,   /*<! Note: receiving data at baud rates > 115200 */
  DEFINE_BAUD(CUSTOM) = 18    /*<! may not be reliable due to interrupt latency */
#endif
};


#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**
 * @brief Assign numerical values for the types of parity. 
 * Use for variables that hold Parity parameters.
 */
enum NameOfType
#else
#ifndef DEFINE_PARITY
#define DEFINE_PARITY(val) PARITY_##val
#endif
typedef int8u SerialParity;
enum
#endif //DOXYGEN_SHOULD_SKIP_THIS
{
  DEFINE_PARITY(NONE) = 0,  // PARITY_NONE
  DEFINE_PARITY(ODD) = 1,   // PARITY_ODD
  DEFINE_PARITY(EVEN) = 2   // PARITY_EVEN
};

/** @name Serial HAL APIs
 * These functions must be implemented by the HAL in order for the
 * serial code to operate. Only the higher-level serial code uses these 
 * functions, so they should not be called directly. The HAL should also 
 * implement the appropriate interrupt handlers to drain the TX queues and fill 
 * the RX FIFO queue.
 *
 *@{
 */


/** @brief Initializes the UART to the given settings (same parameters 
 * as  ::emberSerialInit() ).
 * 
 * @param port     Serial port number (0 or 1).
 *  
 * @param rate     Baud rate (see  SerialBaudRate).
 * 
 * @param parity   Parity value (see  SerialParity).
 * 
 * @param stopBits Number of stop bits.
 * 
 * @return An error code if initialization failed (such as invalid baud rate),  
 * otherise EMBER_SUCCESS.
 */
EmberStatus halInternalUartInit(int8u port, 
                                SerialBaudRate rate,
                                SerialParity parity,
                                int8u stopBits);

/** @brief This function is typically called by ::halPowerDown()
 * and it is responsible for performing all the work internal to the UART
 * needed to stop the UART before a sleep cycle.
 */
void halInternalPowerDownUart(void);

/** @brief This function is typically called by ::halPowerUp()
 * and it is responsible for performing all the work internal to the UART
 * needed to restart the UART after a sleep cycle.
 */
void halInternalPowerUpUart(void);

/** @brief Called by serial code whenever anything is queued for 
 * transmission to start any interrupt-driven transmission. May
 * be called when transmission is already in progess.
 *  
 *  @param port  Serial port number (0 or 1).
 */
void halInternalStartUartTx(int8u port);


/** @brief Called by serial code to stop any interrupt-driven serial
 * transmission currently in progress.
 * 
 * @param port  Serial port number (0 or 1).
 */
void halInternalStopUartTx(int8u port);


/** @brief Directly writes a byte to the UART for transmission, 
 * regardless of anything currently queued for transmission. Should wait 
 * for anything currently in the UART hardware registers to finish transmission 
 * first, and block until \c data is finished being sent. 
 *  
 * @param port    Serial port number (0 or 1).
 *  
 * @param data    Pointer to the data to be transmitted.
 *
 * @param length  The length of data to be transmitted
 */
EmberStatus halInternalForceWriteUartData(int8u port, int8u *data, int8u length);

/** @brief Directly reads a byte from the UART for reception, 
 * regardless of anything currently queued for reception. Does not block
 * if a data byte has not been received.
 *  
 * @param port      Serial port number (0 or 1).
 *  
 * @param dataByte  The byte to receive data into.
 */
EmberStatus halInternalForceReadUartByte(int8u port, int8u *dataByte);

/** @brief Blocks until the UART has finished transmitting any data in 
 * its hardware registers.
 * 
 * @param port  Serial port number (0 or 1).
 */
void halInternalWaitUartTxComplete(int8u port);

/** @brief This function is used in FIFO mode when flow control is enabled.
 *  It is called from emberSerialReadByte(), and based on the number of bytes
 *  used in the uart receive queue, decides when to tell the host it may resume
 *  transmission.
 * 
 * @param port  Serial port number (0 or 1). (Does nothing for port 0)
 */
#if (EMBER_SERIAL1_MODE == EMBER_SERIAL_FIFO) &&          \
    ( defined(EMBER_SERIAL1_XONXOFF) ||                   \
    (defined(XAP2B) && defined(EMBER_SERIAL1_RTSCTS) ) ) 
void halInternalUartFlowControl(int8u port);
#else
#define halInternalUartFlowControl(port) do {} while(FALSE)
#endif

/** @brief This function exists only in Buffer Mode on the EM2xx and in
 * software UART (SOFTUART) mode on the EM3xx.  This function is called
 * by ::emberSerialReadByte().  It is responsible for maintaining synchronization
 * between the emSerialRxQueue and the UART DMA.
 * 
 * @param port  Serial port number (0 or 1).
 */
#if (defined(XAP2B)&&(EMBER_SERIAL1_MODE == EMBER_SERIAL_BUFFER)) || defined(CORTEXM3)
void halInternalUartRxPump(int8u port);
#else
#define halInternalUartRxPump(port) do {} while(FALSE)
#endif

/** @brief This function is typically called by ::halInternalPowerUpBoard()
 * and it is responsible for performing all the work internal to the UART
 * needed to restart the UART after a sleep cycle. (For example, resyncing the
 * DMA hardware and the serial FIFO.)
 */
void halInternalRestartUart(void);

/** @brief Checks to see if the host is allowed to send serial data to the
 * ncp - i.e., it is not being held off by nCTS or an XOFF. Returns TRUE is
 * the host is able to send. 
 * 
 */
boolean halInternalUartFlowControlRxIsEnabled(int8u port);
// define the old name for backwards compatibility
#define halInternalUart1FlowControlRxIsEnabled() halInternalUartFlowControlRxIsEnabled(1)

/** @brief When Xon/Xoff flow control is used, returns TRUE if the
 *  host is not being held off and XON refreshing is complete.
 * 
 */
boolean halInternalUartXonRefreshDone(int8u port);
#define halInternalUart1XonRefreshDone() halInternalUartXonRefreshDone(1)

/** @brief Returns true if the uart transmitter is idle, including the
 * transmit shift register.
 * 
 */
boolean halInternalUartTxIsIdle(int8u port);
// define the old name for backwards compatibility
#define halInternalUart1TxIsIdle() halInternalUartTxIsIdle(1)

/** @brief Testing function implemented by the upper layer.
 * Determines whether the next packet should be dropped.
 * Returns TRUE if the next packet should be dropped, FALSE otherwise.
 */
boolean serialDropPacket(void);

/** @}  END of Serial HAL APIs */

/** @name Buffered Serial Utility APIs
 * The higher-level serial code implements these APIs, which the HAL uses 
 * to deal with buffered serial output.
 *@{
 */

/** @brief When new serial transmission is started and  
 * \c bufferQueue->nextByte is equal to NULL, this can be called to set up  
 * \c nextByte and \c lastByte for the next message.
 *  
 * @param q  Pointer to the buffer queue structure for the port.
 */
void emSerialBufferNextMessageIsr(EmSerialBufferQueue *q);

/** @brief When a serial transmission is in progress and  
 * \c bufferQueue->nextByte has been sent and incremented leaving it equal to 
 * lastByte, this should be called to set up \c nextByte and \c lastByte 
 * for the next block.
 *  
 * @param q     Pointer to the buffer queue structure for the port.
 *  
 * @param port  Serial port number (0 or 1). 
 */
void emSerialBufferNextBlockIsr(EmSerialBufferQueue *q, int8u port);

/** @} END of Buffered serial utility APIs  */
 
/** @name Virtual UART API
 * API used by the stack in debug builds to receive data arriving over the
 * virtual UART.
 *@{
 */

/** @brief When using a debug build with virtual UART support,
 * this API is called by the stack when virtual UART data has been received
 * over the debug channel
 *  
 * @param data  Pointer to the the data received
 *  
 * @param length   Length of the data received
*/
void halStackReceiveVuartMessage(int8u* data, int8u length);

/** @} END Virtual UART API  */

void halHostFlushBuffers(void);
int16u halHostEnqueueTx(const int8u *data, int16u length);
void halHostFlushTx(void);

// 'data' points to the next 'length' bytes of input.
int16u serialCopyFromRx(const int8u *data, int16u length);

// tell the upper layer to load serial data
void emLoadSerialTx(void);

#endif //__HAL_SERIAL_H__

/** @} END serial group  */
  
