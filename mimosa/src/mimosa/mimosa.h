#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config/config.h"
#include "tagging/tagging.h"

// External global variables
extern marfs_config* config;
extern marfs_position* root_ns_pos;
extern marfs_position* dest_pos;

// External functions to be called in pwalk.c

void mimosa_main_init(marfs_config** config, pthread_mutex_t* mutex, char* dest_path);
void mimosa_thread_init();
int mimosa_convert(char* name, struct stat* stat_struct, char* dest);

// Internal helper functions

int mimosa_create_file(marfs_position* pos, char* ref_path, char* user_path, FTAG ftag_struct);
FTAG mimosa_generate_ftag();
marfs_position* dup_pos(marfs_position* src);
void err(char* err_msg);
