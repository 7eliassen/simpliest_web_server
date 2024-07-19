#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define RETURN_SUCCESS 1
#define RETURN_FAILURE 0

bool read_from_file(const char *filename, char ***keys, char ***meanings, int *count);



#endif // SETTINGS_H_INCLUDED
