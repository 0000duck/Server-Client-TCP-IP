//-------------------------------------------
//
// CS454 ASSIGNMENT 2
// stringClient.cpp
// Server/Client with TCP/Sockets in C++
// Rui Sun, 20479235
//
//-------------------------------------------

//With help from:
//http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf
//http://www.bogotobogo.com/cplusplus/sockets_server_client.php

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <limits.h>
using namespace std;

int main(int argc, char *argv[])
{
	int sockfd, bytes_sent, bytes_recv;
	struct addrinfo hints, *server, *p;
	int rv;
	string msg;

	//get host address and port from enviroment variables.
	char *port = getenv("SERVER_PORT");
	char *address = getenv("SERVER_ADDRESS");


	// Argument count check,should have no parameters.
	if (argc > 1){
		cout << "Usage: ./stringClient" << endl;
		exit(1);
	}

	//make sure its empty structure
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Error checking
	// based on http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf
	if ((rv = getaddrinfo(address, port, &hints, &server)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = server; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	//job's done!
	freeaddrinfo(server);

	for (;;){
		if (getline(cin, msg)) {
			pid_t pid = fork();
			//successive request, sleeping time!
			if (pid == 0) {
				sleep(2);
				// communicate by sending a hex of length and the message following
				unsigned int len = msg.length() + 1;
				unsigned char bytes[4];
				for (int i = 0; i < 4; i++){
					bytes[i] = (len >> (24 - (8 * i))) & 0xFF;
				}
				string ret(reinterpret_cast<char*>(bytes), 4);

				//append to input
				ret.append(msg);
				ret[ret.length()] = '\0';

				// get a buffer for outputs ready
				char buffer[USHRT_MAX];

				//get how many bytes sent
				if ((bytes_sent = send(sockfd, ret.c_str(), ret.length(), 0)) == -1) {
					perror("send");
					exit(1);
				}
				//read 4 bytes
				read(sockfd, bytes, 4);

				//get how many bytes received and output the message
				if ((bytes_recv = recv(sockfd, buffer, USHRT_MAX - 1, 0)) == -1) {
					perror("recv");
					exit(1);
				}
				buffer[bytes_recv] = '\0';
				string out(buffer);
				cout << "Server: " << out << endl;
				break;
			}
		}
	}
	close(sockfd);
}