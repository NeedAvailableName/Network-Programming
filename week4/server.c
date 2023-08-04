/*UDP Echo Client*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "LinkedList.h"

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Error argument\n");
        return 0;
    }
    int server_sock;
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client_send, client_receive; /* client's address information */
    int sin_size;
    int PORT = atoi(argv[1]);
    char username[BUFF_SIZE], password[BUFF_SIZE];
    char message[BUFF_SIZE], number[BUFF_SIZE], character[BUFF_SIZE];
    int logged_in = 0;
    accountList *root = NULL;

    // Step 1: Construct a UDP socket
    if((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Error: \n");
        return 0;
    }

    // Step 2: Bind address to socket
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */
    bzero(&(server.sin_zero),8); /* zero the rest of the structure */

    if((bind(server_sock, (struct sockaddr *)&server, sizeof(server))) == -1) {
        perror("Error: \n");
        return 0;
    }

    // Step 3: Connect to client
    sin_size = sizeof(struct sockaddr_in);
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client_send, &sin_size);
    if(bytes_received < 0) {
        perror("Error: \n");
        return 0;
    }

    // send message to client
    bytes_sent = sendto(server_sock, "send", strlen("send"), 0, (struct sockaddr *)&client_send, sin_size);
    if(bytes_sent < 0) {
        perror("Error: \n");
        return 0;
    }
    // Step 4: Communicate with clients
    readToList(&root);
    while(logged_in == 0) {
        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client_send, &sin_size);
        if(bytes_received < 0) {
            perror("Error: \n");
            return 0;
        }

        int i = 0, j = 0, k = 0;
        char *token;
        buff[bytes_received] = '\0';
        token = strtok(buff, "-");
        while(token != NULL) {
            if(i == 0) {
                strcpy(username, token);
            }
            else if(i == 1) {
                strcpy(password, token);
            }
            token = strtok(NULL, "-");
            i++;
        }
        username[strlen(username) - 1] = '\0';

        // login with username and password
        signIn(root, username, password, message);
        if(strcmp(message, "OK") == 0) {
            logged_in = 1;
        }

        // Send message to client
        bytes_sent = sendto(server_sock, message, strlen(message), 0, (struct sockaddr *)&client_send, sin_size);
        if(bytes_sent < 0) {
            perror("Error: \n");
            return 0;
        }
    }

    // change password
    while(1) {
        // Receive new password from client
        bytes_received = recvfrom(server_sock, password, BUFF_SIZE - 1, 0, (struct sockaddr *)&client_send, &sin_size);
        if(bytes_received < 0) {
            perror("Error: ");
            return 0;
        }
        password[bytes_received] = '\0';

        // If received "bye" message
        if(strcmp(password, "bye") == 0) {
            signOut(root, username, message);
            // Send message to client
            bytes_sent = sendto(server_sock, message, strlen(message), 0, (struct sockaddr *)&client_send, sin_size);
            if(bytes_sent < 0) {
                perror("Error: ");
                return 0;
            }
            break;
        }
        // Change password and print 2 string
        changePassword(root, username, password, message);
        // Send message to client
        bytes_sent = sendto(server_sock, message, strlen(message), 0, (struct sockaddr *)&client_send, sin_size);
        if(bytes_sent < 0) {
            perror("Error: ");
            return 0;
        }
    }

    // Step 5: Close socket
    close(server_sock);
    writeToFile(root);
    freeList(root);
    return 0;
}
