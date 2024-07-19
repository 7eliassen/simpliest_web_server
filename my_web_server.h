#ifndef MY_WEB_SERVER_H_INCLUDED
#define MY_WEB_SERVER_H_INCLUDED

#define ERROR404 404
#define ERROR403 403
#define ERROR500 500

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>





typedef struct {
    char pathToFiles[1024];
} myWebSever;

char *readFile(const char *path, int *code_of_error);

char * get_path(char* buffer, unsigned int length);

char* path_to_file(char *path_, char webfiles[]);

char * detectType(char * filename, int length);

char* generateResponse(char * file, char* type, char* code);

void send_image(int socket, char* image_path);

char* generate_redirect_301(char * destination);

char* generate_Not_Found_404(char * path, char * files_directory);




#endif // MY_WEB_SERVER_H_INCLUDED
