#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "chat_server.h"

void *recv_thread(void *param) {
    int clientSocket = *((int *) param);
    ssize_t bytesReceived;
    char receivedMessage[IP_ADDRESS_LENGTH + USERNAME_LENGTH + MESSAGE_LENGTH + 5];
    
    while (1) {
        bzero(&receivedMessage, sizeof(receivedMessage));

        // receive message from server
        bytesReceived = recv(clientSocket, receivedMessage, sizeof(receivedMessage), 0);
        if (bytesReceived == -1) {
            printf("ERROR: Receive on client socket failed with %s\n", strerror(errno));
            close(clientSocket);
            pthread_exit(0);
        }

        printf("%s\n", receivedMessage);
    }

    pthread_exit(0);
}

int chat_connect(char ipAddr[], int port) {
    int clientSocket;
    struct sockaddr_in socketAddr;
    pthread_t recvThread;
    ssize_t bytesSend;
    char message[MESSAGE_LENGTH];
    char username[USERNAME_LENGTH];

    // create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1) {
        printf("ERROR: Creation of client socket failed with %s\n", strerror(errno));
        return 1;
    }

    // initialize socketAddr struct with port and ip-Address from target server
    bzero(&socketAddr, sizeof(socketAddr));
    socketAddr.sin_family      = AF_INET;
	socketAddr.sin_addr.s_addr = inet_addr(ipAddr);
	socketAddr.sin_port        = htons(port);

    // connect socket
    if (connect(clientSocket, (struct sockaddr *) &socketAddr, sizeof(socketAddr)) == -1) {
        printf("ERROR: Connect of client socket failed with %s\n", strerror(errno));
        close(clientSocket);
        return 1;
    }

    printf("Connected to server with IP-Address '%s' on port '%d'.\n", ipAddr, port);

    // setup thread for receiving messages
    if (pthread_create(&recvThread, NULL, recv_thread, (void *) &clientSocket) != 0) {
        printf("ERROR: Thread creation failed with %s\n", strerror(errno));
        close(clientSocket);
        return 1;
    }

    // get username
    printf("Enter username:\n");
    fgets(username, sizeof(username), stdin);
    // remove new line from username
    if (username[strlen(username) - 1] == '\n') {
        username[strlen(username) - 1] = '\0';
    }
    // send username
    bytesSend = send(clientSocket, username, strlen(username), 0);
    if (bytesSend == -1) {
        printf("ERROR: Send on client socket failed with %s\n", strerror(errno));
        close(clientSocket);
        return 1;
    }

    printf("\nJoined chat with Username '%s'.\n\n", username);

    // send loop
    while(1) {
        bzero(&message, sizeof(message));

        // get message
        fgets(message, sizeof(message), stdin);
        // remove new line from message
        if (message[strlen(message) - 1] == '\n') {
            message[strlen(message) - 1] = '\0';
        }
        // send message
        bytesSend = send(clientSocket, message, strlen(message), 0);
        if (bytesSend == -1) {
            printf("ERROR: Send on client socket failed with %s\n", strerror(errno));
            close(clientSocket);
            return 1;
        }
    }

    // block read, write and close socket
	shutdown(clientSocket, SHUT_RDWR);
	close(clientSocket);

    return 0;
}