#define FUSE_USE_VERSION 31


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
#include "log.h"

struct HTTPResponse {
	struct MemoryChunk *mem;
	CURLcode res;
};

char JSONTemplate[] = "endpoints.json";

struct HTTPResponse* http_get(const char *url) {
	CURL *curl_handle;
  	CURLcode res;
	struct HTTPResponse *response;
	struct MemoryChunk chunk;
//	printf("%s\n", url);
	chunk.memory = malloc(1);
	chunk.size = 0;
	curl_handle = curl_easy_init();
  	if(curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    		res = curl_easy_perform(curl_handle);

		response->mem = &chunk;
//		printf("%s\n", chunk.memory);
		response->res = res;

    		curl_easy_cleanup(curl_handle);
		// TODO: Remember to free chunk memory once done with it
    		//free(chunk.memory);
  	}
	return response;		
}

static int bb_getattr( const char* path, struct stat *st ) {
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time( NULL );
	st->st_mtime = time( NULL );

	if ( strstr( path, ".txt") == NULL ) {
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
	
	const char *end_ptr;
	const char *file_path;
	cJSON *endpoints;
	cJSON *res;
	//printf((char *)path);	
	// TODO: Make this a constant TOP OF FILE or whatever the C standard is
	file_path = "endpoints.json";
	endpoints = parseJSONFile(file_path, end_ptr);	
	if (!end_ptr) {
		cJSON_Delete(endpoints);
		return -1;
	}	
	char abs_path[256];
	memset(abs_path,0,sizeof(abs_path));
	if (getcwd(abs_path, sizeof(abs_path)) == NULL) {
	      perror("getcwd");
	      return -1;
      	}
	if (strcmp(path, "/") == 0 || strcmp(path, abs_path) == 0) {
		
		res = endpoints;		
	}
	else {
		// TODO: check required params is empty and all path params (besides ver) are not required
		char *sub_dir = (char *)path + 1;
		if (strstr(sub_dir, "/")) {
			printf("ERROR: Sub_directory not bottom-most child\n");
			cJSON_Delete(res);
			return -1;
		}
//		printf("%s\n", sub_dir);
		// Now we make the API call and populate res
		const cJSON *data = cJSON_GetObjectItemCaseSensitive(endpoints, sub_dir);
		struct HTTPResponse *response = http_get(data->child->valuestring);
		printf("http get complete\n");
		res = parseJSONChunk(response->mem);
		fprintf(stdout, "%s\n", res->valuestring);
		printf("JSON parse complete\n");
		printcJSON(res);
	}
	

	cJSON *current = NULL;
	cJSON_ArrayForEach(current, res) {
		filler(buffer, current->string, NULL, 0);
	}		
	cJSON_Delete(res);
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
/*	FILE *log_file;
	char path[256];
	memset(path,0,sizeof(path));
	if (getcwd(path, sizeof(path)) == NULL) {
		perror("getcwd");
		exit(EXIT_FAILURE);
	}
*/
	umask(0);
/*	log_file = fopen(strcat(path, "/log.txt"), "rw");
	if (!log_file) {
		printf("Failed to open log file\n");
		return -1;
	}
	log_add_fp(log_file, LOG_TRACE);
	log_info("Test test");*/
	return fuse_main( argc, argv, &operations, NULL );
}
