#include <netinet/in.h>

#define MAX_CONNECTIONS 6
#define IP_ADDRESS_LENGTH 128
#define USERNAME_LENGTH 256
#define MESSAGE_LENGTH 1024

struct client_thread_params {
    int socket;
    struct sockaddr_in *socketAddr;
} typedef client_thread_params_t;

struct chat_user {
    int socket;
    char ipAddr[IP_ADDRESS_LENGTH];
    char username[USERNAME_LENGTH];
    struct chat_user *next;
} typedef chat_user_t;

void *client_thread(void *param);
int chat_serve(int port);