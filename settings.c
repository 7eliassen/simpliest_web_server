#include "settings.h"

bool read_from_file(const char *filename, char ***keys, char ***meanings, int *count) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "[-] Failed to open %s file\n", filename);
        return RETURN_FAILURE;
    }

    // Allocate initial memory for the arrays
    int capacity = 64;
    *keys = malloc(capacity * sizeof(char *));
    *meanings = malloc(capacity * sizeof(char *));
    *count = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Check if we need to resize the arrays
        if (*count >= capacity) {
            capacity *= 2;
            *keys = realloc(*keys, capacity * sizeof(char *));
            *meanings = realloc(*meanings, capacity * sizeof(char *));
        }

        // Parse the key and meaning
        char *key = strtok(line, ":");
        char *meaning = strtok(NULL, "\n");

        if (key && meaning) {
            (*keys)[*count] = strdup(key);
            (*meanings)[*count] = strdup(meaning);
            (*count)++;
        }
    }

    fclose(file);
    return RETURN_SUCCESS;
}
