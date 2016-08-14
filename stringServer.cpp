//-------------------------------------------
//
// CS454 ASSIGNMENT 2
// stringServer.cpp
// Server/Client with TCP/Sockets in C++
// Rui Sun, 20479235
//
//-------------------------------------------

//With help from:
//http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf
//http://www.bogotobogo.com/cplusplus/sockets_server_client.php

#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sstream>


using namespace std;

//simple function that convert every letter after space to upper
void titlecase(char* in){
	for (int i = 0; i < strlen(in); i++){
		if (i == 0){
			if ((in[i] >= 97) && (in[i] <= 122)){
				in[i] = in[i] - 32;
			}
		}
		else{
			if (in[i - 1] == 32){
			    if (in[i] >= 97 && in[i] <= 122){
				    in[i] = in[i] - 32;
			    }
            }
			else{
				if ((in[i] >= 65) && (in[i] <= 90)){
					in[i] = in[i] + 32;
				}
			}
		}
	}
}

//server
int main(int argc,char *argv[]) {
	char hostname[128];
	int listenfd;
	int bytes_sent;
	int bytes_recv;
	int fdmax;
	int connectionfd;
	fd_set master, readfd;
	socklen_t addrlen;
	struct sockaddr_in hostaddress;
	struct sockaddr_storage clientaddr;
	
	// Argument count check,should have no parameters.
	if (argc > 1){
	cout << "Usage: ./stringServer" << endl;
	exit(1);
	}

	//get socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	listen(listenfd, 5);

	//get port
	hostaddress.sin_family = AF_INET;
	hostaddress.sin_addr.s_addr = htonl(INADDR_ANY);
	hostaddress.sin_port = htons(0);
	
	//bind to port
	bind(listenfd, (sockaddr *) &hostaddress, sizeof(hostaddress)); 

	//print server address
	gethostname(hostname, 128);
	cout << "SERVER_ADDRESS " << hostname << endl;

	addrlen = sizeof(hostaddress);

	//print server port
	if (getsockname(listenfd, (sockaddr *)&hostaddress, &addrlen) == -1) {
		perror("getsockname");
	}
	cout << "SERVER_PORT " << ntohs(hostaddress.sin_port) << endl;

	//Select
	//listen is max so far,add listenfd to master set,
	fdmax = listenfd; 
	// add listener to master
	FD_SET(listenfd, &master);
	for(;;) {
		readfd = master;
		if (select(fdmax + 1, &readfd, NULL, NULL, NULL) == -1) {
			cerr <<"select"<<endl;
			exit(1);
		}
		for (int i = 0; i <= fdmax; i++) {
		//if it is valid fd
			if (FD_ISSET(i, &readfd)) {
				if (i == listenfd) {
					//accept connection, get info
					addrlen = sizeof(clientaddr);
					connectionfd = accept(listenfd, (sockaddr *)&clientaddr, &addrlen);
					if (connectionfd == -1) {
						continue;
					}
					else {
						 // add to master set
						if (fdmax < connectionfd) {
							fdmax = connectionfd;
						}
						//add to master
						FD_SET(connectionfd, &master);
					}
				}
				else {
					//read 4 bytes
					char len[4];
					read(i, len, 4);

					char buffer[USHRT_MAX];
					if ((bytes_recv = recv(i, buffer, USHRT_MAX, 0)) <= 0) {
						FD_CLR(i, &master);
						close(i);
					}
					else {

						//convert the string !
						string ret(reinterpret_cast<char*>(len), 4);
						titlecase(buffer);
						string temp(buffer);
						ret.append(temp);
						ret[ret.length()] = '\0';

						// Thanks to http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf again!
						if ((bytes_sent = send(i, ret.c_str(), ret.length(), 0)) == -1) {
							cerr << "sent failed" << endl;
							exit(1);
						}
						//cleanup......finally
						memset(buffer, 0, sizeof(buffer));
					}
				}
			}
		}
	}
}
