#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define SERV_TCP_PORT 9000 //server's default port number
#define MAX_SIZE 5000
int *socketList;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *socketListener(void *socketdesc){
  int l = *(int *)socketdesc;
  char info[5000]; // max char count is 5000
  size_t f;

  while(1) {
    f = read(l, info, sizeof(info)); //tries to read from certain socket
    printf("%s\n", info);
    if(f < 0){ //if f < 0 it means that there was an error reading from the socket connection
      perror("Error reading from socket");
      break;
    }

    if(f > 0){ // if f > 0 it means that it read something from the socket connection, this means we should send read string to every client
      for(int i = 0; i < 100; i++){ // iterating through array of socket connections
        if(socketList[i] != 0 && socketList[i] != l){ // if the socket is not 0 and not the current socket
          write(socketList[i],info, sizeof(info)); //write the string to each socket
        }

      }
    }
    memset(info, 0, sizeof(info)); //clear the buffer
  }

  close(l); //close the socket connection
}

int main(int argc, char *argv[]){
  int sockfd, newsockfd, clilen, counter; //counter will keep track of index in socketList
  int i = 0;
  struct sockaddr_in cli_addr, serv_addr;
  //int *socketList = NULL;
  int port;
  pthread_t l[100];
  socketList = (int *)calloc(100 , (sizeof(int))); // an array of 100 integers established by malloc
  if(socketList == NULL){ // error checking
    perror("Malloc error");
    exit(1);
  }
  /*command line: serve [port_number]*/
  if (argc == 2)
  {
    sscanf(argv[1],"%d",&port);/*read the port number if provided*/
  } else port = SERV_TCP_PORT;

  /*Open TCP socket (an Internet stream socket)*/
  /*socket(int domain, int type,int protocol)*/
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("can't open stream socket");
    exit(1);
  }

  //Now bind the local address, so that the client can send to server
  /* bind the local address, so that the client can send to server */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("can't bind local address");
    exit(1);
  }

  //Listen to the socket
  listen(sockfd, 100);

  for(;;){
    printf("Server is waiting\n");
    //Wait for a connection from a client; this is an iterative server
    if(i < 100){ //over max connections
      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

      if (newsockfd < 0){
        perror("can't accept client connection");
        exit(1);
      }

      pthread_mutex_lock(&lock);

      if(newsockfd < 0){
        perror("cant accept local connection");
        exit(1);
      }
      socketList[i] = newsockfd;
      printf("%d", newsockfd);
      if(pthread_create(&l[i],NULL, socketListener, (void *) &newsockfd) == -1){
        perror("thread error");
        exit(-1);
      }

      i +=1;
      pthread_mutex_unlock(&lock);

    }
  }
}

