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


extern void emberAfLedOn( int8u time );

extern void emberAfLedOff( int8u time );

extern void emberAfLedBlink( int8u count, int16u blinkTime );

// 
// function to set the GPIO and maintain the state during sleep.
// Port is 0 for port a, 1 for port b, and 2 for port c.
extern void emSleepySetGpio( int8u port, int8u pin );

// 
// function to clear the GPIO and maintain the state during sleep.
// Port is 0 for port a, 1 for port b, and 2 for port c.
extern void emSleepyClearGpio( int8u port, int8u pin );
