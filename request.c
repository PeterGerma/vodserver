/*
 * request.c - Handles http requests
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


#define BUFSIZE 1024

void error(char *msg) {
  perror(msg);
  exit(1);
}

void handleRequest(int connfd, struct sockaddr_in *clientaddr, char* rootDirectory) {
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
    fprintf(stderr,"filled: %d\n", filled);
    fprintf(stderr,"Start: %d, End: %d\n", start, end);
  }


  char* request = strtok(buf, "\t\n");
  char* fileToGet = &request[4];
  fileToGet = strtok(fileToGet, " ");
  char* requestType = strtok(request, " ");
  printf("requestType: %s\nfileToGet: %s\n", requestType, fileToGet);



  char response[BUFSIZE];
  if(strncmp(fileToGet, "/peer/", 6) == 0) {
    backendRequest(fileToGet);
  }
  else if(strcmp(requestType, "GET \0")) {
    
    char filePath[BUFSIZE];
    
    //Dealing with timer header
    char timeHeader[100];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);

    strftime(timeHeader, sizeof(timeHeader)-1, "Date: %a, %d %b %Y %H:%M:%S GMT", t); 
    printf("%s\n", timeHeader);

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


        fseek(fptr, start, SEEK_SET);
        n = fread(contentBuf, sizeof(unsigned char), end-start, fptr);
        n = write(connfd, contentBuf, n);

        //n = pread(content, contentBuf, start-end, (off_t)start);
        //n = write(connfd, contentBuf, n);
      }
    }
  }
  printf("RESPONSE:%s\n", response);

  //Example response
  //response = "HTTP/1.1 200 OK\n Date: Sun, 2 Oct 2018 08:56:53 GMT\n Server: Apache/2.2.14 (Win32)\n Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\n ETag: \"10000000565a5-2c-3e94b66c2e680\"\n Accept-Ranges: bytes\n Content-Length: 44\n Connection: close\n Content-Type: text/html\n X-Pad: avoid browser bug\n\r\n<html><body><h1>It works!</h1></body></html>\n";

  if (n < 0)
    error("ERROR writing to socket");
}