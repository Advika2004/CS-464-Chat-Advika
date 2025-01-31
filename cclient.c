/******************************************************************************
* myClient.c
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

#define MAXBUF 1024
#define DEBUG_FLAG 1

void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int serverSocket);
void processMsgFromServer(int serverSocket);
void processStdin(int serverSocket);

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG);
	clientControl(socketNum);
	
	close(socketNum);
	return 0;
}

void sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	sendLen = readFromStdin(sendBuf);
	printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
	sent = sendPDU(socketNum, sendBuf, sendLen);

	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	//printf("Enter data: ");
	fflush(stdout);
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 3)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}
}

void clientControl(int serverSocket){

	// first want to add the server socket and the stdin to the poll set
	setupPollSet();
	addToPollSet(serverSocket);
	addToPollSet(STDIN_FILENO);

	int printPromptFlag = 1;

	while(1){

		if (printPromptFlag == 1) {
			printf("Enter data: ");
			fflush(stdout);
			printPromptFlag = 0;
		}
	
		// the current socket is what the pollcall will return 
		int currentSocket = pollCall(-1);
		if (currentSocket == serverSocket){
			processMsgFromServer(currentSocket);
			printPromptFlag = 1;
		}
		else if (currentSocket == STDIN_FILENO) {
			processStdin(serverSocket);
			printPromptFlag = 1;
		}
	}
}


void processMsgFromServer(int serverSocket)
{

	uint8_t buffer[MAXBUF];
	int serverStatus = recvPDU(serverSocket, buffer, MAXBUF);

	if (serverStatus == 0)
    {
        printf("Server has terminated.\n");
        close(serverSocket);
        exit(0);
    }
	else if (serverStatus < 0) {
		perror("recv call");
	}
	else {
		printf("\nMessage received on socket %d, length: %d Data: %s\n", serverSocket, serverStatus, buffer);
	}
}


void processStdin(int serverSocket)
{
	sendToServer(serverSocket);
	return;
}