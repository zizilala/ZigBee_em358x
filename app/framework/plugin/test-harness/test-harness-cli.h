
#if defined(EMBER_AF_PLUGIN_TEST_HARNESS)

  #define TEST_HARNESS_CLI_COMMANDS \
    emberCommandEntrySubMenu( "test-harness", emberAfPluginTestHarnessCommands, "Commands for acting like a test harness."),
#else
  #define TEST_HARNESS_CLI_COMMANDS
#endif 

