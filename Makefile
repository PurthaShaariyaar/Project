# Makefile for running echo_client.c and echo_server.c 
# Establish a TCP connection 

CC=gcc
CFLAGS=-Wall

all: echo_client echo_server

echo_client: echo_client.c
	$(CC) $(CFLAGS) -o echo_client echo_client.c

echo_server: echo_server.c
	$(CC) $(CFLAGS) -o echo_server echo_server.c

clean:
	rm -f echo_client echo_server