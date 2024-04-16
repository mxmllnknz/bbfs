#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

struct MemoryChunk {
  char *memory;
  size_t size;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

cJSON* parseJSONFile(const char *path, const char *end_return_ptr);

void printcJSON(cJSON *item);
#endif
