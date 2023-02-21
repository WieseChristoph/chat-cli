#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "chat_server.h"

pthread_mutex_t mutex;

chat_user_t *head = NULL;
chat_user_t *tail = NULL;

void *client_thread(void *param) {
    client_thread_params_t *clientThreadParams = (client_thread_params_t *)param;
    chat_user_t *user = malloc(sizeof(chat_user_t));
    user->socket = clientThreadParams->socket;
    user->next = NULL;
    ssize_t bytesReceived, bytesSend;
    char message[MESSAGE_LENGTH];

    // convert binary address to string
    inet_ntop(AF_INET, &clientThreadParams->socketAddr->sin_addr, user->ipAddr, sizeof(user->ipAddr));

    printf("[%s] Accepted connection.\n", user->ipAddr);

    // receive username from client
    bytesReceived = recv(user->socket, user->username, sizeof(user->username), 0);
    if (bytesReceived == -1) {
        printf("ERROR: Receive on client socket failed with %s\n", strerror(errno));
        close(user->socket);

        free(user);
        free(clientThreadParams->socketAddr);
        free(clientThreadParams);
        pthread_exit(0);
    }

    // add user to chat
    pthread_mutex_lock(&mutex);
    if (head == NULL)
    {
        head = user;
    }
    else {
        tail->next = user;
    }
    tail = user;
    pthread_mutex_unlock(&mutex);

    printf("[%s] User '%s' joined.\n", user->ipAddr, user->username);

    // chat loop
    while (1) {
        bzero(&message, sizeof(message));

        // receive message from client
        bytesReceived = recv(user->socket, message, sizeof(message), 0);
        if (bytesReceived == -1) {
            printf("ERROR: Receive on client socket failed with %s\n", strerror(errno));
            close(user->socket);

            free(clientThreadParams->socketAddr);
            free(clientThreadParams);
            pthread_exit(0);
        }

        printf("[%s] Received message: '%s'.\n", user->ipAddr, message);

        // create chat message
        char chatMessage[IP_ADDRESS_LENGTH + USERNAME_LENGTH + MESSAGE_LENGTH + 5];
        snprintf(chatMessage, sizeof(chatMessage), "[%s:%s] %s", user->ipAddr, user->username, message);

        // send chat message to all users
        chat_user_t *currUser = head;
        while (currUser != NULL) {
            bytesSend = send(currUser->socket, chatMessage, strlen(chatMessage), 0);
            if (bytesSend == -1) {
                printf("ERROR: Send on client socket failed with %s\n", strerror(errno));
                close(user->socket);

                free(clientThreadParams->socketAddr);
                free(clientThreadParams);
                pthread_exit(0);
            }
            currUser = currUser->next;
        }
    }

    printf("[%s] Closing socket.\n", user->ipAddr);

    // block read, write and close socket
    shutdown(user->socket, SHUT_RDWR);
    close(user->socket);

    free(clientThreadParams->socketAddr);
    free(clientThreadParams);
    pthread_exit(0);
}

int chat_serve(int port) {
    int listenSocket;
    struct sockaddr_in serverAddr;
    pthread_t threads[MAX_CONNECTIONS];

    bzero(threads, sizeof(threads));

    // create listen socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        printf("ERROR: Creation of listen socket failed with %s\n", strerror(errno));
        return 1;
    }

    // initialize serverAddr struct with port and allow connections from any address
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // bind socket
    if (bind(listenSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
        printf("ERROR: Binding of listen socket failed with %s\n", strerror(errno));
        close(listenSocket);
        return 1;
    }

    // listen on port with a maximum of MAX_CONNECTION of waiting clients
    if (listen(listenSocket, MAX_CONNECTIONS) == -1) {
        printf("ERROR: Listen on listen socket failed with %s\n", strerror(errno));
        close(listenSocket);
        return 1;
    }

    printf("Listening on port %d.\n", port);
    printf("Waiting for connections...\n");

    while (1) {
        client_thread_params_t *clientThreadParams = malloc(sizeof(client_thread_params_t));
        struct sockaddr_in *clientAddr = malloc(sizeof(struct sockaddr_in));
        clientThreadParams->socketAddr = clientAddr;
        unsigned clientAddrLen = sizeof(clientAddr);

        // block until a client wants to connect
        clientThreadParams->socket = accept(listenSocket, (struct sockaddr*) clientAddr, &clientAddrLen);
        if (clientThreadParams->socket == -1) {
            printf("ERROR: Accept on listen socket failed with %s\n", strerror(errno));
            continue;
        }

        int created = 0;
        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            // check if thread is dead
            if (threads[i] == 0 || pthread_tryjoin_np(threads[i], NULL) == 0)
            {
                // create new thread for client
                if (pthread_create(&threads[i], NULL, client_thread, (void *) clientThreadParams) != 0) {
                    printf("ERROR: Thread creation failed with %s\n", strerror(errno));
                    return 1;
                }
                created = 1;
                break;
            }
        }

        if (created == 0) {
            close(clientThreadParams->socket);
            free(clientThreadParams->socketAddr);
            free(clientThreadParams);
        }
    }

    close(listenSocket);

    // free users
    while (head != NULL) {
        chat_user_t *curr = head;
        head = head->next;
        free(curr);
    }

    return 0;
}