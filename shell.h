/**
 *  \file shell.h
 *  \brief Arduino Shell header file.
 *  This shell reads data from serial port and execute the commands received using the client-server paradigm
 */

#ifndef _SHELL_h
#define _SHELL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//////////////////////////////////////////////////////////////////////////
///
///	Defines Section
///
//////////////////////////////////////////////////////////////////////////
/// \brief Constant Definition
/// \param NULL NULL keyword definition
/// \param SHELL_MAX_LINE_LENGTH max length of the input buffer
/// \param SHELL_MAX_ARGUMENTS max number of arguments of the commands (not used)
/// \param SHELL_PROMPT prompt string
/// \param FW_VERSION version of the Arduino firmware
/// \param OS_VERSION version of the NilRTOS
/// \param CR shell end of line string
/// \param CMD_STRING_LEN lenght of the command string buffer
/// \param PROMPT shell prompt
/// \param SOL_LED Pin number of the sign of life LED
/// \param USE_SHELL_THREAD activate/deactivate the shell code as NilRTOS thread (deactivated)
///
#define NULL						0					//	NULL definition (error trying to include from elsewhere
#define SHELL_MAX_LINE_LENGTH       32					//	Shell input buffer length
#define SHELL_MAX_ARGUMENTS         4					//	Shell MAX number of arguments
#define SHELL_PROMPT                "AVR> "				//	Shell prompt
#define FW_VERSION                  "0.4.6"				//	Firmware Revision
#define OS_VERSION                  NIL_KERNEL_VERSION	//	OS type and revision
#define CR							"\r\n"				//	Shell EOL characters
#define CMD_STRING_LEN				32					//	Command String Lenght
#define PROMPT						"AVR> "				//	Shell prompt
#define SOL_LED						13					//	Sign of Life LED
#undef USE_SHELL_THREAD									//	Activate/Deactivate the use of the shell as thread

//	Debug Activation/Deactivation
#undef DEBUG

//////////////////////////////////////////////////////////////////////////
///
///	Shell Variables Section
///
//////////////////////////////////////////////////////////////////////////
//extern String cmdString = "";							//        Line Command string buffer
//extern boolean cmdReady = false;							//        Flag indicating that a command is ready
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

/// \brief Shell Command Data type:
/// Struct composed by the name of the command and a function prototype for the action to execute
/// \param char * sc_name: command name
/// \param shellcmd_t sc_function: pointer to the function implementing the command
///
///
typedef struct
{
	const char *sc_name;
	shellcmd_t  sc_function;
} ShellCommand_t;

/// \brief Shell Configuration data type (don't think it is used)
///
/// \param
/// \param
///
///
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

/// \brief Definition of the shell commands:
/// This data structure links the command to the corresponding action function
/// \param <command ASCII friendly> char *  String defining the command
/// \param <command function> ShellCommand_t * Function executing the command
///
///
static ShellCommand_t ShellCommands[] =
{
	{
		"get_ACK",   SendACK
	},
	{
		"pos", getPosition
	},
	{
		"set_pos", setPosition
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
	{
		"get_state", getState
	},
	{
		"gear_config", gearCfg 
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

//////////////////////////////////////////////////////////////////////////
///
///	Function Prototype Section
///
//////////////////////////////////////////////////////////////////////////
//void ShellThread (void *p);
char * Strtok(char *str, const char *delim, char **saveptr);                    //  string.h strtok implementation
void Usage(char *str);                                                          //  Usage of a command
int CmdExec(const ShellCommand_t *scp, char *name, int argc, char *argv[]);     //  Command Execute
void ListCommands(ShellCommand_t *scp);                                         //  List implemented commands
void printDouble(double val, byte precision);                                   //  Double to ASCII on serial port
void printFloat(float value, int places);                                       //  Float to ASCII on serial port
void avrPrintf(const char * str);                                               //  Send a string on serial port
void avrPrintf(const int val);                                                  //  Send an integer on serial port
void avrPrintf(const uint16_t val);
void avrPrintf(const uint32_t val);                                             //  Send a long on serial port
void avrPrintf(const double val);                                               //  Send a double on serial port
void ShellTask(void *p, char *line);                                            //  Shell Task
void CDC_Task();                                                                //  USB CDC task


#endif

