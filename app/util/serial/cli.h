// *******************************************************************
// cli.h
//
// Simple command line interface (CLI) for use with Ember applications.
// The application defines a list of top-level commands and functions
// to be called when those commands are used. The application also
// defines a prompt to use. Then the application can parse arguments
// passed to the top-level commands using the utility functions provided
// for comparing strings, getting integer values, and hex values. See below
// for an example. 
//
// The cli library is deprecated and will be removed in a future release.
//
// EXAMPLE:
// ------------------------------
// // define the functions called for the top-level cmds: version, reset, print 
// void versionCB(void)
//   {  emberSerialPrintf(APP_SERIAL, "version 0.1\r\n"); }
// 
// void resetCB(void)
//   { while (1) ; }
// 
// // the commands accepted are "print info", "print child", and 
// // "print bindings" 
// void printCB(void)
// {
//   if (compare("info",4,1) == TRUE) {
//     printInfo();
//   }
//   else if (compare("child",5,1) == TRUE) {
//     printChildTable();
//   }
//   else if (compare("bindings",8,1) == TRUE) {
//     printBindingTableUtil(APP_SERIAL, EMBER_BINDING_TABLE_SIZE);
//   }
// }
//
// // match top-level commands to the functions they will call
// cliSerialCmdEntry cliCmdList[] = {
//   {"version", versionCB},
//   {"reset", resetCB},
//   {"print", printCB}
// };
// int8u cliCmdListLen = sizeof(cliCmdList)/sizeof(cliSerialCmdEntry);
//
// // define the prompt
// PGM_P cliPrompt = "qa-host";
//
// // initialize the cli by setting the port
// cliInit(APP_SERIAL);
//
//  Copyright 2007 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include PLATFORM_HEADER //compiler/micro specifics, types

// The CONFIGURATION_HEADER is included to allow the user to change the 
// maximim number of arguments accepted and/or the maximim size of cmds.
// This is done by setting the defines: CLI_MAX_NUM_SERIAL_ARGS and/or 
// CLI_MAX_SERIAL_CMD_LINE
#ifdef CONFIGURATION_HEADER
  #include CONFIGURATION_HEADER 
#endif

// *********************************************
// apps must initialize cli with a serial port before using it
void cliInit(int8u serialPort);

// apps should call this from their main loop
// this processes characters from the serial port
void cliProcessSerialInput(void);


// *********************************************
// Constants for the size of the largest argument and the number of
// arguments accepted. These can be defined by the application in the
// CONFIGURATION_HEADER to fit the applications needs. Decreasing 
// these reduces RAM usage. Increasing these increases RAM usage.
#ifndef CLI_MAX_SERIAL_CMD_LINE
  #define CLI_MAX_SERIAL_CMD_LINE  17
#endif
#ifndef CLI_MAX_NUM_SERIAL_ARGS  
  #define CLI_MAX_NUM_SERIAL_ARGS  8
#endif


// *********************************************
// these are utility functions to help the application parse the cmd line

// this returns the nth byte from an argument entered as a hex string
// as a byte value
int8u cliGetHexByteFromArgument(int8u whichByte, 
                                int8u argument);

// this returns an int16s from an argument entered as a string in decimal
int16s cliGetInt16sFromArgument(int8u bufferIndex);

// this returns an int16u from an argument entered as a string in decimal
int16u cliGetInt16uFromArgument(int8u argument);

// this returns an int32u from an argument entered as a string in decimal
int32u cliGetInt32uFromArgument(int8u argument);

// returns an int16u from an argument entered as a hex string
int16u cliGetInt16uFromHexArgument(int8u index);

// returns an int32u from an argument entered as a hex string
int32u cliGetInt32uFromHexArgument(int8u index);

// this returns TRUE if the argument specified matches the keyword provided
boolean cliCompareStringToArgument(PGM_P keyword, 
                                   int8u argument);

// This copies the string at the argument specified into the bufferToFill.
// The bufferToFill must already point to initialized memory and the maximum
// length of this buffer should be maxBufferToFillLength. If
// maxBufferToFillLength is smaller than the string being copied into
// bufferToFill, no copy will be done and a length of zero will be returned.
// If maxBufferToFillLength is larger than the string being copied in,
// the copy will be done and the length of the copied string will be returned. 
int8u cliGetStringFromArgument(int8u argument, 
                               int8u* bufferToFill,
                               int8u maxBufferToFillLength);


// *********************************************
// This is the structure that the application uses to enter
// commands into the command parser. This should not be changed.
typedef PGM struct cliSerialCmdEntryS {
  PGM_P cmd;
  void (*action)(void);
} cliSerialCmdEntry;

// *********************************************
// the application needs to define these:
// --------------------------------------
extern cliSerialCmdEntry cliCmdList[];
extern int8u cliCmdListLen;
extern PGM_P cliPrompt;





