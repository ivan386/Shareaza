CC = gcc
RM = rm -f
RC = windres

OBJZLIB =   lib/zlib/adler32.o \
		lib/zlib/compress.o \
    		lib/zlib/crc32.o \
    		lib/zlib/deflate.o \
    		lib/zlib/gzio.o \
    		lib/zlib/inffast.o \
    		lib/zlib/infback.o \
    		lib/zlib/inflate.o \
    		lib/zlib/inftrees.o \
    		lib/zlib/trees.o \
    		lib/zlib/uncompr.o \
    		lib/zlib/zutil.o

OBJ =	lib/unzip/ioapi.o \
		lib/unzip/unzip.o \
		extract.o \
		registry.o \
		skin.o \
		utils.o \
		window.o

LIB = -lgdi32 -lcomctl32 ../zlib/libzdll.a
BIN = ../bin/skin.exe

CFLAGS  = -Os -DNOCRYPT -DWIN32 -I../zlib -Ilib/unzip
LFLAGS  = -s -mwindows
RCFLAGS = --input-format rc --output-format coff

all: $(OBJ) resource.res
	$(CC) $(CFLAGS) $(LFLAGS) -o $(BIN) $(OBJ) $(LIB) resource.res

resource.res : resource.rc resource.h
	$(RC) $(RCFLAGS) -o resource.res -i resource.rc

clean:
	$(RM) lib/unzip/*.o
	$(RM) ../zlib/*.o
	$(RM) *.aps
	$(RM) *.o
	$(RM) *.res

distclean: clean
	$(RM) $(BIN)
