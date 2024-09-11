#include "util.h"
#include "cJSON.h"

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryChunk *mem = (struct MemoryChunk *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    printf("error: not enough memory\n");

    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

cJSON* parseJSONFile(const char* path, const char* end_return_ptr) {
	FILE *f = fopen(path, "r");
	char *buffer;
	size_t fileSize;

	if(!f) {
		printf("Not able to open file %s", path);
		return NULL;
	}
	
	// Get the size of the file for the buffer size
	// NOTE: for file sizes >4GB, this won't work
	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	rewind(f);

	buffer = malloc(fileSize);
	if(!buffer) {
		printf("Failed to malloc memory.");
		return NULL;
	}
	size_t read = fread(buffer, 1, fileSize, f);
	if (ferror(f)) {
		printf("Failed to read file. Bytes read: %ld", read);
		return NULL;
	}

	fclose(f);
	
	cJSON *jptr = cJSON_ParseWithOpts(buffer, &end_return_ptr, 0);
	
	if (!end_return_ptr) {
		fprintf(stderr, "Error ptr: %s\n", end_return_ptr);
		return NULL;
	}
	free(buffer);

	return jptr;

}

cJSON* parseJSONChunk(struct MemoryChunk *mem) {
	const char *end_return_ptr;	
	cJSON *monitor_json = cJSON_ParseWithLengthOpts((char*)mem->memory, mem->size, &end_return_ptr, 0);
	if (!end_return_ptr) {
		fprintf(stderr, "Error ptr: %s\n", end_return_ptr);
		cJSON_Delete(monitor_json);
		return NULL;		
	}
	return monitor_json;
	
}

void printcJSON(cJSON *item) {
	if (item) {
		const char *str = cJSON_Print(item);
		fprintf(stdout, "%s\n", str);
	}	
	fprintf(stdout, "end of print\n");
}
