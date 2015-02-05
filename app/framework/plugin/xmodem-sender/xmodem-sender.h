

#define XMODEM_SOH   (0x01)
#define XMODEM_EOT   (0x04)
#define XMODEM_ACK   (0x06)
#define XMODEM_NAK   (0x15)
#define XMODEM_CANCEL (0x18)
#define XMODEM_BLOCKOK (0x19)
#define XMODEM_FILEDONE (0x17)


typedef EmberStatus (EmberAfXmodemSenderTransmitFunction)(int8u* data, int8u length);

typedef EmberStatus (EmberAfXmodemSenderGetNextBlockFunction)(int32u address, 
                                                              int8u length, 
                                                              int8u* returnData, 
                                                              int8u* returnLength,
                                                              boolean* done);
typedef void (EmberAfXmodemSenderFinishedFunction)(boolean success);

void emberAfPluginXmodemSenderIncomingBlock(int8u* data,
                                            int8u  length);

// The maxSizeOfBlock does not include the Xmodem overhead (5-bytes)
EmberStatus emberAfPluginXmodemSenderStart(EmberAfXmodemSenderTransmitFunction* sendRoutine,
                                           EmberAfXmodemSenderGetNextBlockFunction* getNextBlockRoutine,
                                           EmberAfXmodemSenderFinishedFunction* finishedRoutine,
                                           int8u maxSizeOfBlock,
                                           boolean waitForReady);

void emberAfPluginXmodemSenderAbort(void);
