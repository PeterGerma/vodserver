CC = gcc
ARGS = -Wall -O2 -I .

all: vodserver udpserver udpclient

vodserver: vodserver.c request.c
	$(CC) $(ARGS) -o vodserver p2pbackend.c request.c vodserver.c
udpserver: udpserver.c
	$(CC) $(ARGS) -o udpserver udpserver.c
udpclient: udpclient.c
	$(CC) $(ARGS) -o udpclient udpclient.c


clean:
	rm -f *.o vodserver *~
