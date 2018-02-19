CC = gcc
ARGS = -Wall -O2 -I .

all: vodserver

echoserver: vodserver.c
	$(CC) $(ARGS) -o vodserver vodserver.c


clean:
	rm -f *.o vodserver *~
