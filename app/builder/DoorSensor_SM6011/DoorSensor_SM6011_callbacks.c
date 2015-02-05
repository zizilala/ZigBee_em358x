//

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"

// Typedef
typedef enum
{
      E_STATE_INIT = 0,
      E_STATE_SCANNING,
      E_STATE_RUNNING
}te_EmberState;

// Variable of Golbal
//static boolean toggleLED1 = TRUE;
static int8u emberState = E_STATE_INIT;


void emberAfMainInitCallback(void)   
{
    //static int16u lastBlinkTime = 0;
    //int16u newTime;
   
    //emberSerialPrintf(">>Using UART 1 running emberAfMainInitCallback()<<\r\n"); 
    emberAfCorePrint(">>Running emberAfMainInitCallback()<<\r\n");      //By Ray 150120

    if(emberState == E_STATE_INIT){

    }

    //newTime = halCommonGetInt16uMillisecondTick();

      //every 500ms to check LED if needed to toggle
//    if( toggleLED1 && elapsedTimeInt16u(lastBlinkTime, newTime) > 500 /*ms*/) 
//    {
//        halToggleLed(BOARDLED1);
//        lastBlinkTime = newTime;
//    }
}

//	void emberAfHalButtonIsrCallback(int8u button, int8u state)
//	{ 
//	    boolean bSend = FALSE;
//	    //int value;
//	    EmberNodeType nodeType; 
//	    EmberNetworkParameters netParameters;
//	    
//	    EmberStatus status = emberAfGetNetworkParameters(&nodeType, &netParameters); 
//	
//	    if(button == BUTTON0){  //BUTTON0 is TAMPER_SW By Ray 150122
//	        emberAfCorePrint(">> BUTTON0 is TAMPER_SW By Ray <<\r\n");
//	        bSend = TRUE;        
//	    }else if(button == BUTTON1){ //BUTTON1 is MAGNETIC_SW By Ray 150122
//	        emberAfCorePrint(">> BUTTON1 is MAGNETIC_SW By Ray <<\r\n");
//	        bSend = TRUE; 
//	    }
//	
//	    
//	    if(bSend){
//	        emberAfCorePrint(">>channel [%d] pwr [%d] panID [0x%2x]<<\r\n",
//	                            netParameters.radioChannel,
//	                            netParameters.radioTxPower,
//	                            netParameters.panId
//	                           );
//	    }
//	}


/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned in
 * the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Get Radio Power For Channel
 *
 * This callback is called by the framework when it is setting the radio power
 * during the discovery process. The framework will set the radio power
 * depending on what is returned by this callback.
 *
 * @param channel   Ver.: always
 */
int8s emberAfPluginNetworkFindGetRadioPowerForChannelCallback(int8u channel)
{
  return EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER;
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been found.
 *  If the application returns TRUE, the plugin will attempt to join the
 * network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
boolean emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork * networkFound,
                                             int8u lqi,
                                             int8s rssi)
{
  return TRUE;
}


