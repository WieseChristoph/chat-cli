#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat_cli.h"
#include "chat_server.h"
#include "chat_client.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("%s", CLI_HELP_MESSAGE);
        return 1;
    }

    if (strcmp(argv[1], "serve") == 0) {
        if (argc < 3) {
            printf("%s", SERVE_HELP_MESSAGE);
            return 1;
        }

        return chat_serve(atoi(argv[2]));
    } else if (strcmp(argv[1], "connect") == 0) {
        if (argc < 4) {
            printf("%s", CONNECT_HELP_MESSAGE);
            return 1;
        }

        return chat_connect(argv[2], atoi(argv[3]));
    }
    else
    {
        printf("%s", CLI_HELP_MESSAGE);
        return 1;
    }

    return 0;
}