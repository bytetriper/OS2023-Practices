# compile main.c in a proper way that can be used by fuse

CC=gcc
CFLAGS=-Wall -g -D_FILE_OFFSET_BITS=64
LDFLAGS=-lfuse -pthread `pkg-config fuse --cflags --libs`
FILES=main.c socket_client.c socket_server.c socket_support.c 
#mkdir /tmp/fuse before run
all: main

main: main.c
	$(CC) $(CFLAGS) -o main $(FILES) $(LDFLAGS)

clean:
	rm -f main

run:
	rm -r /tmp/fuse
	mkdir /tmp/fuse
	./main /tmp/fuse -f -d -o allow_other -o default_permissions
srun:
	rm -r /tmp/fuse_server
	mkdir /tmp/fuse_server
	./main /tmp/fuse_server -f -d -o allow_other -o default_permissions
crun:
	rm -r /tmp/fuse_client
	mkdir /tmp/fuse_client
	./main /tmp/fuse_client -f -d -o allow_other -o default_permissions
stop:
	fusermount -u /tmp/fuse/
sstop:
	fusermount -u /tmp/fuse_server/
cstop:
	fusermount -u /tmp/fuse_client/