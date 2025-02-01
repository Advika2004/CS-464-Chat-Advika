
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

// functoons
void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int serverSocket);
void processMsgFromServer(int serverSocket);
void processStdin(int serverSocket);

// structs
typedef struct PercentMMessage {
    uint8_t handle[101];
    uint8_t message[200];
}PercentMMessage;

// globals
#define MAX_MESSAGE_LENGTH 1400
#define MAXBUF 1024
#define DEBUG_FLAG 1
#define MAX_CHUNKS 12