#include "file_processing.h"
#include <string.h>
#include <stdlib.h>
FILE* historyCounter;
int numberOfBytes;
int numberOfBytesOfCommand;

/*
	history file section
*/
void open_history_file(FILE** historyFile)
{
	char pathOfHome[1600];
	strcpy(pathOfHome, getenv("HOME"));
	strcat(pathOfHome,"/historyfile");
	*historyFile = fopen(pathOfHome, "a+");
	historyCounter = *historyFile;
}

/**
    get the history line commands sequentially.
**/
void get_history_file_line(FILE** historyFile, char ** pointer)
{
	char history[1600];
	fseek(historyCounter, numberOfBytes, SEEK_SET);
    if (fgets(history, sizeof(history), *historyFile) == NULL) {
        strcpy(*pointer, "\0");
        return;
    }
    numberOfBytes += strlen(history);
    strcpy(*pointer, history);
}

/**
    close the history file.
**/
void close_history_file(FILE** historyFile)
{
	fclose(*historyFile);
}

/**
    start up the history file.
**/
void restart_history_file() {
    numberOfBytes = 0;
}


/*
	log file section
*/
void open_log_file(FILE** logFile)
{
	char pathOfHome[1600];
	strcpy(pathOfHome, getenv("HOME"));
	strcat(pathOfHome,"/logfile");
	strcat(pathOfHome,"\0");
	int count = 0;
	while(strlen(pathOfHome)) {
        if (pathOfHome[count] == '\0') {
            break;
        }
        count++;
	}
	char path[count];
	strcpy(path, pathOfHome);
	*logFile = fopen(path, "a+");
}


void close_log_file(FILE** logFile )
{
	fclose(*logFile);
}
/*
	CommandsBatch file section
*/
void open_commands_batch_file( FILE** commandsFile, char *textfile)
{
	*commandsFile = fopen(textfile, "r");
}

const char* get_command_from_file(FILE** commandsFile)
{
    char command[1600];
	fseek(*commandsFile, numberOfBytesOfCommand, SEEK_SET);
    if (fgets(command, sizeof(command), *commandsFile) == NULL) {
        return NULL;
    }
    numberOfBytesOfCommand += strlen(command);
    return strdup(command);
}

void close_commands_batch_file(FILE** commandsFile)
{
	fclose(*commandsFile);
}
