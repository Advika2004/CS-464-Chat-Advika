/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "communicate.h"
#include "pollLib.h"
#include "dict.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int socketNumber);
void processClient(int socketNumber);
void addNewSocket(int socketNumber);

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);

	// create the table
	struct Dict *table = dctCreate();
	printf("DEBUG: %d\n", table->size);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);
	serverControl(mainServerSocket);

	while(1) {
		// wait for client to connect
		clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);

		recvFromClient(clientSocket);

		// after recv from client, need to grab that out of the message

		// add to the table
		dctInsert(table, handle, clientSocket);
	
		/* close the sockets */
		close(clientSocket);
	}

	close(mainServerSocket);

	return 0;
}


void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF);
	//printf("what recvPDu is returning: %d\n", messageLen);
	if (messageLen < 0) {
		perror("recv call");
	}

	if (messageLen > 0)
	{
		printf("Message received on socket %d, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
		
		//! part b of part 8 - uncomment to test that processMsgFromServer works

		// //send the same PDU back to the client
		// int sentLen = sendPDU(clientSocket, dataBuffer, messageLen);
		// //error check that it was sent properly
		// if (sentLen < 0)
        // {
        //     perror("sendPDU failed");
        //     close(clientSocket);
        //     removeFromPollSet(clientSocket);
        // }
	}
	else if (messageLen == 0)
	{
		close(clientSocket);
		removeFromPollSet(clientSocket);
		printf("Connection closed by other side\n");
	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

void serverControl(int serverSocket){

	// first want to add the server socket in
	addToPollSet(serverSocket);

	while(1){
		// the current socket is what the pollcall will return
		int currentSocket = pollCall(-1);

		if (currentSocket == serverSocket){
			addNewSocket(currentSocket);
			continue;
		}
		else {
			processClient(currentSocket);
			continue;
		}
	}
	return;
}

void addNewSocket(int serverSocket){
	int newSocket = tcpAccept(serverSocket, DEBUG_FLAG);
	addToPollSet(newSocket);
	return;
}

void processClient(int socketNumber){
	recvFromClient(socketNumber);
	return;
}
