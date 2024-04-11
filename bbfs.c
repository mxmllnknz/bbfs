#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS  64

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "util.h"
#include "cJSON.h"

struct HTTPResponse {
	struct MemoryChunk *mem;
	CURLcode res;
};

char JSONTemplate[] = "endpoints.json";

static int bb_getattr( const char* path, struct stat *st ) {
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time( NULL );
	st->st_mtime = time( NULL );

	if ( strstr( path, ".txt") != 0 ) {
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
		// TODO: Add root path JSON parse
		cJSON *res = parseJSONFile("endpoints.json");
		cJSON_Print(res);
		
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

struct HTTPResponse* http_get(const char *url) {
	CURL *curl_handle;
  	CURLcode res;
	
	struct MemoryChunk chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_handle = curl_easy_init();
  	if(curl_handle) {
		struct HTTPResponse response;

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    		res = curl_easy_perform(curl_handle);

		response.mem = &chunk;
		response.res = res;

    		curl_easy_cleanup(curl_handle);
		// TODO: Remember to free chunk memory once done with it
    		//free(chunk.memory);
  	}		
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
