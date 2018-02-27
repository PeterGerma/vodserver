/*
 * echoserver.c - A simple connection-based echo server
 * usage: echoserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "request.h"
#include <sys/mman.h>

#define BUFSIZE 1024
#define MAX 10

#if 0
/*
 * Structs exported from netinet/in.h (for easy reference)
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr;
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

//Taken from https://stackoverflow.com/questions/5656530/how-to-use-shared-memory-with-linux-in-c
void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_ANONYMOUS | MAP_SHARED;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, 0, 0);
}

int main(int argc, char **argv) {
  

  int listenfd, connfd; /* listening socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  int optval; /* flag value for setsockopt */
  int pid; /*Forked process ID */
  int backPortNo; //backend port num

  //pid = fork();
  remoteContent* contentTable = (remoteContent *) create_shared_memory(sizeof(remoteContent)*BUFSIZE);
  int* contentIndex = create_shared_memory(sizeof(*contentIndex));
  *contentIndex = 0;
  char rootDirectory[BUFSIZE];
  strcpy(rootDirectory, getenv("PWD"));
  strcat(rootDirectory, "/content");


  /* check command line args */
  if (argc != 3) {
    fprintf(stderr, "usage: %s <port> <port2>\n", argv[0]);
    exit(1);
  }
  fprintf(stderr,"program running: %s\n",argv[0]);
  fprintf(stderr,"port #: %s\n",argv[1]);
  portno = atoi(argv[1]);
  backPortNo = atoi(argv[2]);



  /* socket: create a socket */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
   * us rerun the server immediately after we kill it;
   * otherwise we have to wait about 20 secs.
   * Eliminates "ERROR on binding: Address already in use" error.
   */
  optval = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
	     (const void *)&optval , sizeof(int));

  /* build the server's internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET; /* we are using the Internet */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
  serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

  /* bind: associate the listening socket with a port */
  if (bind(listenfd, (struct sockaddr *) &serveraddr,
	   sizeof(serveraddr)) < 0)
    error("ERROR on binding");

  /* listen: make it a listening socket ready to accept connection requests */
  if (listen(listenfd, 5) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");

  /* main loop: wait for a connection request, echo input line,
     then close connection. */
  clientlen = sizeof(clientaddr);

  while (1) {
    /* accept: wait for a connection request */
    connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    if((pid = fork()) == 0) { /*this is the child process*/
      if (connfd < 0)
        error("ERROR on accept");
    	close(listenfd);
      handleRequest(connfd, &clientaddr, rootDirectory, contentTable, contentIndex, backPortNo);
      exit(0);
    }
    close(connfd);
  }
  return 0;
}
