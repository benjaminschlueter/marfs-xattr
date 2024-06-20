#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "mimosa.h"

//MarFS Includes

#include "tagging/tagging.h"
#include "config/config.h"

// MarFS Globals (initialize with mimosa_init())
marfs_config* config = NULL;
marfs_position* root_ns_pos = NULL;

int mimosa_init(marfs_config** config, pthread_mutex_t* mutex)
{
	// populate global config struct
	if (*config = config_init(getenv("MARFS_CONFIG_PATH"), mutex) == NULL)
	{ 
		err("Failed to Initialize MarFS context\n"); 
	}

	// establish position
	if (config_establishposition(root_ns_pos, config) == -1)
	{
		err("Failed to establish position of root namespace\n");
	}	


}

int mimosa_convert(char* path, struct stat* stat_struct, char* dest)
{
	if (S_ISDIR(stat_struct->st_mode)) printf("%s is a dir\n", path);
	if (S_ISREG(stat_struct->st_mode)) printf("%s is a file\n", path);

	FTAG ftag_struct = { 0 };
	ftag_struct.state = FTAG_INIT;


	return 0;
}

void err(char* err_msg)
{
	printf("%s", err_msg);
}

