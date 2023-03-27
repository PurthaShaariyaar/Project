/* A simple echo server using TCP */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>

#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256 /* buffer length */
#define FILENAME_LEN 256 /* max file length */

void reaper(int);
void file_send(int);

int main(int argc, char **argv)
{
  int sd, new_sd, client_len, port;
  struct sockaddr_in server, client;
  switch(argc){
    case 1:
    port = SERVER_TCP_PORT;
    break;
    case 2:
    port = atoi(argv[1]);
    break;
    default:
    fprintf(stderr, "Usage: %s [port]\n", argv[0]);
    exit(1);
  }

  /* Create a stream socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Can't creat a socket\n");
    exit(1);
  }

  /* Bind an address to the socket */
  bzero((char *)&server, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
    fprintf(stderr, "Can't bind name to socket\n");
    exit(1);
  }

  /* queue up to 5 connect requests */
  listen(sd, 5);
  (void) signal(SIGCHLD, reaper);
  while(1) {
    client_len = sizeof(client);
    new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
    if(new_sd < 0){
      fprintf(stderr, "Can't accept client \n");
      exit(1);
    }
    switch (fork()){
      case 0: /* child */
        (void) close(sd);
        file_send(new_sd);
        exit(0);
      default: /* parent */
        (void) close(new_sd);
        break;
      case -1:
        fprintf(stderr, "fork: error\n");
    }
  }
}

/* Transfer the file to the client */
 
 void file_send(int sd) {
    FILE *fp;
    char buf[BUFLEN];
    int error = 0;
    size_t bytes_read;
    char filename[FILENAME_LEN];

    /* receive filename from client */
    if (recv(sd, filename, FILENAME_LEN, 0) < 0) {
        perror("Error, unable to receive filename.");
        exit(1);
    }

    /* open file */
    if ((fp = fopen(filename, "rb")) == NULL) {
        error = 1;
        send(sd, &error, sizeof(error), 0);
        perror("Error opening file.");
        close(sd);
        return;
    }

    /* send file in chunks of up to BUFLEN bytes */
    while ((bytes_read = fread(buf, 1, BUFLEN, fp)) > 0) {
        if (send(sd, buf, bytes_read, 0) < 0) {
            perror("Error, unable to send file to client.");
            fclose(fp);
            close(sd);
            return;
        }
    }

    fclose(fp);
    close(sd);
}



/* reaper */
void reaper(int sig)
{
  int status;
  while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}