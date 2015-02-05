#include "app/framework/include/af.h"

typedef struct {
  boolean waitForAck ;
  boolean checkCca ;
  int8u   ccaAttemptMax ;
  int8u   backoffExponentMin ;
  int8u   backoffExponentMax ;
  int8u   minimumBackoff ;
  boolean appendCrc ;
} RadioTransmitConfig ;
  
extern boolean  emEraseChildTableOnJoin ;
extern int8u    emMacMaxAckRetries ;
extern int8u    emZigbeeMaxNetworkRetries ;
extern RadioTransmitConfig radioTransmitConfig ;

extern boolean emRadioTruncateIncomingAckToPoll;
extern boolean emRadioManagePollTxRxTurnaround;
extern boolean emRadioAlwaysUseZeroBackoff;

void initSuperSleepyDevice(void)
{
  emEraseChildTableOnJoin   = FALSE ;
  emZigbeeMaxNetworkRetries = 0 ;
  emMacMaxAckRetries        = 0 ;
#if 0
  // Configure CSMA behavior
  radioTransmitConfig.ccaAttemptMax = 4 ;
  radioTransmitConfig.backoffExponentMin = 0 ;
  radioTransmitConfig.backoffExponentMax = 1  ;
  radioTransmitConfig.minimumBackoff = 0 ;
#endif

  //emZigbeeMaxNetworkRetries = 0 ;
  //emMacMaxAckRetries        = 0 ;

  // This switch enables an operational mode in which the stack 
  // turns off the radio after observing the Frame Pending bit set to 0 
  // in the incoming ACK to a Data Poll.
  emRadioTruncateIncomingAckToPoll = TRUE;
 // This switch enables an operational mode in which the stack 
  // turns off the radio immediately after transmitting a Data Poll 
  // and turns it back on just before the ACK is expected to arrive.
  emRadioManagePollTxRxTurnaround = TRUE;
    
  // Enabling the internal variable emRadioAlwaysUseZeroBackoff puts the 
  //  MAC into a mode were only CSMA backoffs of zero are used.  
  //  This is intended only to remove variability when doing power 
  //  consumption measurements and eases comparison between different
  //  scenarios.
  // Enabling this variable in a normal application will have a negative
  //  impact on overall network performance.
  emRadioAlwaysUseZeroBackoff = TRUE;

  //halSleep(SLEEPMODE_IDLE);
  //halSleep(SLEEPMODE_WAKETIMER);
  //halSleep(SLEEPMODE_MAINTAINTIMER);
  //halSleep(SLEEPMODE_NOTIMER);

  //radioTransmitConfig.backoffExponentMin = 0 ;
  //radioTransmitConfig.backoffExponentMax = 1  ;
  //radioTransmitConfig.minimumBackoff = 0 ;

}