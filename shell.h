// shell.h

#ifndef _SHELL_h
#define _SHELL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//
//        Definitions sections
//
#define NULL						0					//	NULL definition (error trying to include from elsewhere
#define SHELL_MAX_LINE_LENGTH       16					//	Shell input buffer length
#define SHELL_MAX_ARGUMENTS         4					//	Shell MAX number of arguments
#define SHELL_PROMPT                "AVR> "				//	Shell prompt
#define FW_VERSION                  "0.4.0"				//	Firmware Revision
#define OS_VERSION                  NIL_KERNEL_VERSION	//	OS type and revision
#define CR							"\r\n"				//	Shell EOL characters
#define CMD_STRING_LEN				16					//	Command String Lenght
#define PROMPT						"AVR> "				//	Shell prompt
#define SOL_LED						13					//	Sign of Life LED
#undef USE_SHELL_THREAD									//	Activate/Deactivate the use of the shell as thread

//	Debug Activation/Deactivation
#undef DEBUG

//
//	Shell Variable Section
//
//extern String cmdString = "";							//        Line Command string buffer
//extern boolean cmdReady = false;						//        Flag indicating that a command is ready
//extern uint8_t inBufCount = 0;							//        Input buffer char counter

//        Function prototype for the action executed by the shell
typedef void (* shellcmd_t)(int argc, char *argv[]);

//
//        Actions to be executed by the shell defined in different files
//
extern void getPosition(int argc, char *argv[]);				//        Get Encoder Position
extern void TurnLeft(int argc, char *argv[]);					//        Turn Dome to the left
extern void TurnRight(int argc, char *argv[]);					//        Turn Dome to the right
extern void Stop(int argc, char *argv[]);						//        Stop turning the Dome
/*
extern void vPwmStop(int argc, char *argv[]);                     //        Stop a PWM
extern void vPwmSet(int argc, char *argv[]);                      //        Set the level of a PWM
*/

//
//        Shell Command Data type:
//        Struct composed by the name of the command and a function prototype for the action to execute
//
typedef struct
{
	const char *sc_name;
	shellcmd_t  sc_function;
} ShellCommand_t;

//
//        Shell Configuration data type (don't think it is used)
//
typedef struct
{
	const ShellCommand_t *sc_command;
} ShellConfig_t;

//
//        Local Actions of the Shell
//
void SendACK(int argc, char *argv[]);                                        //        Send back a ACK to the PC
void CmdInfo(int argc, char *argv[]);                                        //        Get back the info about the FW revision
void CmdSystime(int argc, char *argv[]);                                //        Command system time (not implemented)

//xShellCommand_t xLocalCommands[] =
//{
//{
//"info", vCmdInfo
//},
//{
//"systime", vCmdSystime
//},
//{
//NULL, NULL
//}
//};

//
//        Definition of the shell commands:
//        This data structure links the command to the corresponding action function
//
static ShellCommand_t ShellCommands[] =
{
	{
		"get_ACK",   SendACK
	},
	{
		"pos", getPosition
	},
	{
		"turn_left", TurnLeft
	},
	{
		"turn_right", TurnRight
	},
	{
		"stop", Stop
	},
/*	{
		"adc_init", ADCInit
	},
	{
		"adc_get", getADC_Result
	},
*/	{
		NULL, NULL
		//(char *)0, (void **)0
	}
} ;

/*
//        Configuration data structure
ShellConfig_t ShellConfig;
*/

//
//        Function prototypes for the shell.c
//
//void ShellThread (void *p);
char * Strtok(char *str, const char *delim, char **saveptr);
void Usage(char *str);
int CmdExec(const ShellCommand_t *scp, char *name, int argc, char *argv[]);
void ListCommands(ShellCommand_t *scp);
void avrPrintf(const char * str);
void avrPrintf(const int val);
void avrPrintf(const uint32_t val);
void avrPrintf(const double val);
void ShellTask(void *p, char *line);
void CDC_Task();

//class shell
//{
 //private:
//
//
 //public:
	//void init();
//};
//
//extern shell SHELL;

#endif

