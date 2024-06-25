#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h> // update err function 
#include <string.h>

#include "mimosa.h"

//MarFS Includes

#include "tagging/tagging.h"
#include "config/config.h"

#define STR_BUF_SIZE 256
#define MDAL_PREFIX MDAL_subspaces/full-access-subspace/

// MarFS Globals (initialize with mimosa_init())
marfs_config* config = NULL;
marfs_position* root_ns_pos = NULL;
marfs_position* dest_pos = NULL;

void mimosa_main_init(marfs_config** config, pthread_mutex_t* mutex, char* dest_arg)
{
	// populate global config struct
	*config = config_init(getenv("MARFS_CONFIG_PATH"), mutex);
	if (*config == NULL)
		err("Failed to Initialize MarFS context\n"); 
	
	// establish position based on config
	root_ns_pos = (marfs_position*) calloc(1, sizeof(marfs_position));
	if (config_establishposition(root_ns_pos, *config) == -1)
		err("Failed to establish position of root namespace\n");
	
	// set position of destination
	dest_pos = dup_pos(root_ns_pos);

	if (config_traverse(*config, dest_pos, &dest_arg, 1) < 0)
		err("Failed to traverse to destination position\n");

	if (config_fortifyposition(dest_pos) == -1)
		err("Failed to create MDAL_CTXT for dest_pos\n");
	
	mimosa_create_file(dest_pos, "create_file_test.ref7", "create_file_test.user7", mimosa_generate_ftag()); 
	
}

/*
void mimosa_thread_init()
{

}
*/

int mimosa_convert(marfs_position* pos, char* user_path, struct stat* stat_struct)
{
	char* ref_str_buf = (char *) calloc(1, STR_BUF_SIZE);
	sprintf(ref_str_buf, "MDAL_subspaces/mimosa-test-space/%s", user_path); 

	if (S_ISDIR(stat_struct->st_mode)) printf("%s is a dir\n", user_path);
	if (S_ISREG(stat_struct->st_mode))
	{
		// mimosa_create_file( pos, ref_str_buf, user_path, mimosa_generate_ftag() );
	}

	return 0;
}

// INTERNAL FUNCTIONS

// take out ref_path arg later because can use datastream_genrpath
int mimosa_create_file(marfs_position* pos, char* ref_path, char* user_path, FTAG ftag_struct)
{
	// If the file exists in the user tree, skip it. If not, try to execute the steps below. If any steps have already been
	// executed before a failure, re-execution will do nothing, and mapping will resume after the point of failure. 
	if (pos->ns->prepo->metascheme.mdal->access(pos->ctxt, user_path, F_OK, 0) == 0)
	{
		printf("file %s exists, skipping\n");
		return -1;
	}

	MDAL_FHANDLE fhandle = pos->ns->prepo->metascheme.mdal->openref(pos->ctxt, ref_path, O_WRONLY | O_CREAT, S_IRWXU);
        if (fhandle == NULL)
                printf("openref Failed to get file handle: %s\n", strerror(errno));
		
        char* xattr_str_buf = (char *) calloc(1, STR_BUF_SIZE);
        ftag_tostr(&ftag_struct, xattr_str_buf, STR_BUF_SIZE); // will not work properly until more fields defined
        
	if (pos->ns->prepo->metascheme.mdal->fsetxattr(fhandle, 1, ref_path, xattr_str_buf, sizeof(FTAG), XATTR_CREATE) == -1)
                printf("Failed to set xattr: %s\n", strerror(errno));

        if (pos->ns->prepo->metascheme.mdal->linkref(dest_pos->ctxt, 0, ref_path, user_path) == -1)
                printf("linkref: failed to link refernce and user file %s\n", strerror(errno));

	return 0;
}

/*
int mimosa_create_dir()
{

}
*/


FTAG mimosa_generate_ftag()
{
	FTAG ftag_struct = { 0 };
        ftag_struct.state = FTAG_INIT;
        ftag_struct.ctag = "mimosa-created";

	return ftag_struct;
}


void marfs_test_create()
{
	FTAG ftag_struct = { 0 };
        ftag_struct.state = FTAG_INIT;
	ftag_struct.ctag = "mimosa-created";
	
	int tgt_buf_size = 256;
	char* xattr_str_buf = (char *) calloc(1, tgt_buf_size);
	ftag_tostr(&ftag_struct, xattr_str_buf, tgt_buf_size); // will not work properly until more fields defined
	
	MDAL_FHANDLE fhandle = dest_pos->ns->prepo->metascheme.mdal->openref(dest_pos->ctxt, "teststrxattr2", O_WRONLY | O_CREAT, S_IRWXU);	
	if (fhandle == NULL) 
		printf("openref Failed to get file handle: %s\n", strerror(errno));

	if (dest_pos->ns->prepo->metascheme.mdal->fsetxattr(fhandle, 1, "test-xattr", xattr_str_buf, sizeof(FTAG), XATTR_CREATE) == -1)
		printf("Failed to set xattr: %s\n", strerror(errno));
	
	if (dest_pos->ns->prepo->metascheme.mdal->linkref(dest_pos->ctxt, 0, "teststrxattr2", "mimosa.strxattr2") == -1)
		printf("linkref: failed to link refernce and user file %s\n", strerror(errno));

	exit(0);
}

marfs_position* dup_pos(marfs_position* src)
{
	marfs_position* dest_pos = malloc(sizeof(marfs_position));
        memcpy(dest_pos, root_ns_pos, sizeof(marfs_position));
	return dest_pos;
}

void err(char* err_msg)
{
	printf("ERROR: %s", err_msg);
}

