CC=gcc
CFLAGS=-O -I../zlib-1.1.4

UNZ_OBJS = miniunz.o unzip.o ioapi.o iowin32.o ../zlib-1.1.4/libz.a
ZIP_OBJS = minizip.o zip.o   ioapi.o iowin32.o ../zlib-1.1.4/libz.a

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: miniunz minizip

miniunz:  $(UNZ_OBJS)
	$(CC) $(CFLAGS) -o $@ $(UNZ_OBJS)

minizip:  $(ZIP_OBJS)
	$(CC) $(CFLAGS) -o $@ $(ZIP_OBJS)

test:	miniunz minizip
	./minizip test readme.txt
	./miniunz -l test.zip
	mv readme.txt readme.old
	./miniunz test.zip

clean:
	/bin/rm -f *.o *~ minizip miniunz
