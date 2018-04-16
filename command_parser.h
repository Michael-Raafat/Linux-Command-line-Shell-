#ifndef COMMAND_PARSER_H   /* Include guard */
#define COMMAND_PARSER_H
#include <regex.h>
typedef enum{ false = 0 , true = 1 } bool ;
/*
	- This function should be responsible for importing all details of the command
	- Should specify the type of the command "comment, cd, echo, expression - X=5 -, else"
	- Should specify the arguments of the command
	- Should specify if the command is background or foreground
	- Should consider all parsing special cases, example: many spaces in  "ls     -a"

	- You're left free to decide how to return your imported details of this command

	- Best practice is to use helper function for each collection of logical instructions,
	  example: function for splitting the command by space into array of strings, ..etc
*/
bool parse_command( char** command, int* size);
// 1 for export variable assignment and 0 for variable assignment
void putVariable(char *command, int mode);
//return true if it's an export command.
// otherwise it returns false.
bool checkExport(char *command);
//return true if it's a variable assignment command
// otherwise it returns false.
bool checkVariable(char *command);
//return true if it's a background mode command.
// otherwise it returns false.
bool isBackgroundCommand();
//return true if it's a variable assignment command
// otherwise it returns false.
bool isEmptyCommand(char *command);
//return true if it's a comment command
// otherwise it returns false.
bool isComment(char *command);
//return true if it's an echo command
// otherwise it returns false.
bool checkEcho(char *commandPart);
//return true if it's a history command
// otherwise it returns false.
bool checkHistoryCommand(char *command);
//return true if it's a cd command
// otherwise it returns false.
bool checkCd(char *commandPart);
//return true if it's an exit command
// otherwise it returns false.
bool checkExit(char *commandPart, bool from_file);
//take a copy for command part.
void getCommandPartByIndex(char **commandPart, int index);
#endif // COMMAND_PARSER_H
