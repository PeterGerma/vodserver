CC = gcc
ARGS = -Wall -O2 -I .

all: vodserver

vodserver: vodserver.c
	$(CC) $(ARGS) -o vodserver request.c vodserver.c


clean:
	rm -f *.o vodserver *~
