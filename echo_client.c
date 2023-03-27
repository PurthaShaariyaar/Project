/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>


#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256 /* buffer length */


int main(int argc, char **argv)
{
  int n;
  int sd, port;
  struct hostent *hp = NULL;
  struct sockaddr_in server;
  char *host, rbuf[BUFLEN], sbuf[BUFLEN];
  switch(argc){
    case 2:
      host = argv[1];
      port = SERVER_TCP_PORT;
      break;
    case 3:
      host = argv[1];
      port = atoi(argv[2]);
      break;
    default:
      fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
      exit(1);
  }

  /* Create a stream socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "Can't creat a socket\n");
    exit(1);
  }

  bzero((char *)&server, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  hp = gethostbyname(host);
  if (hp == NULL) {
      if ( inet_aton(host, (struct in_addr *) &server.sin_addr) == 0 ){
          fprintf(stderr, "Can't get server's address\n");
          exit(1);
      }
  }

  /* Connecting to the server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
    fprintf(stderr, "Can't connect \n");
    exit(1);
  }

  /* Request user to enter the name of the file */
  printf("Please enter the filename: ");
  fgets(sbuf, BUFLEN, stdin);
  sbuf[strlen(sbuf)-1] = '\0';

  /* Send the filename to the server */
  write(sd, sbuf, strlen(sbuf));

  /* Receive data from the server */
  n = read(sd, rbuf, BUFLEN); 

  if (n < 0) { // Error handling when reading file 
    fprintf(stderr, "Error no response from the server, file cannot be read.");
  }

  if (rbuf[0] == 'E') { // Skip error character
    fprintf(stderr, "Error: %s\n", rbuf+1); 
    exit(1);
  }

  else if (rbuf[0] == 'F') { // Received file > write to local file 
    FILE *fp = fopen("downloaded_file", "wb"); 
    fwrite(rbuf+1, sizeof(char), n-1, fp); 
    while((n = read(sd, rbuf, BUFLEN)) > 0) {
      fwrite(rbuf, sizeof(char), n, fp); 
    }
    fclose(fp);
    printf("File downloaded.\n"); 
  }

  else {
    fprintf(stderr, "Error unable to read response from server.\n");
  }

  /* Close TCP Connection */
  close(sd);
  return(0);
}