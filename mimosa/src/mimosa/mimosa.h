#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config/config.h"

// External global variables
extern marfs_config* config;
extern marfs_position* root_ns_pos;

// External functions to be called in pwalk.c

int mimosa_init(marfs_config** config, pthread_mutex_t* mutex);
int mimosa_convert(char* name, struct stat* stat_struct, char* dest);

// Internal helper functions

void err(char* err_msg);
