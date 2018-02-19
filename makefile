CC = gcc
ARGS = -Wall -O2 -I .

all: echoserver

echoserver: echoserver.c
	$(CC) $(ARGS) -o echoserver echoserver.c


clean:
	rm -f *.o echoserver *~
