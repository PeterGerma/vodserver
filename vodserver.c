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
//#include <sys/sendfile.h>

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

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

void handle_request(int connfd, struct sockaddr_in *clientaddr, char* rootDirectory) {
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int n; /* message byte size */


  /* gethostbyaddr: determine who sent the message */
  hostp = gethostbyaddr((const char *)&clientaddr->sin_addr.s_addr, sizeof(clientaddr->sin_addr.s_addr), AF_INET);
  if (hostp == NULL)
    error("ERROR on gethostbyaddr");
  hostaddrp = inet_ntoa(clientaddr->sin_addr);
  if (hostaddrp == NULL)
    error("ERROR on inet_ntoa\n");
  printf("server established connection with %s (%s)\n",
    hostp->h_name, hostaddrp);

/* TRYING TO USE ROBUST IO
  char theType[BUFSIZE], theFile[BUFSIZE];
  rio_readinitb(&rio, connfd);
  rio_readlineb(&rio, buf, BUFSIZE);
  sscanf(buf, "%s %s", theType, theFile);
  printf("theType: %s  theFile: %s\n", theType, theFile);
  printf("rio: %s\n", rio); */



  /* read: read input string from the client */
  bzero(buf, BUFSIZE);
  n = read(connfd, buf, BUFSIZE);
  if (n < 0)
    error("ERROR reading from socket");

  printf("server received %d bytes: \n%s", n, buf);


  // Confirmed the next line grabs "Range: bytes=0-100"
  char* rangeRequest = strstr(buf, "Range: ");
  printf("found: %s\n", rangeRequest);
  bool isRange;
  int start = 0;
  int end = 0;
  printf("Got here\n");

  if(rangeRequest == NULL) {
    isRange = false;
  }
  else {
    isRange = true;
  }

  if(isRange) {
    printf("In isRange\n");
    fprintf(stderr, "%s\n", rangeRequest);
    for(int i=0; i<strlen(rangeRequest); i++) {
      fprintf(stderr, "%d\n", rangeRequest[i]);
    }
    int filled = sscanf(rangeRequest, "Range: bytes=%d-%d\r\n\r\n", &start, &end);
    //int filled = sscanf(rangeRequest, "%*[^0123456789]%lu%*[^0123456789]%lu", start, end);
    fprintf(stderr,"filled: %d\n", filled);
    fprintf(stderr,"Start: %d, End: %d\n", start, end);
  }


  char* request = strtok(buf, "\t\n");
  char* fileToGet = &request[4];
  fileToGet = strtok(fileToGet, " ");
  char* requestType = strtok(request, " ");
  printf("requestType: %s\nfileToGet: %s\n", requestType, fileToGet);





  //char* response;
  char response[BUFSIZE];

  if(strcmp(requestType, "GET \0")) {
    
    char filePath[BUFSIZE];
    
    char timeHeader[100];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);

    // switch(t->tm_wday) {
    //   case 1 :
    //     timeHeader = "Mon, ";
    //     break;
    //   case 2 :
    //     timeHeader = "Tue, ";
    //     break;
    //   case 3 :
    //     timeHeader = "Wed, ";
    //     break;
    //   case 4 :
    //     timeHeader = "Thu, ";
    //     break;
    //   case 5 :
    //     timeHeader = "Fri, ";
    //     break;
    //   case 6 : 
    //     timeHeader = "Sat, ";
    //     break;
    //   case 7 : 
    //     timeHeader = "Sun, ";
    //     break;
    // }
    
    //strcat(timeHeader, t->tm_mday);

    strftime(timeHeader, sizeof(timeHeader)-1, "Date: %a, %d %b %Y %H:%M:%S GMT", t); 
    printf("%s\n", timeHeader);

    // time(&t);
    // struct tm *gtm = gmtime(&t);
    // printf("%d, %d, %d, %d, %s\n", gtm->tm_hour, gtm->tm_mday, gtm->tm_year, gtm->tm_yday, gtm->tm_zone);

    printf("Root: %s\n", rootDirectory);  
    strcpy(filePath, rootDirectory);
    strcpy(&filePath[strlen(rootDirectory)], fileToGet);
    printf("PATH: %s\n", filePath);

    FILE *fptr = fopen(filePath, "r");
    int content = open(filePath, O_RDONLY);
    struct stat st;
    stat(filePath, &st);
    int size = (int)st.st_size;
    char fileSize[BUFSIZE];
    sprintf(fileSize, "Content-size: %d", size);
    printf("%s",fileSize);

    printf("%d\n",content);
    if(content == -1) {
      strcpy(response, "HTTP/1.1 404 Not Found\n Date: Sun, 2 Oct 2018 08:56:53 GMT\n Server: Apache/2.2.14 (Win32)\n Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\n ETag: \"10000000565a5-2c-3e94b66c2e680\"\n Accept-Ranges: bytes\n Content-Length: 44\n Connection: close\n Content-Type: text/html\n X-Pad: avoid browser bug\n\r\n<html><body><h1>404: Content Not Found</h1></body></html>\n");
      n = write(connfd, response, strlen(response));
    } 
    else {
      printf("Delivering Content!\n");
      //snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n%s", timeHeader);
      //response = "HTTP/1.1 200 OK\r\n\r\n";

      if(!isRange) {
        strcpy(response, "HTTP/1.1 200 OK\n");
        strcat(response, timeHeader);
        strcat(response, "\n");
        strcat(response, "Content-length: ");
        strcat(response, fileSize);
        strcat(response, "\n\r\n");
        printf("Repsonse Length: %lu\n", strlen(response));

        n = write(connfd, response, strlen(response));
        char contentBuf[BUFSIZE];
        while( (n = read(content, contentBuf, BUFSIZE)) > 0) {
          write(connfd, contentBuf, n);
        }
      }
      else {
        strcpy(response, "HTTP/1.1 206 Partial Content\n");
        strcat(response, timeHeader);
        strcat(response, "\n");
        strcat(response, "Content-length: ");
        strcat(response, fileSize);
        strcat(response, "\n\r\n");
        printf("Repsonse Length: %lu\n", strlen(response));
        n = write(connfd, response, strlen(response));
        char contentBuf[BUFSIZE];
        //Not built into C apparently
        //sendfile(connfd, content, &start, end - start);
        //Use fseek

        //size_t indexOffset = 256 * index;

        fseek(fptr, start, SEEK_SET);
        n = fread(contentBuf, sizeof(unsigned char), end-start, fptr);
        n = write(connfd, contentBuf, n);

        //n = pread(content, contentBuf, start-end, (off_t)start);
        //n = write(connfd, contentBuf, n);
      }
    }
  }
  // else {  
  //   strcpy(response, "HTTP/1.1 404 Not Found\n Date: Sun, 2 Oct 2018 08:56:53 GMT\n Server: Apache/2.2.14 (Win32)\n Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\n ETag: \"10000000565a5-2c-3e94b66c2e680\"\n Accept-Ranges: bytes\n Content-Length: 44\n Connection: close\n Content-Type: text/html\n X-Pad: avoid browser bug\n\r\n<html><body><h1>400: Bad Request!</h1></body></html>\n");
  //   n = write(connfd, response, strlen(response));
  // }

  printf("RESPONSE:%s\n", response);

  /* write: echo the input string back to the client */
  //n = write(connfd, buf, strlen(buf));

  //Going to be commented out
  //response = "HTTP/1.1 200 OK\n Date: Sun, 2 Oct 2018 08:56:53 GMT\n Server: Apache/2.2.14 (Win32)\n Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\n ETag: \"10000000565a5-2c-3e94b66c2e680\"\n Accept-Ranges: bytes\n Content-Length: 44\n Connection: close\n Content-Type: text/html\n X-Pad: avoid browser bug\n\r\n<html><body><h1>It works!</h1></body></html>\n";

  if (n < 0)
    error("ERROR writing to socket");
}

int main(int argc, char **argv) {
  
  int listenfd, connfd; /* listening socket */
  int portno; /* port to listen on */
  socklen_t clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  int optval; /* flag value for setsockopt */
  int pid; /*Forked process ID */

  char* rootDirectory = getenv("PWD");
  strcat(rootDirectory, "/content");


  /* check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  fprintf(stderr,"program running: %s\n",argv[0]);
  fprintf(stderr,"port #: %s\n",argv[1]);
  portno = atoi(argv[1]);



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
      handle_request(connfd, &clientaddr, rootDirectory);
      exit(0);
    }
    close(connfd);
  }
  return 0;
}
