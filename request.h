/**
 *  @file   request.h
 * 
 *  @brief  Handling parsing and writing of HTTP requests
 * 
 *  @date February 22 2018
 *  @author Peter Germa <pgerma@andrew.cmu.edu>
 */

#ifndef _REQUEST_H_
#define _REQUEST_H_

typedef struct remoteContent {
  char* path;
  char* remoteHost;
  int remotePortNo;
  int rate;
} remoteContent;

/** @brief Calls perror and exits
 *
 *  @param msg to send to perror
 */
void error(char* msg);

/** @brief Reads HTTP request, parses, and writes response
 *
 *  @param connfd the socket fd to read and write
 *  @param clientaddr the HTTP information of the client
 *  @param rootDirecotry the rootDirectory of the project
 */
void handleRequest(int connfd, struct sockaddr_in *clientaddr, char* rootDirectory);


#endif /* _REQUEST_H_ */