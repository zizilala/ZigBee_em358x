// ****** LED blinking code
//
// This code provides a few APIs:
// emberAfLedOn( time )
//       time:  the amount of time in seconds for the LED to be on.  
//              will stay on if the time is 0.
// emberAfLedOff( time )
//       time:  the amount of time in seconds for the LED to be off.  
//              will stay on if the time is 0.
// emberAfLedBlink( count )
//       count:  the number of times for the LED to blink.  0 means 
//               keep blinking until further notice.

#include "app/framework/include/af.h"
#include "hal/micro/micro.h"
// useful defines from led.c
#define GPIO_PxCLR_BASE (GPIO_PACLR_ADDR)
#define GPIO_PxSET_BASE (GPIO_PASET_ADDR)
#define GPIO_PxOUT_BASE (GPIO_PAOUT_ADDR)
// Each port is offset from the previous port by the same amount
#define GPIO_Px_OFFSET  (GPIO_PBCFGL_ADDR-GPIO_PACFGL_ADDR)

int8u ledBlinkState;

//EmberEventControl ledEventControl;
EmberEventControl emberAfPluginLedBlinkLedEventFunctionEventControl;

#define ledEventControl emberAfPluginLedBlinkLedEventFunctionEventControl
#define ledEventFunction emberAfPluginLedBlinkLedEventFunctionEventHandler

void ledEventFunction( void );

static int8u ledEventState = 0x00;
static int8u ledBlinkCount = 0x00;
static int16u ledBlinkTime;

enum {
  LED_ON            = 0x00,
  LED_OFF           = 0x01,
  LED_BLINKING_ON   = 0x02,
  LED_BLINKING_OFF  = 0x03,
  LED_BLINK_PATTERN = 0x04,
};

extern int8u gpioOutPowerDown[];
extern int8u gpioOutPowerUp[];

void setBit(int8u *data, int8u bit)
{
  int8u mask = 0x01 << bit;

  *data = *data | mask;
}

void clearBit(int8u *data, int8u bit)
{
  int8u mask = ~(0x01 << bit);

  *data = *data & mask;
}

void emSleepySetGpio( int8u port, int8u pin )
{
  assert( port < 3 );
  assert( pin < 8 );

  *((volatile int32u *)(GPIO_PxSET_BASE+(GPIO_Px_OFFSET*(port)))) = BIT(pin);

  // Make sure this stays set during the next power cycle
  setBit(&(gpioOutPowerUp[port]), pin);
  setBit(&(gpioOutPowerDown[port]), pin);
}

void emSleepyClearGpio( int8u port, int8u pin )
{
  assert( port < 3 );
  assert( pin < 8 );

  *((volatile int32u *)(GPIO_PxCLR_BASE+(GPIO_Px_OFFSET*(port)))) = BIT(pin);

  // Make sure this stays clear during the next power cycle
  clearBit(&(gpioOutPowerUp[port]), pin);
  clearBit(&(gpioOutPowerDown[port]), pin);
}

static void turnLedOn( int8u led )
{
  int8u port = (led) >> 3;
  int8u pin = (led) & 0x07;

  emSleepyClearGpio( port, pin );

}

static void turnLedOff( int8u led )
{

  int8u port = (led) >> 3;
  int8u pin = (led) & 0x07;

  emSleepySetGpio( port, pin );

}

void emberAfLedOn( int8u time )
{
  turnLedOn(BOARDLED3);
  ledEventState = LED_ON;

  if(time > 0) {
    emberEventControlSetDelayQS(ledEventControl,
                                ((int16u) time) * 4);
  } else {
    emberEventControlSetInactive(ledEventControl);
  }
}    

void emberAfLedOff( int8u time )
{
  turnLedOff(BOARDLED3);
  ledEventState = LED_OFF;

  if(time > 0) {
    emberEventControlSetDelayQS(ledEventControl,
                                ((int16u) time) * 4);
  } else {
    emberEventControlSetInactive(ledEventControl);
  }
}

void emberAfLedBlink( int8u count, int16u blinkTime )
{
  ledBlinkTime = blinkTime;

  turnLedOff(BOARDLED3);
  ledEventState = LED_BLINKING_OFF;
  emberEventControlSetDelayMS(ledEventControl,
                              ledBlinkTime);
  ledBlinkCount = count;
}

int16u blinkPattern[20];
int8u blinkPatternLength;
int8u blinkPatternPointer;

void emberAfLedBlinkPattern( int8u count, int8u length, int16u *pattern )
{
  int8u i;

  if(length < 2) {
    return;
  }

  turnLedOn(BOARDLED3);

  ledEventState = LED_BLINK_PATTERN;

  if(length > 20) {
    length = 20;
  }

  blinkPatternLength = length;
  ledBlinkCount = count;

  for(i=0; i<blinkPatternLength; i++) {
    blinkPattern[i] = pattern[i];
  }

  emberEventControlSetDelayMS(ledEventControl,
                              blinkPattern[0]);
  
  blinkPatternPointer = 1;
}


void ledEventFunction( void )
{
  switch(ledEventState) {
  case LED_ON:
    // was on.  this must be time to turn it off.
    turnLedOff(BOARDLED3);
    emberEventControlSetInactive(ledEventControl);
    break;

  case LED_OFF:
    // was on.  this must be time to turn it off.
    turnLedOn(BOARDLED3);
    emberEventControlSetInactive(ledEventControl);
    break;

  case LED_BLINKING_ON:
    turnLedOff(BOARDLED3);
    if(ledBlinkCount > 0) {
      if(ledBlinkCount != 255) { // blink forever if count is 255
        ledBlinkCount --;
      }
      if (ledBlinkCount > 0) {
        ledEventState = LED_BLINKING_OFF;
        emberEventControlSetDelayMS(ledEventControl,
                                    ledBlinkTime);

      } else {
        ledEventState = LED_OFF;
        emberEventControlSetInactive(ledEventControl);
      } 
    } else {
      ledEventState = LED_BLINKING_OFF;
      emberEventControlSetDelayMS(ledEventControl,
                                  ledBlinkTime);
    }
    break;
  case LED_BLINKING_OFF:
    turnLedOn(BOARDLED3);
    ledEventState = LED_BLINKING_ON;
    emberEventControlSetDelayMS(ledEventControl,
                                ledBlinkTime);
    break;
  case LED_BLINK_PATTERN:
    if(ledBlinkCount == 0) {
      turnLedOff(BOARDLED3);

      ledEventState = LED_OFF;
      emberEventControlSetInactive(ledEventControl);

      break;
    }

    if(blinkPatternPointer %2 == 1) {
      turnLedOff(BOARDLED3);
    } else {
      turnLedOn(BOARDLED3);
    }

    emberEventControlSetDelayMS(ledEventControl,
                              blinkPattern[blinkPatternPointer]);
  
    blinkPatternPointer ++;

    if(blinkPatternPointer >= blinkPatternLength) {
      blinkPatternPointer = 0;

      if(ledBlinkCount != 255) { // blink forever if count is 255
        ledBlinkCount --;
      } 
    }

  default:
    break;
  }
}
