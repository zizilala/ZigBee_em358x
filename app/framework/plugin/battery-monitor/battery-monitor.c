// *****************************************************************************
// * battery-monitor.c
// *
// * Code to implement a battery monitor.
// *
// * This code will read the battery voltage during a transmission (in theory 
// * when we are burning the most current), and update the battery voltage 
// * attribute in the power configuration cluster.
// *
// *
// * Copyright 2014 by Silicon Labs. All rights reserved.                   *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"


//------------------------------------------------------------------------------
// Forward Declaration
void initializeTxActiveIrq( void );

//------------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginBatteryMonitorBatteryMonitorEventControl;

#define batteryEvent         emberAfPluginBatteryMonitorBatteryMonitorEventControl

#define BATTERY_MONITOR_TIMEOUT  EMBER_AF_PLUGIN_BATTERY_MONITOR_MONITOR_TIMEOUT

// init event
void emberAfPluginBatteryMonitorInitCallback(void)
{
  initializeTxActiveIrq();
}

// battery monitor code
// This code will grab the battery voltage during a transmit event.  
// Note:  for this to work, PC5 must be set to be an alt-out push/pull,
// and the PHY_CONFIG token needs to be set to 0xFFFB (if there is no
// PA, or if the PA is on the bidirectional port) or 0xFFFD (if there 
// is a PA).  
/* 
 * need PC5 to be alternate output, push/pull
 * need to set PHY_CONFIG token to eitehr 0xFFFB (for non-PA) or 0xFFFD
   (for PA).
 * need to set up PC5 as button 2.
 * need to set up PC5 as a wake from sleep interrupt (to overcome a 
   strange behavior in the HAL layer where non-wake interrupts get
   unset).
 * need to have the battry voltage 
 */
static int32u lastBatteryMeasureTick = 0xe0000000; // take reading after reset.
#define MS_BETWEEN_BATTERY_CHECK (60 * 1000 * BATTERY_MONITOR_TIMEOUT)  // convert to milliseconds
//#define MS_BETWEEN_BATTERY_CHECK 1000 // whenever we transmit poll.  can get annyoing.

// implement 16 sample fifo
#define FIFO_SIZE 16
static int8u samplePtr = 0;
static int16u voltageFifo[FIFO_SIZE];
static boolean fifoInitialized = FALSE;

void printFifo( void )
{
  int8u i;

  for(i=0; i<FIFO_SIZE; i++) {
    emberSerialPrintf(APP_SERIAL,"%d ", voltageFifo[i]);
  }
  emberSerialPrintf(APP_SERIAL,"\r\n");
}

int16u filterVoltageSample(int16u sample)
{
  int16u voltageSum;
  int8u i;

  if(fifoInitialized) {
    voltageFifo[samplePtr++] = sample;

    if(samplePtr >= FIFO_SIZE) {
      samplePtr = 0;
    }
    voltageSum = 0;
    for(i=0; i<FIFO_SIZE; i++) {
      voltageSum += voltageFifo[i];
    }
    sample = voltageSum / FIFO_SIZE;
  } else {
    for(i=0; i<FIFO_SIZE; i++) {
      voltageFifo[i] = sample;
    }
    fifoInitialized = TRUE;
  } 

  //printFifo();

  return sample;
}

void emberAfPluginBatteryMonitorBatteryMonitorEventHandler( void )
{
  emberEventControlSetInactive( batteryEvent );

//	  int16u voltage, filteredVoltage;      //By Ray 150203
  int16u voltage;
  int8u voltage8u;
  int32u currentMsTick = halCommonGetInt32uMillisecondTick();
  int32u timeSinceLastMeasure = currentMsTick - lastBatteryMeasureTick;

//	  int32u voltageAccumulator = 0;        //By Ray 150203
//	  int8u i;                              //By Ray 150203

emberSerialPrintf(APP_SERIAL,"current tick %4x, last tick %4x, delta %4x\r\n",
currentMsTick, lastBatteryMeasureTick, timeSinceLastMeasure);

  if(timeSinceLastMeasure < MS_BETWEEN_BATTERY_CHECK) {
  } else {
    // length of a data poll = 512 uS
    voltage = halMeasureVdd(ADC_CONVERSION_TIME_US_256);  

    // filter the voltage
    voltage = filterVoltageSample(voltage);
    
    emberSerialPrintf(APP_SERIAL,"Transmit battery sample:%d %d\r\n", 
      voltage, currentMsTick);

    // convert from mV to 100 mV.  Also, change number of bits from 16 to 8
    voltage8u = ((int8u) (voltage / 100));
    emberAfWriteAttribute(1, // endpoint
                          ZCL_POWER_CONFIG_CLUSTER_ID,
                          ZCL_BATTERY_VOLTAGE_ATTRIBUTE_ID,
                          CLUSTER_MASK_SERVER,
                          &voltage8u,
                          ZCL_INT8U_ATTRIBUTE_TYPE);

    lastBatteryMeasureTick = currentMsTick;
  }
}

// ******  battery read ISR handling
void isrBatteryIface( void )
{
  emberEventControlSetActive( batteryEvent );
}

#define TX_ACTIVE_PIN PORTC_PIN(5)
#define TX_ACTIVE_IN  GPIO_PCIN
#define TX_ACTIVE_INTCFG      GPIO_INTCFGD
#define TX_ACTIVE_INT_EN_BIT  INT_IRQD
#define TX_ACTIVE_FLAG_BIT    INT_IRQDFLAG
#define TX_ACTIVE_MISS_BIT    INT_MISSIRQD

int8u batteryGPIOPinState(int8u button) 
{
  return (TX_ACTIVE_IN & BIT(TX_ACTIVE_PIN&7));
}

void initializeTxActiveIrq( void )
{
  emberSerialPrintf(APP_SERIAL,"Init IRQD\r\n");

  //start from a fresh state just in case
  TX_ACTIVE_INTCFG = 0;              //disable TX_ACTIVE triggering
  INT_CFGCLR = TX_ACTIVE_INT_EN_BIT; //clear TX_ACTIVE top level int enable
  INT_GPIOFLAG = TX_ACTIVE_FLAG_BIT; //clear stale TX_ACTIVE interrupt
  INT_MISS = TX_ACTIVE_MISS_BIT;     //clear stale missed TX_ACTIVE interrupt
  //configure TX_ACTIVE
  do { GPIO_IRQDSEL = TX_ACTIVE_PIN; } while(0);     //point IRQ at the desired pin

  TX_ACTIVE_INTCFG  = (0 << GPIO_INTFILT_BIT); //no filter
  TX_ACTIVE_INTCFG |= (3 << GPIO_INTMOD_BIT);  //3 = both edges

  INT_CFGSET = TX_ACTIVE_INT_EN_BIT; //set top level interrupt enable
}

void halIrqDIsr(void)
{
halToggleLed(BOARDLED0);

  //clear int before read to avoid potential of missing interrupt
  INT_MISS = TX_ACTIVE_MISS_BIT;     //clear missed BUTTON2 interrupt flag
  INT_GPIOFLAG = TX_ACTIVE_FLAG_BIT; //clear top level BUTTON2 interrupt flag

  if(batteryGPIOPinState(TX_ACTIVE_PIN)) {
    // pin is high, time to call the ISR
    isrBatteryIface();
  }
}
