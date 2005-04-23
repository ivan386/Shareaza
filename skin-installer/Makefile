CC = gcc
RM = rm -f
RC = windres

OBJ =	lib/unzip/ioapi.o \
		lib/unzip/unzip.o \
		extract.o \
		registry.o \
		skin.o \
		utils.o \
		window.o

LIB = -lgdi32 -lcomctl32 ../shareaza/zlib/zdll.lib
BIN = skin.exe

CFLAGS  = -Os -DZLIB_DLL -DNOCRYPT -DWIN32 -I../shareaza/zlib -Ilib/unzip
LFLAGS  = -s -mwindows
RCFLAGS = --input-format rc --output-format coff

all: $(OBJ) resource.res
	$(CC) $(CFLAGS) $(LFLAGS) -o $(BIN) $(OBJ) $(LIB) resource.res

resource.res : resource.rc resource.h
	$(RC) $(RCFLAGS) -o resource.res -i resource.rc

clean:
	$(RM) lib/unzip/*.o
	$(RM) *.aps
	$(RM) *.o
	$(RM) *.res

distclean: clean
	$(RM) $(BIN)
