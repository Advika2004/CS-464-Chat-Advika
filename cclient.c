/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include "networks.h"
#include "safeUtil.h"
#include "communicate.h"
#include "pollLib.h"
#include "cclient.h"

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

// gets the buffer from readfromstdin, parses it properly, stores stuff into array of pointers
char** parseLine(uint8_t *buffer){

	static char* chunks[MAX_CHUNKS];
	int i = 0;
    char *firstChunk = strtok((char*)buffer, " ");
	chunks[i] = firstChunk;
	i++;

	if (strcmp(firstChunk, "%M") == 0) { 
       char *handle = strtok(NULL, " "); 
	   chunks[i] = handle;  
	   i++;
       char *message = strtok(NULL, "\n"); 
	   chunks[i] = message;
	   i++;
    }

	if (strcmp(firstChunk, "%C") == 0) { 
       char *number = strtok(NULL, " ");

	   int numHandles = atoi(number);

	   if (numHandles < 2 || numHandles > 9){
		printf("Too many or too little clients specified, please re-enter between 2-9 other clients\n");
		return NULL;
	   }

	   int j = 0;
	   for (j = 0; j < numHandles; j++){
		char* curHandle = strtok(NULL, " ");
		chunks[i] = curHandle;
		i++;
	   }

       char *message = strtok(NULL, "\n"); 
	   chunks[i] = message;
	   i++;
    }

	if (strcmp(firstChunk, "%B") == 0) { 
       char *message = strtok(NULL, "\n"); 
	   chunks[i] = message;  
	   i++;
    }

	if (strcmp(firstChunk, "%L") == 0) { 
	   //fill this in later idk what this does yet 
    }

	chunks[i] = NULL;
	return chunks;
}


void sendToServer(int socketNum)
{
	uint8_t sendBuf[MAXBUF];   
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	int lengthRead = 0;
	
	// how much was read from the user input, now need to parse this
	lengthRead = readFromStdin(sendBuf);

	printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);
	
	sent = sendPDU(socketNum, sendBuf, sendLen);

	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
}

//reads the command line into a buffer with a maximum 1400 characters and handles the error for that
//if it returns 0 that means invalid answer
int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
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
	
	// error checks for if the message given is larger than max amount
	//if that goes over, print error message, ignore all input until \n, ignore command, don't send anything
	if (inputLen - 1 > MAX_MESSAGE_LENGTH){
		printf("Error: Input length exceeds maximum input message length %d\n", MAX_MESSAGE_LENGTH);

		// go back through the entire thing and clear it
		if (aChar != '\n') {
        while ((aChar = getchar()) != '\n' && aChar != EOF) {
            // Discard extra characters
        	}
   		}
		return 0;
	}

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
			printf("$: ");
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