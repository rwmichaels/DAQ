# Compiles a simple code to read a CODA file.

CC = g++
LD = g++
EVIO = /adaqfs/home/adaq/evio/Linux-x86_64/lib
CFLAGS = -Wno-write-strings -pthread
INCFLAGS = -I /adaqfs/home/adaq/evio/Linux-x86_64/include
LIBS =	${EVIO}/libevio.so

all : rwcoda rwcoda_split

rwcoda: rwcoda.o
	rm -f $@
	$(LD) $(CFLAGS) -o rwcoda  rwcoda.o $(LIBS) 

rwcoda.o:rwcoda.c 
	$(CC) -c -g $(CFLAGS) $(INCFLAGS) rwcoda.c

rwcoda_split: rwcoda_split.o
	rm -f $@
	$(LD) $(CFLAGS) -o rwcoda_split  rwcoda_split.o  $(LIBS) 

rwcoda_split.o:rwcoda_split.c 
	$(CC) -c -g $(CFLAGS) $(INCFLAGS) rwcoda_split.c

clean:
	rm rwcoda rwcoda_split *.o
