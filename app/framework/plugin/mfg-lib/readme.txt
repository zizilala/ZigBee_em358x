MFG-LIB to support manufacturing test:
----------------------------------------------------
This plugin will help create CLI options for driving the manufacturing library. 

Instructions:
1) Drop these files into your 5.1.0 release under the directory app/framework/plugins/pwm-control\
2) Create a project.  Include the mfg-lib plugin.
3) generate the project
4) remove the buzzer.c file from the project.
5) add the library mfg-lib.a.  It is located in build/mfg-lib-XXX/mfg-lib.a

TODO
1) token to disable other functionality until mfg-lib has been completed
2) cli to enable/disable this token and reset the device
