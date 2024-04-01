#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

static int bb_getattr( const char* path, struct stat *st ) {
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time( NULL );
	st->st_mtime = time( NULL );

	if ( strcmp( path, "/") == 0 ) {
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
	}
	else {
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}

	return 0;
		
}

static int bb_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ) {
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	if (strcmp(path, "/") == 0) {
		filler(buffer, "test1", NULL, 0);
		filler(buffer, "test2", NULL, 0);
	}

	return 0;
}

static int bb_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *i) {
	char fileTest1[] = "Hello, World!";
	char fileTest2[] = "Lorem ipsum asldkfjasldkfjslkjnjknvjasdlkfjsl dlkjadslkf jaskjdj flkasjdl fkjasdlkfjaskjdhflkasdjfl;k";
	char *selectedText = NULL;

	if (strcmp(path, "/test1") == 0) {
		selectedText = fileTest1;
	} else if (strcmp(path, "/test2") == 0) {
		selectedText = fileTest2;
	}

	memcpy(buffer, selectedText + offset, size);

	return strlen(selectedText) - offset;	
}

static struct fuse_operations operations = {
    .getattr	= bb_getattr,
    .readdir	= bb_readdir,
    .read	= bb_read,
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &operations, NULL );
}
