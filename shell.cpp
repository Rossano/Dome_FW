// 
// 
// 

//#include "shell.h"

//void shell::init()
//{
//
//
//}


//shell SHELL;


#include <string.h>
#include <stdio.h>

//        My included file and headers
#include <stddef.h>
#include "encoder.h"
#include "dome.h"

//        Shell own inclusion header
#include "shell.h"

//
//	Shell Variable Section
//
String cmdString = "";							//        Line Command string buffer
boolean cmdReady = false;						//        Flag indicating that a command is ready
uint8_t inBufCount = 0;							//        Input buffer char counter
//        ???
static boolean bEnd = false;

//
//        Shell Local command definition data structure
//
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

SEMAPHORE_DECL(SerialOutSem, 1);

/// <summary>
/// Shells  task code.
/// </summary>
/// <param name="p">Pointer to a data structure storing all implemented commands</param>
/// <param name="line">The commanbd line.</param>
void ShellTask(void *p, char *line)
{
	int n;
	//        Initialize the shell command data structure
	const ShellCommand_t *scp=((ShellConfig_t *)p)->sc_command;
	char *lp, *cmd, *tokp;
	char *args[SHELL_MAX_ARGUMENTS + 1];

	//        Get all the tokens from the input string and stores them on lp and tokp pointer
	lp = Strtok(line, " \r\n", &tokp);
	
	#ifdef DEBUG
	avrPrintf("lp -> ");
	avrPrintf(lp);
	avrPrintf("\ntokp -> ");
	avrPrintf(tokp);
	avrPrintf(CR);
	#endif // DEBUG
	
	//        The command to execute is stored into lp
	cmd = lp;
	n = 0;
	
	//
	//        Until there are valid tokens fill the arguments array args with them
	//
	while ((lp = Strtok(NULL, " \r\n", &tokp)) != NULL)
	{
		//        If there are too many arguments display an error
		if (n > SHELL_MAX_ARGUMENTS)
		{
			avrPrintf ("Too many arguments\r\n");
			cmd = NULL;
			break;
		}
		// else fill arguments array
		args[n++] = lp;
	}
	//        End the args array with a NULL argument
	args[n] = NULL;
	// do we really need it ????
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
	
	//
	//        If there is a valid command to execute (not NULL),parse it and execute the corresponding action
	//
	if (cmd != NULL)
	{
		//        Exit the Shell
		if (strcasecmp(cmd, "exit") == 0)
		{
			//        Exit has no arguments
			if (n > 0)
			{
				Usage("exit not implemented being under RTOS");
			}
			// Set the shell end flag
			//bEnd = true;
			return;
		}
		//        Display the list of supported commands
		else if (strcasecmp(cmd, "help") == 0)
		{
			//        Help has no arguments
			avrPrintf("Entering help\r\n");
			if (n > 1)
			{
				Usage("help");
			}
			avrPrintf("Commands:\r\n");
			//        Display the Local Commands
			ListCommands(LocalCommands);
			//        Display the Shell Commands
			ListCommands(ShellCommands);
			avrPrintf(CR);
		}
		//        Try to Execute the other command, if it exits an error the command is not recognized
		else if (CmdExec(LocalCommands, cmd, n, args) && ((scp == NULL) || CmdExec(/*scp*/ ShellCommands, cmd, n, args)))
		{
			avrPrintf("Error: Command not recognized -> ");
			avrPrintf (cmd);
			avrPrintf (" ???\r\n");
		}
	}
}

#ifdef USE_SHELL_THREAD

NIL_WORKING_AREA(waShellThread, STACKSIZE);

/// <summary>
/// Shell thread loop.
/// this function read from stdin and calls the real thread code
/// </summary>
/// <param name="p">A pointer to the implemented commands.</param>
NIL_THREAD(ShellThread, arg)
//void vShellThread(void *p)
{
	//char line[SHELL_MAX_LINE_LENGTH];
	// chRegSetThreadName("shell");
	
	//        Reserve space for the command line string buffer
	cmdString.reserve(CMD_STRING_LEN);
	avrPrintf("\r\nAVR Shell;\r\n");
	
	while(TRUE)
	{
		char ch;
		
		//        Execute the USB-CDC task
		CDC_Task();
		//
		//        If the '\n' is received the flag cmdReady is set and the input string is consumed by the Shell
		//
		if (cmdReady)
		{
			#ifdef DEBUG
			//Serial.println("Received -> " + cmdString);
			avrPrintf("Received -> ");
			//avrPrintf(cmdString.toCharArray());
			#endif // DEBUG
			
			char buffer[CMD_STRING_LEN];
			//        Create the char buffer pointer for the shell
			char *buf = (char *)&buffer;
			cmdString.toCharArray(buf, CMD_STRING_LEN);
			#ifdef DEBUG
			avrPrintf(buf);
			avrPrintf("Buf Len: ");
			//avrPrintf((const int)cmdString.length());
			avrPrintf(inBufCount);
			#endif // DEBUG
			//        Execute the Shell task with the data coming for the PC
			ShellTask((void *)ShellCommands, buf);
			//        Print the prompt
			//Serial.print(PROMPT);
			avrPrintf(PROMPT);
			//        Reinitialize the input buffer and the flag
			cmdString = "";
			cmdReady = false;
			inBufCount = 0;
		}
	}
	//while (!bEnd)
	//{
		//// Display the prompt
		//avrPrintf(SHELL_PROMPT);
		//// Get the command line from stdin
		//gets(line);
		//// Calls the shell task
		//ShellTask((void *)ShellCommands, line);
	//}
	//return;
}

#endif // USE_SHELL_THREAD
//
//        USB-CDC Task
//
void CDC_Task()
{
	char ch;
	//
	//        Until data are available from the Serial Port read the data and store it into the input buffer cmdString
	//        Process ends if '\n' is received or the MAX input string length is reached
	//
	while(Serial.available())
	{
		//        If '\n" is received or MAX string lenght is reached set the cmdReady flag
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

/// <summary>
/// Command Execution.
/// Execute a command and return the result.
/// </summary>
/// <param name="scp">A pointer to the implemented commands.</param>
/// <param name="name">Name of the command to execute</param>
/// <param name="argv">A pointer to the Argument list.</param>
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

/// <summary>
/// System Time Command.
/// Not implemented.
/// </summary>
/// <param name="argc">Number of parameters.</param>
/// <param name="argv">A pointer to the Argument list.</param>
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
	avrPrintf("Sys Time: Not implemented yet\r\nOK\r\n");
}

/// <summary>
/// Info Command.
/// Display firmware information.
/// </summary>
/// <param name="argc">Number of parameters.</param>
/// <param name="argv">A pointer to the Argument list.</param>

void CmdInfo(int argc, char *argv[])
{
	(void)argv;
	//        If there are arguments plot an error message
	if(argc > 1)
	{
		Usage("info");
		return;
	}
	//        Else display Firmware and OS versions
	avrPrintf("Firmware: ");
	avrPrintf(FW_VERSION);
	avrPrintf("\r\nOS Version: ");
	avrPrintf(OS_VERSION);
	avrPrintf("\r\nOK\r\n");
}

/// <summary>
/// List the Commands in the scp data structure.
/// </summary>
/// <param name="scp">Command data structure.</param>
void ListCommands(ShellCommand_t *scp)
{
	//        Until the commands data structure has valid elements display the command name
	while (scp->sc_name != NULL)
	{
		avrPrintf((char *)scp->sc_name);
		avrPrintf("\r\n");
		scp++;
	}
}

/// <summary>
/// Command Usage Function.
/// Display information how to use the command.
/// </summary>
/// <param name="strc">Command usage string.</param>
void Usage(char *str)
{
	avrPrintf("Error: Usage-> ");
	avrPrintf(str);
	avrPrintf("\r\n");
}

/// <summary>
/// Substring token extraction.
/// </summary>
/// <param name="strc">Input string.</param>
/// <param name="delim">String containing a list of delimiters.</param>
/// <param name="saveptr">Vector of String containing all the substrings.</param>
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

/// <summary>
/// Send the ACK
/// </summary>
/// <param name="argc">Number of parameters.</param>
/// <param name="argv">A pointer to the Argument list.</param>
void SendACK(int argc, char *argv[])
{
	//        If there are arguments send an error message
	if (argc > 0)
	{
		Usage("get_ACK");
	}
	else
	{
		//        Send the ACK
		avrPrintf("ACK\r\nOK\r\n");
	}
}

void avrPrintf(const char * str)
{
	nilSemWait(&SerialOutSem);
	Serial.print(str);
	nilSemSignal(&SerialOutSem);
}

void avrPrintf(const int val)
{
	nilSemWait(&SerialOutSem);
	Serial.print(val);
	nilSemSignal(&SerialOutSem);
}
