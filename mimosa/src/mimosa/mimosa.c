#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mimosa.h"

//MarFS Includes

#include "tagging/tagging.h"

int mimosa_convert(char* path, struct stat* stat_struct, char* dest)
{
	if (S_ISDIR(stat_struct->st_mode)) printf("%s is a dir\n", path);
	if (S_ISREG(stat_struct->st_mode)) printf("%s is a file\n", path);

	FTAG ftag_struct = { 0 };
	ftag_struct.state = FTAG_INIT;

	return 0;
}

