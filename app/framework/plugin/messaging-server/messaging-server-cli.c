// *******************************************************************
// * messaging-server-cli.c
// *
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/messaging-server/messaging-server.h"

void emAfMessagingServerCliMsg(void);
void emAfMessagingServerCliAppend(void);
void emAfMessagingServerCliId(void);
void emAfMessagingServerCliTime(void);
void emAfMessagingServerCliRelativeTime(void);
void emAfMessagingServerCliTransmission(void);
void emAfMessagingServerCliImportance(void);
void emAfMessagingServerCliConfirm(void);
void emAfMessagingServerCliValid(void);
void emAfMessagingServerCliDisplay(void);
void emAfMessagingServerCliCancel(void);
void emAfMessagingServerCliPrint(void);

static EmberAfPluginMessagingServerMessage message;

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginMessagingServerTransmissionCommands[] = {
  emberCommandEntryAction("normal",  emAfMessagingServerCliTransmission, "", ""),
  emberCommandEntryAction("ipan",  emAfMessagingServerCliTransmission, "", ""),
  emberCommandEntryAction("both",  emAfMessagingServerCliTransmission, "", ""),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emberAfPluginMessagingServerImportanceCommands[] = {
  emberCommandEntryAction("low",  emAfMessagingServerCliImportance, "", ""),
  emberCommandEntryAction("medium",  emAfMessagingServerCliImportance, "", ""),
  emberCommandEntryAction("high",  emAfMessagingServerCliImportance, "", ""),
  emberCommandEntryAction("critical",  emAfMessagingServerCliImportance, "", ""),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emberAfPluginMessagingServerConfirmCommands[] = {
  emberCommandEntryAction("not",  emAfMessagingServerCliConfirm, "", ""),
  emberCommandEntryAction("req",  emAfMessagingServerCliConfirm, "", ""),
  emberCommandEntryTerminator(),
};

EmberCommandEntry emberAfPluginMessagingServerCommands[] = {
  emberCommandEntryAction("message",  emAfMessagingServerCliMsg, "b", ""),
  emberCommandEntryAction("append",  emAfMessagingServerCliAppend, "b", ""),
  emberCommandEntryAction("id",  emAfMessagingServerCliId, "w", ""),
  emberCommandEntryAction("time",  emAfMessagingServerCliTime, "wv", ""),
  emberCommandEntryAction("relative-time",  emAfMessagingServerCliRelativeTime, "sv", ""),
  emberCommandEntryAction("transmission",  NULL, (PGM_P)emberAfPluginMessagingServerTransmissionCommands, ""),
  emberCommandEntryAction("importance",  NULL, (PGM_P)emberAfPluginMessagingServerImportanceCommands, ""),
  emberCommandEntryAction("confirm",  NULL, (PGM_P)emberAfPluginMessagingServerConfirmCommands, ""),
  emberCommandEntryAction("valid",  emAfMessagingServerCliValid, "u", ""),
  emberCommandEntryAction("invalid",  emAfMessagingServerCliValid, "u", ""),
  emberCommandEntryAction("display",  emAfMessagingServerCliDisplay, "vuu", ""),
  emberCommandEntryAction("cancel",  emAfMessagingServerCliCancel, "vuu", ""),
  emberCommandEntryAction("print", emAfMessagingServerCliPrint, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin messaging-server message <message string>
void emAfMessagingServerCliMsg(void)
{
  int8u length = emberCopyStringArgument(0,
                                         message.message + 1,
                                         EMBER_AF_PLUGIN_MESSAGING_SERVER_MESSAGE_SIZE,
                                         FALSE);
  message.message[0] = length;
}

// plugin messaging-server append <message string>
void emAfMessagingServerCliAppend(void)
{
  int8u oldLength = message.message[0];
  int8u length = emberCopyStringArgument(0,
                                         message.message + oldLength + 1,
                                         (EMBER_AF_PLUGIN_MESSAGING_SERVER_MESSAGE_SIZE
                                          - oldLength),
                                         FALSE);
  message.message[0] = oldLength + length;
}

// plugin messaging-server id <messageId:4>
void emAfMessagingServerCliId(void)
{
  message.messageId = emberUnsignedCommandArgument(0);
}

// plugin messaging-server time <start time:4> <duration:2>
void emAfMessagingServerCliTime(void)
{
  message.startTime = emberUnsignedCommandArgument(0);
  message.durationInMinutes = (int16u)emberUnsignedCommandArgument(1);
}

// plugin messaging-server relative-time <+/-time> <duration>

// Rather than use absolute time, this will set the start-time relative to the current time +/-
// the CLI parameter in MINUTES.
void emAfMessagingServerCliRelativeTime(void)
{
  message.startTime = (emberAfGetCurrentTime()
                       + (emberSignedCommandArgument(0) * 60));
  message.durationInMinutes = (int16u)emberUnsignedCommandArgument(1);
}

// plugin messaging-server transmission <normal | ipan | both>
void emAfMessagingServerCliTransmission(void)
{
  int8u commandChar = emberCurrentCommand->name[0];
  message.messageControl &= ~ZCL_MESSAGING_CLUSTER_TRANSMISSION_MASK;
  if (commandChar == 'b') { // both
    message.messageControl |= EMBER_ZCL_MESSAGING_CONTROL_TRANSMISSION_NORMAL_AND_ANONYMOUS;
  } else if (commandChar == 'i') { // inter pan
    message.messageControl |= EMBER_ZCL_MESSAGING_CONTROL_TRANSMISSION_ANONYMOUS;
  }
  // Do nothing for 'normal'.
}

// plugin messaging-server importance <low | medium | high | critical>
void emAfMessagingServerCliImportance(void)
{
  int8u commandChar = emberCurrentCommand->name[0];
  message.messageControl &= ~ZCL_MESSAGING_CLUSTER_IMPORTANCE_MASK;
  if (commandChar == 'm') { // medium
    message.messageControl |= EMBER_ZCL_MESSAGING_CONTROL_IMPORTANCE_MEDIUM;
  } else if (commandChar == 'h') { // high
    message.messageControl |= EMBER_ZCL_MESSAGING_CONTROL_IMPORTANCE_HIGH;
  } else if (commandChar == 'c') { // critical
    message.messageControl |= EMBER_ZCL_MESSAGING_CONTROL_IMPORTANCE_CRITICAL;
  }
  // Do nothing for 'low' importance.
}

// plugin messaging-server confirm <not | req>
void emAfMessagingServerCliConfirm(void)
{
  int8u commandChar = emberCurrentCommand->name[0];
  message.messageControl &= ~ZCL_MESSAGING_CLUSTER_CONFIRMATION_MASK;
  if (commandChar == 'r') { // required
    message.messageControl |= EMBER_ZCL_MESSAGING_CONTROL_CONFIRMATION_REQUIRED;
  }
  // Do nothing for 'not' (not required).
}

// plugin messaging-server <valid | invalid>
void emAfMessagingServerCliValid(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emberAfPluginMessagingServerSetMessage(endpoint,
                                         emberCurrentCommand->name[0] == 'v'
                                         ? &message
                                         : NULL);
}

// plugin messaging-server display <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void emAfMessagingServerCliDisplay(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  int8u srcEndpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u dstEndpoint = (int8u)emberUnsignedCommandArgument(2);
  emberAfPluginMessagingServerDisplayMessage(nodeId,
                                             srcEndpoint,
                                             dstEndpoint);
}

// plugin messaging-server cancel <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void emAfMessagingServerCliCancel(void)
{
  EmberNodeId nodeId = (EmberNodeId)emberUnsignedCommandArgument(0);
  int8u srcEndpoint = (int8u)emberUnsignedCommandArgument(1);
  int8u dstEndpoint = (int8u)emberUnsignedCommandArgument(2);
  emberAfPluginMessagingServerCancelMessage(nodeId,
                                            srcEndpoint,
                                            dstEndpoint);
}

// plugin messaging-server print <endpoint:1>
void emAfMessagingServerCliPrint(void)
{
  int8u endpoint = (int8u)emberUnsignedCommandArgument(0);
  emAfPluginMessagingServerPrintInfo(endpoint);
}
