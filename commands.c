#include "commands.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
typedef enum{ false = 0 , true = 1 } bool ;
char cdPrevious[513];

void cd(  char* path )
{
	char *home = NULL;
    struct passwd *pass;
    char *path_temp = path;
    char home_path[250];
    char previous_temp[513];
    char cwd[513];
    bool userNameFlag = false;
    bool cd_previous_flag = false;
    if (path[0] == '-' && (!path[1] || path[1] == ' ')) {
        strcpy(path, cdPrevious);
        strcpy(cdPrevious, getcwd(cwd, sizeof (cwd)));
        printf("%s\n", path);
        fflush(stdout);
        cd_previous_flag = true;
    }
    if (path_temp[0] == '~') {
        path_temp++;
        pass = getpwnam(path_temp);
        if (pass != NULL) {
            strcpy(home_path, "/home/");
            int count = 6;
            while (strlen(path_temp) > 0) {
                home_path[count] = path_temp[0];
                count++;
                path_temp++;
            }
            home_path[count] = '\0';
            userNameFlag = true;
        }
    }
    if(path[0] == ' ' || strlen(path) == 0 ) {
        home = getenv( "HOME" );
        if ( home == NULL ) {
            printf("there is an error");
            return;
        }
    }
    char *temp;
    temp = malloc(strlen(path) + 1);
    temp[0] = '\0';
    if (userNameFlag) {
       strcat(temp, home_path);
    } else if (home == NULL) {
       strcat(temp , path);
    } else {
        strcat(temp, home);
    }
    strcpy(previous_temp, getcwd(cwd, sizeof (cwd)));
    if( chdir( temp ) != 0 ) {
        printf("cd: %s: No such file or directory.\n", path);
        fflush(stdout);
    } else {
        if (!cd_previous_flag) {
            strcpy(cdPrevious, previous_temp);
        }
    }
    free(temp);
}


void echo( const char* message )
{
    printf("%s\n", message);
    fflush(stdout);
}


