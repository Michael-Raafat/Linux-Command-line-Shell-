#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include "command_parser.h"
#include "variables.h"
bool correctCommand = true;
bool backgroundCommand = false;
char* commandParts[256];

void getDollarModifiedCommand(char *command, char** result);
void getQuotesModifiedCommand(char **command);
int checkAndPresence(char *command);
void getArgWithoutAnd(char **commandPart);


//take a copy for command part.
void getCommandPartByIndex(char **commandPart, int index) {
    strcpy(*commandPart, commandParts[index]);
}


//free list of commands that were allocated.
void freeCommandList(int length) {
    int i = 0;
    while (i <= length) {
        if (commandParts[i] != 0) {
            free(commandParts[i]);
        }
        i++;
    }
}

//return true if it's a comment command
// otherwise it returns false.
bool isCommentCommand(char* command) {
    while (strlen(command) > 0 && command[0] == ' ') {
        command++;
    }
    if (command[0] == '#') {
        return true;
    }
    return false;
}

//parse the command given as parameter
bool parse_command( char** command, int* size)
{
    correctCommand = true;
    backgroundCommand = false;
    char* ptr = malloc(513);
    ptr[0] = '\0';
    getDollarModifiedCommand(*command, &ptr);
    char* cursor = ptr;
    int limit = 512;
    while(ptr[0] == ' ' && limit >= 0) {
        ptr++;
        cursor++;
        limit--;
    }
    int i = 0;
    bool singleQuoteFlag = false;
    bool doubleQuoteFlag = false;
    while (cursor && strlen(cursor) > 0) {
        if (!singleQuoteFlag && cursor[0] == '\'') {
            singleQuoteFlag = true;
        } else if (!doubleQuoteFlag && cursor[0] == '"') {
            doubleQuoteFlag = true;
        } else if (singleQuoteFlag && cursor[0] == '\'') {
            singleQuoteFlag = false;
        } else if (doubleQuoteFlag && cursor[0] == '"') {
            doubleQuoteFlag = false;
        }
        if (!singleQuoteFlag && !doubleQuoteFlag && (cursor[0] == ' ' || cursor[0] == '\t') ) {
            int length = strlen(ptr) - strlen(cursor);
            char token[length];
            strcpy(token,strsep(&ptr, " "));
            commandParts[i] = malloc( length + 1);
            strcpy(commandParts[i], token);
            i++;
            while (cursor[1] == ' ' || cursor[1] == '\t' ) {
                ptr++;
                cursor++;
            }
        }
        cursor++;
    }
    if (ptr && strlen(ptr) > 0) {
        int length = strlen(ptr);
        commandParts[i] = malloc(length + 1);
        strcpy(commandParts[i], ptr);
        i++;
    }
    *size = i;
    int j = 0;
    while (j < i && !checkVariable(commandParts[j])) {
       getQuotesModifiedCommand(&commandParts[j]);
        j++;

    }

    if (checkAndPresence(commandParts[i - 1]) == 0) {
        getArgWithoutAnd(&commandParts[i - 1]);
        if (strlen(commandParts[i - 1]) == 0) {
                *size = i - 1;
        }
        backgroundCommand = true;
    } else if (checkAndPresence(commandParts[i - 1]) == 0 && (i - 1) == 0) {
        correctCommand = false;
    }
    return correctCommand;
}

//return true if it's a background mode command.
// otherwise it returns false.
bool isBackgroundCommand() {
    return backgroundCommand;
}

// evaluate the $ special symbol.
void getDollarModifiedCommand(char *command, char **result) {
    char *temp = command;
    char *original = temp;
    char *variable;
    bool variableFlag = false;
    bool doubleQuoteFlag = false;
    while (command && strlen(command) && temp && strlen(temp)) {
        if (!doubleQuoteFlag && temp[0] == '"') {
            doubleQuoteFlag = true;
        } else if(doubleQuoteFlag && temp[0] == '"') {
            doubleQuoteFlag = false;
        }
        if (!doubleQuoteFlag && temp[0] == '~' && !isalpha(temp[1])) {
            strcat(*result, strsep(&command, "~"));
            strcat(*result, getenv("HOME"));

        } else if (temp[0] == '\'') {
            strcat(*result, strsep(&command, "'"));
            temp++;
            strcat(*result, "'");
            while(strlen(temp) > 0 && temp[0] != '\'') {
                temp++;
            }
            if (temp[0] == '\'') {
                strcat(*result, strsep(&command, "'"));
                strcat(*result, "'");
                temp++;
            } else {
                correctCommand = false;
                printf("Unclosed quoted field\n");
                fflush(stdout);
            }
        } else if (temp[0] == '$') {
            temp++;
            strcat(*result, strsep(&command, "$"));
            if (temp[0] == ' ' || temp[0] == '\n') {
                    if (temp[0] == ' ') {
                        strcat(*result, "$ ");
                    } else {
                        strcat(*result, "$\n");
                    }
                    temp++;
            } else if (isdigit((unsigned char)temp[0])) {
                    temp++;
                    command++;
            } else {
                    char *pointerCounter = command;
                    int counter = 0;
                    char temp_array[strlen(command) + 1];
                    while(pointerCounter && (isalnum(pointerCounter[0]) || pointerCounter[0] == '_') ) {
                            temp_array[counter] = pointerCounter[0];
                            pointerCounter++;
                            counter++;
                    }
                    temp_array[counter] = '\0';
                    variable = lookup_variable(temp_array);
                    variableFlag = true;
                    if ( variable != NULL) {
                        strcat(*result, variable);
                    }
                    command += counter;
                    temp += counter;
            }
        } else {
            temp++;
        }
    }
    strcat(*result, command);
}

// evaluate the presence of single and double quotes in the command.
void getQuotesModifiedCommand(char **command) {
    char temp[513];
    char *pointer = *command;
    int i = 0;
    while (pointer && strlen(pointer) > 0) {
        if (pointer[0] == '\'') {
            pointer++;
            while(pointer && strlen(pointer) > 0 && pointer[0] != '\'') {
                temp[i] = pointer[0];
                i++;
                pointer++;
            }
            if (pointer && strlen(pointer) > 0 && pointer[0] == '\'') {
                pointer++;
            } else {
                // handle error;
                correctCommand = false;
                printf("Can't Handle multiple line of commands.\n");
                fflush(stdout);
            }
        } else if (pointer[0] == '"') {
            pointer++;
            while(pointer && strlen(pointer) > 0 && pointer[0] != '"') {
                temp[i] = pointer[0];
                i++;
                pointer++;
            }
            if (pointer && strlen(pointer) > 0 && pointer[0] == '"') {
                pointer++;
            } else {
                // handle error;
                printf("Can't Handle multiple line of commands.\n");
                fflush(stdout);
                correctCommand = false;
            }
        } else {
            temp[i] = pointer[0];
            i++;
            pointer++;
        }
    }
    temp[i] = '\0';
    strcpy(*command, temp);
}

//return true if it's a variable assignment command
// otherwise it returns false.
bool checkVariable(char *command) {
    int length = strlen(command);
    char temp[length];
    strcpy(temp, command);
    int i = 0;
    while (temp[i] == ' ') {
        i++;
    }
    if (isalpha(temp[i]) || temp[i]== '_') {
        while (i < length) {
            if ((i + 1 != length) && temp[i] == '=' && temp[i + 1] != ' ') {
                return true;
            }
            if ((!isalnum(temp[i]) && !(temp[i] == '_'))) {
                return false;
            }
            i++;
        }
    } else {
        return false;
    }
    return false;
}

//return true if it's an export command.
// otherwise it returns false.
bool checkExport(char *command) {
    return (command[0] == 'e' &&
        command[1] == 'x' &&
        command[2] == 'p' &&
        command[3] == 'o' &&
        command[4] == 'r' &&
        command[5] == 't' &&
        ( !command[6] || command[6] == '\n') ) ;

}

//return true if it's an echo command
// otherwise it returns false.
bool checkEcho(char *commandPart) {
   if (commandPart[0] == 'e' &&
       commandPart[1] == 'c' &&
       commandPart[2] == 'h' &&
       commandPart[3] == 'o' &&
       (!commandPart[4] || commandPart[4] == '\n') ) {
            return true;
   }
   return false;
}

//return true if it's a history command
// otherwise it returns false.
bool checkHistoryCommand(char *command) {
    return (command[0] == 'h' &&
        command[1] == 'i' &&
        command[2] == 's' &&
        command[3] == 't' &&
        command[4] == 'o' &&
        command[5] == 'r' &&
        command[6] == 'y' &&
        (!command[7] || command[7] == '\n'));

}

//return true if it's a cd command
// otherwise it returns false.
bool checkCd(char *commandPart) {
    if (commandPart[0] == 'c' &&
       commandPart[1] == 'd' &&
       (!commandPart[2] || commandPart[2] == '\n') ) {
            return true;
    }
   return false;
}


//return true if it's an exit command
// otherwise it returns false.
bool checkExit(char *commandPart, bool from_file) {
    if (!from_file &&
        commandPart[0] == 'C' &&
       commandPart[1] == 't' &&
       commandPart[2] == 'r' &&
         commandPart[3] == 'l' &&
         commandPart[4] == '-' &&
         commandPart[5] == 'D' &&
         ( !commandPart[6] || commandPart[6] == '\n') ) {
            return true;
    } else if ( commandPart[0] == 'e' &&
            commandPart[1] == 'x' &&
            commandPart[2] == 'i' &&
            commandPart[3] == 't' &&
             ( !commandPart[4] || commandPart[4] == '\n') ) {
                return true;
             }
   return false;
}


// assign a value to a variable.
void putVariable(char *command, int mode) {
    while (command[0] == ' ') {
        command++;
    }
    char *value = malloc(strlen(command) + 1);
    value[0] = '\0';
    if (isalpha(command[0]) || command[0] == '_') {
        char *var = strdup(strsep(&command, "="));
        if (command[0] == '\'') {
                command++;
                char*buffer = command;
                bool flag = false;
                while (buffer && strlen(buffer) > 0) {
                    int count = 0;
                    if (buffer[0] == '\'') {
                        flag = true;
                    }
                    buffer++;
                }
                if (flag) {
                    strcat(value, strsep(&command, "'"));
                    if (mode == 1) {
                        if (setenv(var, value, 1) == -1) {
                            putenv(strcat(strcat(var, "="),value));
                        }
                    } else {
                        set_variable(var, value);
                    }
                } else {
                    printf("Not required to handle multiple line.\n");
                }
        } else if (command[0] == '"') {
            command++;
            char *buffer = command;
            bool flag = false;
            while (buffer && strlen(buffer) > 0) {
                    int count = 0;
                    if (buffer[0] == '"') {
                        flag = true;
                    }
                    buffer++;
                }
            if (flag) {
                strcat(value, strsep(&command, "\""));
                if (mode == 1) {
                    if (setenv(var, value, 1) == -1) {
                        putenv(strcat(strcat(var, "="),value));
                    }
                } else {
                    set_variable(var, value);
                }
            } else {
                printf("Not required to handle multiple line.\n");
            }
        } else {
            strcat(value, strsep(&command, " "));
            if (mode == 1) {
                if (setenv(var, value, 1) == -1) {
                    putenv(strcat(strcat(var, "="),value));
                }
            } else {
                set_variable(var, value);
            }
        }
      free(var);
    } else {
        printf("No command found.\n");
    }
    free(value);
}


//return true if it contains & in the end.
// otherwise it returns false.
int checkAndPresence(char *command) {
    int length = strlen(command);
    char temp[length];
    strcpy(temp, command);
    int i = 0;
    if (length == 1 && temp[0] == '&' ) {
        return 0;
    }
    while (i < length - 1) {
        i++;
    }
    if (temp[i] == '&') {
        return 0;
    }
    return 1;
}

//removes & from command.
void getArgWithoutAnd(char **commandPart) {
    char temp[513];
    char *pointer = *commandPart;
    int i = 0;
    if (strlen(*commandPart) == 1 && *commandPart[0] == '&') {
        temp[0] = '\0';
    }
    else {
        while (strlen(pointer) > 1) {
            temp[i] = pointer[0];
            i++;
            pointer++;
        }
        temp[i] ='\0';
    }
    strcpy(*commandPart, temp);

}

//return true if it's an empty command
// otherwise it returns false.
bool isEmptyCommand(char *command) {
    int length = strlen(command);
    char temp[length];
    strcpy(temp, command);
    int i = 0;
    while(i < length && (temp[i] == ' ' || temp[i] == '\t')) {
        i++;
    }
    if (temp[i] == '\n') {
        return true;
    }
    return false;
}
