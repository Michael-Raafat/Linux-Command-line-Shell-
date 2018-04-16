#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "variables.h"
#include "file_processing.h"
#include "command_parser.h"
#include "commands.h"
#include "environment.h"
#define BUFFERSIZE 1600
FILE* commandFile;
FILE* logFile;
FILE* historyFile;
char* commandList[256];
bool varWithExport = false;
bool varAlone = false;
bool backgroundFlag = false;
bool usingFile = false;
bool exitCommandFlag = false;
int lengthOfCommandList;

bool runCdEchoCommands();
int setcommandList();
void start_shell(bool read_from_file);
void shell_loop(bool input_from_file);
void start(bool read_from_file,char* fileName);
void trimCommandPartEndline(char *commandPart);
void forkProcess();
int isFile(const char* name);
void getCommandCorrectPath(char **command, char** result);
void handleCommand(char **commandPointer);
bool runAdditionalCommands();

/*
handler for signal from child.
*/
void handler(int sig)
{
    waitpid(-1, NULL, WNOHANG);
    fprintf(logFile, "Child process is terminated\n");
    fflush(logFile);
}


int main(int argc, char *argv[])
{
    setup_environment();

    signal(SIGCHLD, handler);
    // any other early configuration should be here
    if( argc > 1 ){
        start(true, argv[1]);
        usingFile = true;
    }
    else{
        start(false, argv[1]);
    }
    return 0;
}
/*
    start the excution.
*/
void start(bool read_from_file,char* fileName)
{
	cd(""); // let shell starts from home
	open_history_file(&historyFile);
    open_log_file(&logFile);
	if(read_from_file){
		// file processing functions should be called from here
		if (fileName[0] == '/') {
            open_commands_batch_file(&commandFile, fileName);
            shell_loop(true);
		} else {
		    char temp[250];
		    getcwd(temp, sizeof(temp));
		    int count = 0;
		    if (temp != NULL) {
		        while(temp[count] != '\0') {
                    count++;
		        }
		        temp[count] = '/';
		        count++;
		    }
		    char currentDir[count + strlen(fileName) + 1];
		    strcpy(currentDir, temp);
		    while(strlen(fileName)) {
                currentDir[count] = fileName[0];
                fileName++;
                count++;
		    }
		    if (isFile(currentDir) == 1) {
                open_commands_batch_file(&commandFile, currentDir);
                shell_loop(true);
		    } else {
		        printf("Invalid path of file.\n");
		    }
		}

	}
	else{

		shell_loop(false);
	}
}

/*
    while loop till the user enters exit command.
    receive input from user or file and pass it as parameter to handle command.
*/
void shell_loop(bool input_from_file)
{
    char* commandPointer = malloc(513);
	bool from_file = input_from_file;
	int temp_i = 0;
	for (temp_i = 0; temp_i < 256; temp_i++) {
        commandList[temp_i] = malloc(513);
	}
	while(true) {
	    bool flagEOF = true;
	    char cwd[513];
		if(from_file){
                commandPointer = get_command_from_file(&commandFile);
                if (commandPointer == NULL) {
                    from_file = false;
                    usingFile = false;
                    flagEOF = false;
                    fseek(stdin,0,SEEK_END);
                    close_commands_batch_file(&commandFile);
                    free(commandPointer);
                    commandPointer = malloc(513);
                } else {
                    printf("Shell:%s>", getcwd(cwd, sizeof(cwd)));
                    fflush(stdout);
                    printf("%s", commandPointer);
                    fflush(stdout);
                    if (!isEmptyCommand(commandPointer)) {
                        fprintf(historyFile,"%s", commandPointer);
                        fflush(historyFile);
                    }
                }
		} else{
		    printf("Shell:%s>", getcwd(cwd, sizeof(cwd)));
		    fflush(stdout);
            commandPointer = fgets(commandPointer, BUFFERSIZE, stdin);
            if ( commandPointer != NULL && !isEmptyCommand(commandPointer)) {
                fprintf(historyFile,"%s", commandPointer);
                fflush(historyFile);
            }
		}
        if (flagEOF) {
           if ( commandPointer != NULL &&
                !isEmptyCommand(commandPointer) &&
                !isCommentCommand(commandPointer) &&
                strlen(commandPointer) <= 513 ) {
                trimCommandPartEndline(commandPointer);
                backgroundFlag = false;
                handleCommand(&commandPointer);
           }
           if (exitCommandFlag || commandPointer == NULL) {
               printf("\n");
               break;
           }
        }
	}

	close_history_file(&historyFile);
	close_log_file(&logFile);
}

/*
    handle commands and deal with the command parser file to parse the command
*/
void handleCommand(char **commandPointer) {
    char *pathOfCommand = malloc(strlen(*commandPointer));
    if (parse_command(commandPointer, &lengthOfCommandList)) {
        int null_index = setcommandList();
        if (!checkCd(commandList[0]) && !checkEcho(commandList[0])
            && !checkExport(commandList[0]) && !checkVariable(commandList[0])
            && !checkHistoryCommand(commandList[0]) && !checkExit(commandList[0],usingFile))
        {
            if (isFile(commandList[0]) != 1) {
                pathOfCommand[0] = '\0';
                getCommandCorrectPath(&commandList[0], &pathOfCommand);
                strcpy(commandList[0], pathOfCommand);
            }
        }
        backgroundFlag = isBackgroundCommand();
        forkProcess();
        commandList[null_index] = malloc(513);
    } else {
        printf("Syntax error or Invalid command.\n");
        fflush(stdout);
    }
    free(pathOfCommand);
}


/*
    the fork process for system.
*/
void forkProcess(){
    pid_t pid;
    int status;
    bool flag = true;
    if (!backgroundFlag) {
        flag = runCdEchoCommands();
        if (!flag) {
            flag = runAdditionalCommands();
        }
    }
    pid = fork();
    if (!pid){ /* child */
        if (backgroundFlag) {
           flag = runCdEchoCommands();
           if (!flag) {
            flag = runAdditionalCommands();
           }
        }
        if (execv(commandList[0], commandList) == -1 && !flag) {
            perror("Error ");
        }
        exit(1);
        return;
    } else { /* parent */
        if (!backgroundFlag) {
            waitpid(pid, &status, 0);
        }
        return;
    }
}

// function that trim the \n from the command.
void trimCommandPartEndline(char *commandPart) {
    commandPart = strsep(&commandPart, "\n");
}

//get the path of the command.
void getCommandCorrectPath(char **command, char** result)
{
    char *path = strdup(getenv("PATH"));
    char *original = path;
    char *temp = malloc(513);
    bool flag = false;
    while (path) {
        temp[0] = '\0';
        strcat(temp, strsep(&path,":"));
        strcat(strcat(temp,"/"), *command);
        if (isFile(temp) == 1) {
            flag = true;
            strcat(*result, temp);
            free(temp);
            free(original);
            break;
        }
    }
    if (!flag) {
        free(temp);
        free(original);
        strcat(*result, *command);
    }

}

// it returns 0 if it's a directory.
// it returns 1 if it's a file.
// it returns -1 if it's neither nor.
int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     return 0;
    }

    if(errno == ENOTDIR)
    {
     return 1;
    }
    return -1;
}

//  set the commandParts to be passed as parameters to execv().
int setcommandList() {
    int k = 0;
    while (k < lengthOfCommandList) {
        getCommandPartByIndex(&commandList[k] , k);
        k++;
    }
    free(commandList[k]);
    commandList[k] = NULL;
    return k;
}

//return true if it execute any of these commands
//other wise it returns false.
bool runAdditionalCommands() {
    if (checkHistoryCommand(commandList[0])) {
            char *temp = malloc(513);
            restart_history_file();
            int i = 1;
            get_history_file_line(&historyFile, &temp);
            while ( temp[0] != '\0') {
                printf("%d\t%s", i, temp);
                fflush(stdout);
                i++;
                get_history_file_line(&historyFile, &temp);
            }
            free(temp);
    } else if ( lengthOfCommandList > 0 && checkExit(commandList[0], usingFile)) {
            exitCommandFlag = true;
    } else if (checkExport(commandList[0]) && (lengthOfCommandList > 1 && checkVariable(commandList[1]))) {
            putVariable(commandList[1], 1);
    } else if (checkVariable(commandList[0])) {
            putVariable(commandList[0], 0);
    } else {
        return false;
    }
    return true;
}

//return true if it execute any of these commands
//other wise it returns false.
bool runCdEchoCommands() {
    if (lengthOfCommandList > 0 && checkEcho(commandList[0])) {
            int i = 1;
            char *message = malloc(513);
            message[0] = '\0';
            if (i >= lengthOfCommandList) {
                echo("\n");
            } else {
                while (i < lengthOfCommandList) {
                    strcat(message, commandList[i]);
                    i++;
                    if (i != lengthOfCommandList) {
                        strcat(message, " ");
                    }
                }
                echo(message);
            }
            free(message);
    } else if (lengthOfCommandList > 0 && checkCd(commandList[0])) {
            int i = 1;
            if (lengthOfCommandList > 2) {
                printf("cd: Error: too much arguments.");
            }
            if (i < lengthOfCommandList) {
                cd(commandList[1]);
            } else {
                cd("");
            }
    } else  {
            return false;
    }
    return true;
}
