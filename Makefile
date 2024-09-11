.RECIPEPREFIX = >
default: build

build: clean bbfs.o util.o cJSON.o log.o
> gcc -Wall -o bbfs bbfs.o util.o cJSON.o log.o -lcurl -g `pkg-config fuse --cflags --libs` 

bbfs.o: bbfs.c util.h cJSON.h
> gcc -c bbfs.c -lcurl -g `pkg-config fuse --cflags --libs`

util.o: util.c cJSON.h
> gcc -g -c util.c

cJSON.o: cJSON.c
> gcc -g -c cJSON.c

log.o: log.c
> gcc -g -c log.c

clean:
> rm -rf bbfs
> rm -rf test
> mkdir test

u:
> fusermount3 -u test

wipe:
> rm -rf bbfs
> rm -rf *.o

test: build
> ./bbfs test 
