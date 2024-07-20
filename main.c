#include "settings.h"
#include "my_web_server.h"
#include "socketutil.h"
#include <stdbool.h>
#include <signal.h>


#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT 6969


char image_log_mgs[] =
"[+] response :\n\n"
"+-----------------------------+\n"
"|                             |\n"
"|   +---------------------+   |\n"
"|   |                     |   |\n"
"|   |       IMAGE         |   |\n"
"|   |                     |   |\n"
"|   +---------------------+   |\n"
"|                             |\n"
"+-----------------------------+\n";

char big_line[] = "-------------------------------------------------------";





int* socketFD_ptr = NULL;
char **red_keys_ptr;
char **red_meanings_ptr;
int *red_count_ptr;

void usage(char* program_name) {
    char msg_string[] = {"Usage: %s [-a (--address) address] [-p (--port) port]\n"
    "By default 127.0.0.1:6969\n"
    "-h (--help) for display this message\n"
    "For the server to work, you need the files directory in the same directory as the program (You may use a symbolic link.).\n"
    "You also need to create a file /files/settings/redirects.settings in which to specify the\n"
    "redirection paths in the format (from:to)\n"
    "Only for GET request(yet)\n"
    };
    printf("%s", msg_string, program_name);
}


void handle_sigint(int sig) {
    // TODO: shutdown question [y/N]
    shutdown(*socketFD_ptr, SHUT_RDWR);
    printf("\nserver shutdown\n");
    free(socketFD_ptr);
    //
    for (int i = 0; i < red_count_ptr; i++) {
//        free(red_keys_ptr[i]);
//        free(red_meanings_ptr[i]);
    }

    exit(0);
}

int main(int argc, char** argv)
{
    char address_[16] = DEFAULT_ADDRESS;
    uint16_t port = DEFAULT_PORT;
    char client_ip[INET_ADDRSTRLEN];
    char files_directory[256];
    char current_directory[256];
    char settings_redirect[256];
    int opt = 1;
    char buffer[1024];


    //processing arguments from the console
    //
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0 || (strcmp(argv[i], "--help") == 0)) {
                usage(argv[0]);
                exit(0);
            }
            if (strcmp(argv[i], "-a" ) ==0 || strcmp(argv[i], "--address") == 0) {
                //set address
                if (i == (argc - 1)) {
                    usage(argv[0]);
                    exit(-1);
                }
                strcpy(address_, argv[i+1]);
                i++;
            }
            if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
                //set port
                if (i == (argc - 1)) {
                    usage(argv[0]);
                    exit(-1);
                }
                port = atoi(argv[i + 1]);
                i++;
            }
        }
    }
    //
    //

    //Setting paths to folders
    getcwd(current_directory, sizeof(current_directory));
    sprintf(files_directory, "%s/files/", current_directory);

    //Import redirection options from settings/redirects.setting
    sprintf(settings_redirect, "%ssettings/redirects.settings", files_directory);
    char **red_keys;
    char **red_meanings;
    red_keys_ptr = red_keys;
    red_meanings_ptr = red_meanings;
    int red_count, i;
    red_count_ptr = &red_count;
    if (read_from_file(settings_redirect, &red_keys, &red_meanings,&red_count) == 0) {
        fprintf(stderr, "[-] Can't open redirects.settings file\n");
    } else {
        printf("[+] redirects.settings successfully open\n[+] Redirects Table:\n");
        for (int key_i = 0; key_i < red_count; key_i++) {
            printf("%s ---> %s\n", red_keys[key_i], red_meanings[key_i]);
        }
    }

    //Set an event ctrl-c
    signal(SIGINT, handle_sigint);


    //Create socket file descriptor
    int socketFD = createTCPIpv4Socket();
    socketFD_ptr = &socketFD;

    //Set reuse address
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //Create address
    struct sockaddr_in* address= createIpv4Address(address_, port);

    //bind socket file descriptor file address
    int result = bind(socketFD, address, sizeof(struct sockaddr_in));

    if (result < 0) {
        printf("[-] error bind: %s\n", strerror(errno));
        return -1;
    }

    printf("[+] Server start at the %s:%d\n", address_, port);

    result = listen(socketFD, 0);

    if (result < 0) {
        printf("[-] error listen\n");
        return -1;
    }

    int addrlen = sizeof(address);



    bool redirect = false;
    char * bu = NULL;
    char * cont_type = NULL;
    char *file_path = NULL;
    char *file = NULL;
    int code_of_error =  0;
    struct sockaddr_in* pV4Addr;
    struct in_addr ipAddr;

    // ---------------------------------------------------------------------------MAIN CYCLE---------------------------------------------------------------------------
    while(true) {
        //Accept the connection
        int new_socket = accept(socketFD, address, &addrlen);

        pV4Addr = (struct sockaddr_in*) address;
        ipAddr = pV4Addr->sin_addr;

        bzero(client_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &ipAddr, client_ip, INET_ADDRSTRLEN);

        redirect = false;
        bu = NULL;
        cont_type = NULL;
        file_path = NULL;
        file = NULL;

        code_of_error = 0;

        if (new_socket < 0) {
            printf("[-] error accept\n");
            continue;
        }

        printf("\n\n[+++]Client connect with IP address: %s\n", client_ip);

        //Read data from user
        ssize_t valread = read(new_socket, buffer, 1024 - 1);
        buffer[valread] = 0;

        //Get path from http request
        char *path = get_path(buffer, valread);
        if (path == NULL) {
            continue;
        }
        //START REDIRECT block
        //
        //TODO: REMAKE THIS SHIT!!!
        int key_i;
        for (key_i = 0; key_i < red_count; key_i++) {
            if (strlen(red_keys[key_i]) == strlen(path)) {
                redirect = true;
                for (int i = 0; i < strlen(path); i++) {
                    if (red_keys[key_i][i] != path[i]) {
                        redirect = false;
                        break;
                    }
                }
            }

            if (redirect) {
                break;
            }
        }

        if (redirect) {
            //Response 301
            printf("[*] path = %s ---> %s\n", path, red_meanings[key_i]);
            bu = generate_redirect_301(red_meanings[key_i]);
            send(new_socket, bu, strlen(bu), 0);


        }
        //END REDIRECT block

        else {
            printf("[*] path = %s\n", path);

            //webfolder + path
            file_path = path_to_file(path, files_directory);
            printf("[*] file_path = %s\n", file_path);



            //Read file
            code_of_error = 0;
            file = readFile(file_path, &code_of_error);


            //START MAKERESPONSE block
            //

            //Define filetype
            cont_type = detectType(path, strlen(path));

            //Check for errors
            //Return 404 if file doesn't found
            if (file == NULL || file_path == NULL || strcmp(cont_type, "ERROR") == 0) {
                printf("[-] file :%s 404_Not_Found", path);
                file_path = path_to_file("/404.html", files_directory);
                code_of_error = 0;
                file = readFile(file_path, &code_of_error);
                if (file==NULL) {
                    file = "404ERROR_Not_Found\n";
                    printf("[-] Pls make 404.html ;)\n");
                }
                bu = generateResponse(file, "text/html", "404");
                printf("[*] response:\n%s\n%s\n%s\n", big_line, bu, big_line);
                //Send response to user
                send(new_socket, bu, strlen(bu), 0);
            }


            //If file exist make response 200
            else {
                printf("[*] content-type = %s\n", cont_type);

                //If filetype is image:
                if (strcmp(cont_type, "image/jpg") ==  0|| strcmp(cont_type, "image/jpeg") == 0 || strcmp(cont_type, "image/png") == 0 || strcmp(cont_type, "image/x-icon") == 0) {
                    send_image(new_socket, file_path);
                    printf("%s", image_log_mgs);

                }

                //If filetype is text:
                else {
                    bu = generateResponse(file, cont_type, "200");
                    printf("[+] response:\n\n%s\n%s\n%s\n", big_line, bu, big_line);
                    //Send response to user
                    send(new_socket, bu, strlen(bu), 0);

        }}}
        //END MAKERESPONSE block
        //
        //Close socket connection
        //Clear memory
        close(new_socket);
        free(file_path);
        free(file);
        free(cont_type);
        free(bu);
    }

    shutdown(socketFD, SHUT_RDWR);
    return 0;
}
