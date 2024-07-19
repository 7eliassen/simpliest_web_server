#include "my_web_server.h"


char * get_path(char* buffer, unsigned int length) {
    char * path = NULL;
    if (buffer[0] == 'G' && buffer[1] == 'E' && buffer[2] == 'T') {
        int i;
        for (i = 0; buffer[i] != '\n' && i < length; i++);
        buffer[i] = 0;
        int j, n;
        for (n = 0; n < i && buffer[n] != ' '; n++);
        path = malloc(length);
        bzero(path, length);
        for (int x = 0; buffer[n + x + 1] != 0; x++) {
            if (buffer[n + x + 1] == ' ') {
                break;
            }
            path[x] = buffer[n + x + 1];
        }
    }
    return path;
}

char *readFile(const char *path, int *code_of_error) {
    FILE *file;
    char *buffer;
    long file_size;
    size_t result;

    code_of_error = 0;

    if (access(path, F_OK) != 0) {
        if (errno == ENOENT) {
            fprintf(stderr, "[-] File '%s' does not exist\n", path);
            code_of_error = ERROR404;
        } else if (errno == EACCES) {
            fprintf(stderr, "[-] File '%s' is not accessible\n", path);
            code_of_error = ERROR403;
        } else {
            fprintf(stderr, "[-] Error accessing file '%s': %s\n", path, strerror(errno));
            code_of_error = ERROR403;
        }

        return NULL;

    }

    // Open the file in binary mode
    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "[-] Could not open file '%s': %s\n", path, strerror(errno));
        code_of_error = ERROR500 ;
        return NULL;
    }

    // Get the file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "[-] Error seeking end of file '%s': %s\n", path, strerror(errno));
        fclose(file);
        code_of_error = ERROR500 ;
        return NULL;
    }
    file_size = ftell(file);
    if (file_size == -1) {
        fprintf(stderr, "[-] Error getting file size of '%s': %s\n", path, strerror(errno));
        fclose(file);
        code_of_error = ERROR500 ;
        return NULL;
    }
    rewind(file);

    // Allocate memory to contain the whole file
    buffer = (char *)malloc(file_size + 1); // Allocate extra space for null terminator
    bzero(buffer, file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "[-] Memory allocation error\n");
        fclose(file);
        code_of_error = ERROR500 ;
        return NULL;
    }

    // Read the file into the buffer
    result = fread(buffer, 1, file_size, file);
    if (result != file_size) {
        fprintf(stderr, "[-] Error reading file '%s'\n", path);
        free(buffer); // Free memory before returning NULL
        fclose(file);
        code_of_error = ERROR500 ;
        return NULL;
    }
    fclose(file);

    // Null-terminate the buffer
    buffer[file_size] = '\0';

    // Clean up

    return buffer;
}


char* path_to_file(char *path_, char webfiles[]) {
    char * path = malloc(1024);
    bzero(path, 1024);
    int i = 0;
    for (; webfiles[i] != 0; i++) {
        path[i] = webfiles[i];
    }
    int x = i;

    for (i = 0; path_[i] != 0; i++) {
        path[i + x] = path_[i];
    }

    int len = strlen(path);
    if (path[len - 1] == 47) {
        path[len - 1] = 0;
    }
    if (path[strlen(path) - 1] == '/') {
        path = NULL;
    }
    return path;
}

char *  detectType(char * filename, int length) {
    int i = length - 1;
    char fileFormat[30] = {0};
    char * content_type = malloc(100);
    bzero(content_type, 100);
    for (;filename[i] != '.'&&i>=0;i--);
    int x = 0;
    if (i==0) {
        strncat(content_type, "ERROR", 100);
        return content_type;
    }
    i++;
    int j = i;
    for(;i < length; i++) {
        fileFormat[x] = filename[i];
        x++;
    }

    if (strcmp(fileFormat, "html") == 0)
        strncat(content_type, "text/html", 100);
    else if (strcmp(fileFormat, "css") == 0)
        strncat(content_type, "text/css", 100);
    else if (strcmp(fileFormat, "js") == 0)
        strncat(content_type, "text/javascript", 100);
    else if (strcmp(fileFormat, "jpg") == 0 || strcmp(fileFormat, "jpeg") == 0)
        strncat(content_type, "image/jpeg", 100);
    else if (strcmp(fileFormat, "png") == 0)
        strncat(content_type, "image/png", 100);
    else if (strcmp(fileFormat, "ico") == 0)
        strncat(content_type, "image/x-icon", 100);
    else
        strncat(content_type, "ERROR", 100);
    return content_type;
}

void send_image(int socket, char* image_path) {
    int image_fd;
    struct stat st;
    char *image_buffer;
    char header[1024];

    // Open the image file
    image_fd = open(image_path, O_RDONLY);
    if (image_fd == -1) {
        error("Error opening image file");
    }

    // Get the size of the image
    if (fstat(image_fd, &st) == -1) {
        error("Error getting image file size");
    }

    // Allocate buffer to hold the image
    image_buffer = (char *)malloc(st.st_size);
    if (image_buffer == NULL) {
        error("Error allocating memory for image");
    }

    // Read the image into the buffer
    if (read(image_fd, image_buffer, st.st_size) == -1) {
        error("Error reading image file");
    }

    // Close the image file
    close(image_fd);

    // Create the HTTP response header
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: image/jpeg\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n"
             "\r\n", st.st_size);

    // Send the header
    if (write(socket, header, strlen(header)) == -1) {
        error("Error sending header");
    }

    // Send the image data
    if (write(socket, image_buffer, st.st_size) == -1) {
        error("Error sending image data");
    }

    // Free the image buffer
    free(image_buffer);
}


char* generateResponse(char * file, char* type, char* code) {
    unsigned response_size = 4096;
    unsigned int content_length = strlen(file);
    char content_len_char[20] = {};
    sprintf(content_len_char, "%d", content_length);
    char * response = malloc(response_size);
    bzero(response, response_size);
    strncat(response, "HTTP/1.1 ", response_size);
    strncat(response, code, response_size);
    strncat(response, "\nContent-Type: ", response_size);
    strncat(response, type, response_size);
    strncat(response, "\nContent-Length: ", response_size);
    strncat(response, content_len_char, response);
    strncat(response, "\n\n", response_size);
    strncat(response, file, response_size);
    return response;
}


char* generate_redirect_301(char * destination) {
    unsigned response_size = 256;
    char response_template[] =
    "HTTP/1.1 301 Moved Permanently\n"
    "Location: ";
    char * response = malloc(response_size);
    bzero(response, response_size);
    strncat(response,response_template, response_size);
    strncat(response, destination, response_size);
    return response;
}

