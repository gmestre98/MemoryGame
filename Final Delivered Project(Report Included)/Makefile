#  Compiler
CC = gcc

#  Compiler Flags
CFLAGS = -g -pthread -lpthread -Wall

#  Sources
SOURCESS = board_library.c	UI_library.c	General.c	memserver.c
SOURCESC = board_library.c	UI_library.c	General.c	memclient.c
SOURCESB = board_library.c	UI_library.c	General.c	bot2.c

LIBS= -lSDL2 -lSDL2_ttf

#  Objects
OBJECTSS =	board_library.o	UI_library.o	General.o	memserver.o
OBJECTSC =	board_library.o	UI_library.o	General.o	memclient.o
OBJECTSB =	board_library.o	UI_library.o	General.o	bot2.o

server: $(OBJECTSS)
	gcc		$(CFLAGS) -o $@	$(OBJECTSS)	$(LIBS)

client: $(OBJECTSC)
	gcc		$(CFLAGS) -o $@	$(OBJECTSC)	$(LIBS)

bot2: $(OBJECTSB)
	gcc		$(CFLAGS) -o $@	$(OBJECTSB)	$(LIBS)

board_library.o:	board_library.h

UI_library.o:	UI_library.h

General.o:	General.h

memserver.o:	memserver.h




clean::
	rm -f *.o core a.out server *~

depend::
	makedepend $(SOURCESS)
	makedepend $(SOURCESC)
	makedepend $(SOURCESB)

