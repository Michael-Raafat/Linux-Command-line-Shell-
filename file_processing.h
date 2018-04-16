#ifndef FILE_PROCESSING_H_   /* Include guard */
#define FILE_PROCESSING_H_
#include <stdio.h>
/*
	history file basic functions' prototypes
*/
void open_history_file(FILE** historyFile);
void get_history_file_line(FILE** historyFile,  char ** pointer);
void close_history_file(FILE** historyFile);
void restart_history_file();

/*
	log file basic functions' prototypes
*/
void open_log_file(FILE** historyFile);
void close_log_file(FILE** historyFile);

/*
	CommandsBatch file basic functions' prototypes
*/
void open_commands_batch_file(FILE** commandFile, char* textFile);
const char* get_command_from_file(FILE** commandFile);
void close_commands_batch_file(FILE** commandFile);


#endif // FILE_PROCESSING_H_
