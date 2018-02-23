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
#include "request.h"

#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT fprintf
#else
#define DEBUG_PRINT(format, args...)((void)0) //Nothing
#endif


void error(char *msg) {
  perror(msg);
  exit(1);
}

void handleRequest(int connfd, struct sockaddr_in *clientaddr, char* rootDirectory, remoteContent* contentDirectory, int* contentIndex) {
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
  DEBUG_PRINT(stderr, "server established connection with %s (%s)\n",
    hostp->h_name, hostaddrp);

  /* read: read input string from the client */
  bzero(buf, BUFSIZE);
  n = read(connfd, buf, BUFSIZE);
  if (n < 0)
    error("ERROR reading from socket");

  DEBUG_PRINT(stderr, "server received %d bytes: \n%s", n, buf);


  //Parsing the first line of the HTTP request
  char* request = strtok(buf, "\t\n");
  char* requestArg1 = &request[4];
  requestArg1 = strtok(requestArg1, " ");
  char* requestArg2 = strtok(NULL, "\t\n");
  char* requestType = strtok(request, " ");
  DEBUG_PRINT(stderr,"requestType: %s\nrequestArg1: %s\nrequestArg2: %s\n", requestType, requestArg1, requestArg2);

  //GETTING THE DIFFERENT HEADERS OF THE HTTP REQUEST
  // Confirmed the next line grabs "Range: bytes=0-100"
  char* rangeRequest = strstr(buf, "Range: ");
  DEBUG_PRINT(stderr,"Range Request: %s\n", rangeRequest);
  bool isRange;
  int start = 0;
  int end = 0;

  if(rangeRequest == NULL) {
    isRange = false;
  }
  else {
    isRange = true;
  }
  if(isRange) {
    DEBUG_PRINT(stderr, "%s\n", rangeRequest);
    /*for(int i=0; i<strlen(rangeRequest); i++) {
      DEBUG_PRINT(stderr, "%d\n", rangeRequest[i]);
    } */
    int filled = sscanf(rangeRequest, "Range: bytes=%d-%d\r\n\r\n", &start, &end);
    DEBUG_PRINT(stderr,"filled: %d\n", filled);
    DEBUG_PRINT(stderr,"Start: %d, End: %d\n", start, end);
  }

  char response[BUFSIZE];
  bzero(response, BUFSIZE);

  if(strncmp(requestArg1, "/peer/", 6) == 0) {
    requestArg1 = &requestArg1[6];
    DEBUG_PRINT(stderr, "requestArg1: %s\n" , requestArg1);
    if(strncmp(requestArg1, "add", 3) == 0) {
      DEBUG_PRINT(stderr, "In add\n");
      const char* path = strtok(&strstr(requestArg1, "path=")[5], "&");
      const char* remoteHost = strtok(NULL, "&");
      DEBUG_PRINT(stderr, "remoteHost: %s\n", remoteHost);
      int remotePortNo = (int) strtok(NULL, "&");
      DEBUG_PRINT(stderr, "remotePortNo: %d\n", remotePortNo);
      DEBUG_PRINT(stderr, "%s\n", path);
      int i;
      int found = 0;
      for(i=0; i<*contentIndex; i++) {
        if(contentDirectory[i].path != NULL) {
          if(strcmp(contentDirectory[i].path, path) == 0) {
            found = 1;
          }
        }
      }
      if(found == 0) {
        strcpy(contentDirectory[*contentIndex].path, path);
      }

    }
    else if(strncmp(requestArg1, "view", 4) == 0) {

    }
    else if (strncmp(requestArg1, "config", 6) == 0) {

    }
  }
  else if(strncmp(requestType, "GET", 3) == 0) {
    
    char filePath[BUFSIZE];
    
    //Dealing with timer header
    char timeHeader[100];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);

    strftime(timeHeader, sizeof(timeHeader)-1, "Date: %a, %d %b %Y %H:%M:%S GMT", t); 
    DEBUG_PRINT(stderr, "%s\n", timeHeader);

    DEBUG_PRINT(stderr, "Root: %s\n", rootDirectory);  
    strcpy(filePath, rootDirectory);
    strcpy(&filePath[strlen(rootDirectory)], requestArg1);
    DEBUG_PRINT(stderr, "PATH: %s\n", filePath);

    FILE *fptr = fopen(filePath, "r");
    int content = open(filePath, O_RDONLY);
    struct stat st;
    stat(filePath, &st);
    int size = (int)st.st_size;
    char fileSize[BUFSIZE];
    DEBUG_PRINT(stderr, "Content-size: %d\n", size);

    DEBUG_PRINT(stderr, "contentfd: %d\n",content);
    if(content == -1) {
      strcpy(response, "HTTP/1.1 404 Not Found\n Date: Sun, 2 Oct 2018 08:56:53 GMT\n Server: Apache/2.2.14 (Win32)\n Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\n ETag: \"10000000565a5-2c-3e94b66c2e680\"\n Accept-Ranges: bytes\n Content-Length: 44\n Connection: close\n Content-Type: text/html\n X-Pad: avoid browser bug\n\r\n<html><body><h1>404: Content Not Found</h1></body></html>\n");
      n = write(connfd, response, strlen(response));
    } 
    else {
      DEBUG_PRINT(stderr, "Delivering Content!\n");
      //DEBUG_PRINT(response, sizeof(response), "HTTP/1.1 200 OK\r\n%s", timeHeader);
      //response = "HTTP/1.1 200 OK\r\n\r\n";

      if(!isRange) {
        strcpy(response, "HTTP/1.1 200 OK\n");
        strcat(response, timeHeader);
        strcat(response, "\n");
        strcat(response, "Content-length: ");
        strcat(response, fileSize);
        strcat(response, "\n\r\n");
        DEBUG_PRINT(stderr, "Repsonse Length: %lu\n", strlen(response));

        n = write(connfd, response, strlen(response));
        char contentBuf[BUFSIZE];
        while( (n = read(content, contentBuf, BUFSIZE)) > 0) {
          write(connfd, contentBuf, n);
        }
      }
      else if(isRange) {
        strcpy(response, "HTTP/1.1 206 Partial Content\n");
        strcat(response, timeHeader);
        strcat(response, "\n");
        strcat(response, "Content-length: ");
        strcat(response, fileSize);
        strcat(response, "\n\r\n");
        DEBUG_PRINT(stderr, "Repsonse Length: %lu\n", strlen(response));
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
  DEBUG_PRINT(stderr, "RESPONSE:%s\n", response);

  //Example response
  //response = "HTTP/1.1 200 OK\n Date: Sun, 2 Oct 2018 08:56:53 GMT\n Server: Apache/2.2.14 (Win32)\n Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\n ETag: \"10000000565a5-2c-3e94b66c2e680\"\n Accept-Ranges: bytes\n Content-Length: 44\n Connection: close\n Content-Type: text/html\n X-Pad: avoid browser bug\n\r\n<html><body><h1>It works!</h1></body></html>\n";

  if (n < 0)
    error("ERROR writing to socket");
}