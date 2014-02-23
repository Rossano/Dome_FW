/**
 *  \file shell.cpp
 *  \brief Arduino Shell Code.
 *  This shell reads data from serial port and execute the commands received using the client-server paradigm
 */

//////////////////////////////////////////////////////////////////////////
///
///	Include Section
///
//////////////////////////////////////////////////////////////////////////

/// Standard Inclusion headers
#include <string.h>
#include <stdio.h>

///        My included file and headers
#include <stddef.h>
#include "encoder.h"
#include "dome.h"

///        Shell own inclusion header
#include "shell.h"

//////////////////////////////////////////////////////////////////////////
///
///	Defines Section
///
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
///
///	Variable Section
///
//////////////////////////////////////////////////////////////////////////

/** \brief Line Command string buffer
 *
 * \return Input string from USB
 *
 */
String cmdString = "";							///        Line Command string buffer

/** \brief Flag indicating that a command is ready
 *
 * \return false command not ready yet, true command is ready to be consumed
 *
 */
boolean cmdReady = false;						///        Flag indicating that a command is ready

/** \brief Input buffer char counter
 *
 */
uint8_t inBufCount = 0;							///        Input buffer char counter
//        ???
static boolean bEnd = false;

/** \brief Shell Local command definition data structure
 *
 * \param info: command to send firmware information
 * \param systime: system time (not yet implemented)
 *
 */
static ShellCommand_t LocalCommands[] =
{
	{
		"info", CmdInfo
	},
	{
		"systime", CmdSystime
	},
	{
		NULL, NULL
	}
};

/** \brief Semaphore to synchronize access to serial port
 *
 */
SEMAPHORE_DECL(SerialOutSem, 1);

//////////////////////////////////////////////////////////////////////////
///
///	Code Section
///
//////////////////////////////////////////////////////////////////////////

/** \brief Shells  task code.
 *
 * \param p void* : Pointer to a data structure storing all implemented commands
 * \param line char* : The command line
 * \return void
 *
 * \details This function parses the input string, getting the single tokens and launching the
 * corresponding actions.
 */
void ShellTask(void *p, char *line)
{
	int n;
	/// Initialize the shell command data structure
	const ShellCommand_t *scp=((ShellConfig_t *)p)->sc_command;
	char *lp, *cmd, *tokp;
	char *args[SHELL_MAX_ARGUMENTS + 1];

	/// Get all the tokens from the input string and stores them on lp and tokp pointer
	lp = Strtok(line, " \r\n", &tokp);

	#ifdef DEBUG
	avrPrintf("lp -> ");
	avrPrintf(lp);
	avrPrintf("\ntokp -> ");
	avrPrintf(tokp);
	avrPrintf(CR);
	#endif // DEBUG

	/// The command to execute is stored into lp
	cmd = lp;
	n = 0;

	///
	/// Until there are valid tokens fill the arguments array args with them
	///
	while ((lp = Strtok(NULL, " \r\n", &tokp)) != NULL)
	{
		/// If there are too many arguments display an error
		if (n > SHELL_MAX_ARGUMENTS)
		{
			avrPrintf ("Too many arguments\r\n");
			cmd = NULL;
			break;
		}
		/// else fill arguments array
		args[n++] = lp;
	}
	/// End the args array with a NULL argument
	args[n] = NULL;
	/// do we really need it ????
	if (n == 0)
	{
		#ifdef DEBUG
		avrPrintf("Forcing end of string\r\n");
		#endif // DEBUG
		int len = strlen(cmd);
		cmd[len] = '\0';
	}

	#ifdef DEBUG
	avrPrintf("Cmd -> ");
	avrPrintf(cmd);
	avrPrintf(CR);
	char numArgv[2];
	numArgv[0] = '0' + n;
	numArgv[1] = '\0';
	avrPrintf("n -> ");
	avrPrintf(numArgv);
	avrPrintf(CR);
	#endif // DEBUG
	///
	/// If there is a valid command to execute (not NULL),parse it and execute the corresponding action
	///
	if (cmd != NULL)
	{
		/// Exit the Shell
		if (strcasecmp(cmd, "exit") == 0)
		{
			/// Exit has no arguments
			if (n > 0)
			{
				Usage("exit not implemented being under RTOS");
			}
			/// Set the shell end flag
			/// bEnd = true;
			return;
		}
		/// Display the list of supported commands
		else if (strcasecmp(cmd, "help") == 0)
		{
			/// Help has no arguments
			avrPrintf("Entering help\r\n");
			if (n > 1)
			{
				Usage("help");
			}
			avrPrintf("Commands:\r\n");
			/// Display the Local Commands
			ListCommands(LocalCommands);
			/// Display the Shell Commands
			ListCommands(ShellCommands);
			avrPrintf(CR);
		}
		/// Try to Execute the other command, if it exits an error the command is not recognized
		else if (CmdExec(LocalCommands, cmd, n, args) && ((scp == NULL) || CmdExec(/*scp*/ ShellCommands, cmd, n, args)))
		{
			avrPrintf("Error: Command not recognized -> ");
			avrPrintf (cmd);
			avrPrintf (" ???\r\n");
		}
	}
}

/** \brief Command Execution.
 *
 * \param scp const ShellCommand_t* A pointer to the implemented commands.
 * \param name char* Name of the command to execute
 * \param argc int Number of arguments
 * \param argv[] char* A pointer to the Argument list
 * \return int Result of the command
 *
 * \details Execute a command and return the result.
 */
int CmdExec(const ShellCommand_t *scp, char *name, int argc, char *argv[])
{
	while (scp->sc_name != NULL)
	{
		if(strcmp(scp->sc_name, name) == 0)
		{
			scp->sc_function(argc, argv);
			return 0;
		}
		scp++;
	}
	return 1;
}

/** \brief System Time Command.
 *
 * \param argc int int Number of arguments
 * \param argv[] char* Pointer to the argument list
 * \return void
 *
 * \details Not implemented.
 */
void CmdSystime(int argc, char *argv[])
{
	(void) argv;
	//        If there are arguments display and error message
	if(argc > 0)
	{
		Usage("systime");
		return;
	}
	//        Else display a string stating that it is not implemented
	avrPrintf("Sys Time: Not implemented yet\r\nsystime OK\r\n");
}

/** \brief Info Command.
 *
 * \param argc int Number of arguments
 * \param argv[] char* Pointer to the argument list
 * \return void
 *
 * \details Display firmware information.
 */
void CmdInfo(int argc, char *argv[])
{
	(void)argv;
	/// If there are arguments plot an error message
	if(argc > 1)
	{
		Usage("info");
		return;
	}
	/// Else display Firmware and OS versions
	avrPrintf("Firmware: ");
	avrPrintf(FW_VERSION);
	avrPrintf("\r\nOS Version: ");
	avrPrintf(OS_VERSION);
	avrPrintf("\r\ninfo OK\r\n");
}

/** \brief List the Commands in the scp data structure.
 *
 * \param scp ShellCommand_t* Commands data structure
 * \return void
 *
 */
void ListCommands(ShellCommand_t *scp)
{
	/// Until the commands data structure has valid elements display the command name
	while (scp->sc_name != NULL)
	{
		avrPrintf((char *)scp->sc_name);
		avrPrintf("\r\n");
		scp++;
	}
}

/** \brief Command Usage Function.
 *
 * \param str char* Command use string
 * \return void
 *
 * \details Display information how to use the command.
 *
 */
void Usage(char *str)
{
	avrPrintf("Error: Usage-> ");
	avrPrintf(str);
	avrPrintf("\r\n");
}

/** \brief Substring token extraction (first one).
 *
 * \param str char* Input string
 * \param delim const char* Delimiters string
 * \param saveptr char** remaining tokens
 * \return char* first token found
 *
 * \details It implements the strtok function of string.h library (to avoid to import the full library)
 *
 */
char * Strtok(char *str, const char *delim, char **saveptr)
{
	char *token;
	if (str) *saveptr = str;
	token = *saveptr;

	if (!token) return NULL;

	token += strspn(token, delim);
	*saveptr = strpbrk(token, delim);

	if (*saveptr) *(*saveptr)++ = '\0';

	return *token ? token : NULL;
}

/** \brief Send the ACK
 *
 * \param argc int Number of arguments
 * \param argv[] char* A pointer to the Argument list
 * \return void
 *
 */
void SendACK(int argc, char *argv[])
{
	/// If there are arguments send an error message
	if (argc > 0)
	{
		Usage("get_ACK");
	}
	else
	{
		/// Send the ACK
		avrPrintf("ACK\r\nget_ACK OK\r\n");
	}
}

/** \brief Send a string on the serial port
 *
 * \param str const char* string to send
 * \return void
 *
 */
void avrPrintf(const char * str)
{
	nilSemWait(&SerialOutSem);
	Serial.print(str);
	nilSemSignal(&SerialOutSem);
}

/** \brief Send an integer on the serial port
 *
 * \param val const int integer to be sent
 * \return void
 *
 */
void avrPrintf(const int val)
{
	nilSemWait(&SerialOutSem);
	Serial.print(val);
	nilSemSignal(&SerialOutSem);
}

/** \brief Send an unsigned integer on the serial port
 *
 * \param val const uint16_t long to be sent
 * \return void
 *
 */
void avrPrintf(const uint16_t val)
{
	nilSemWait(&SerialOutSem);
	Serial.print(val);
	nilSemSignal(&SerialOutSem);
}

/** \brief Send a long on the serial port
 *
 * \param val const uint32_t long to be sent
 * \return void
 *
 */
void avrPrintf(const uint32_t val)
{
	nilSemWait(&SerialOutSem);
	Serial.print(val);
	nilSemSignal(&SerialOutSem);
}

/** \brief Send a double on the serial port
 *
 * \param val const double double to be sent
 * \return void
 *
 */
void avrPrintf(const double val)
{
	nilSemWait(&SerialOutSem);
	//Serial.print(val);
	printDouble(val, 2);
	//printFloat(val, 2);
	nilSemSignal(&SerialOutSem);
}

/** \brief Routine to display a double on the terminal
 *
 * \param val double Number to be displayed
 * \param precision byte Number of digit to display
 * \return void
 *
 */
void printDouble( double val, byte precision)
{
	/// prints val with number of decimal places determine by precision
	/// precision is a number from 0 to 6 indicating the desired decimial places
	/// example: printDouble( 3.1415, 2); // prints 3.14 (two decimal places)

	//Serial.print("Position= ");//Entering printDouble\n");

	//Serial.print("dummy dummy dummy\n");
	//Serial.print (int(val));  //prints the int part
	avrPrintf(int(val));
	if( precision > 0) {
		//Serial.print("."); // print the decimal point
		avrPrintf(".");
		unsigned long frac;
		unsigned long mult = 1;
		byte padding = precision -1;
		while(precision--)
		mult *=10;

		if(val >= 0)
		frac = (val - int(val)) * mult;
		else
		frac = (int(val)- val ) * mult;
		unsigned long frac1 = frac;
		while( frac1 /= 10 )
		padding--;
		while(  padding--)
		//Serial.print("0");
		avrPrintf("0");
		//Serial.print(frac,DEC) ;
		avrPrintf((int)frac);
		//Serial.print(frac);
	}
	//Serial.println("Exiting printDouble");
	//Serial.println(" ");
}

/** \brief Routine to display a float on the terminal
 *
 * \param value float Number to be displayed
 * \param places int Number of digit to print
 * \return void
 *
 */
void printFloat(float value, int places)
{
	int digit;
	float tens = 0.1;
	int tenscount = 0;
	int i;
	float tempfloat = value;
	float d = 0.5;

	if (value < 0) d *= -1.0;
	for (i=0; i<places; i++) d /= 10.0;

	tempfloat += d;
	if (value < 0 ) tempfloat *= -1.0;
	while ((tens * 10.0) <= tempfloat)
	{
		tens *= 10.0;
		tenscount++;
	}

	if (value < 0) Serial.print('-');
	if (tenscount == 0) Serial.print(0, DEC);

	for (i=0; i<tenscount; i++)
	{
		digit = (int)(tempfloat / tens);
		Serial.print(digit, DEC);
		tempfloat -= ((float)digit * tens);
		tens /= 10.0;
	}

	if (places <= 0) return;
	Serial.print('.');
	for (i=0; i<places; i++)
	{
		tempfloat *= 10.0;
		digit = (int)tempfloat;
		Serial.print(digit, DEC);
		tempfloat -= (float)digit;
	}
}

//////////////////////////////////////////////////////////////////////////
///
///	Thread Section
///
//////////////////////////////////////////////////////////////////////////


/** \brief Thread to implement a terminal shell
 *
 * \details This code is used to implement a terminal shell as thread.
 * To note that this code does not work, therefore it is not implemented.
 */
#ifdef USE_SHELL_THREAD
//
//  This part of the code is used only if USE_SHELL_THREAD switch is defined
//
///< Shell Thread working area definition
NIL_WORKING_AREA(waShellThread, STACKSIZE);

/** \brief Shell thread loop.
 *
 * \param extern void * ShellThread: thread function
 * \param char * arg[]: shell arguments
 * \return void *
 *
 * \details This thread waits a command from the serial port and executes it.
 * Not USED!!!
 *
 */
NIL_THREAD(ShellThread, arg)
//void vShellThread(void *p)
{
	/// Reserve space for the command line string buffer
	cmdString.reserve(CMD_STRING_LEN);
	avrPrintf("\r\nAVR Shell;\r\n");

	while(TRUE)
	{
		char ch;

		/// Execute the USB-CDC task
		CDC_Task();
		///
		/// If the '\n' is received the flag cmdReady is set and the input string is consumed by the Shell
		///
		if (cmdReady)
		{
			#ifdef DEBUG
			//Serial.println("Received -> " + cmdString);
			avrPrintf("Received -> ");
			//avrPrintf(cmdString.toCharArray());
			#endif // DEBUG

			char buffer[CMD_STRING_LEN];
			/// Create the char buffer pointer for the shell
			char *buf = (char *)&buffer;
			cmdString.toCharArray(buf, CMD_STRING_LEN);
			#ifdef DEBUG
			avrPrintf(buf);
			avrPrintf("Buf Len: ");
			//avrPrintf((const int)cmdString.length());
			avrPrintf(inBufCount);
			#endif // DEBUG
			/// Execute the Shell task with the data coming for the PC
			ShellTask((void *)ShellCommands, buf);
			/// Print the prompt
			avrPrintf(PROMPT);
			/// Reinitialize the input buffer and the flag
			cmdString = "";
			cmdReady = false;
			inBufCount = 0;
		}
	}
}

#endif // USE_SHELL_THREAD

/** \brief USB-CDC Task
 *
 * \return void
 *
 * \details USB CDC task code, read if a char is available on the serial port, and store it
 * into cmdString data structure, if CR is received unlock the cmdReady flag to execute
 * the command
 *
 */
void CDC_Task()
{
	char ch;
	///
	/// Until data are available from the Serial Port read the data and store it into the input buffer cmdString
	/// Process ends if '\n' is received or the MAX input string length is reached
	///
	while(Serial.available())
	{
		/// If '\n" is received or MAX string lenght is reached set the cmdReady flag
		if (++inBufCount == CMD_STRING_LEN) ch = '\n';
		else
		{
			ch = (char)Serial.read();
			cmdString += ch;
			//cmdString.concat(ch);
		}
		if (ch == '\n') cmdReady = true;
		#ifdef _DEBUG
		avrPrintf("\n-> ");
		avrPrintf(ch);
		avrPrintf("\nLen-> ");
		avrPrintf(inBufCount);
		#endif // DEBUG
	}
}
