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

#define BUFF_SIZE 1024

void string_process(char *buff, char *character, char *number) {
    // If received string include non-digit or non-alphabet character,
    // return error message
    int i;
    // Ignore the last character "\n"
    buff[strlen(buff) - 1] = '\0';

    for(i = 0; i < strlen(buff); i++) {
        if(isdigit(buff[i]) == 0 && isalpha(buff[i]) == 0) {
            strcpy(buff, "Error\0");
            return;
        }
    }
    // Split received string into character and number sub-string
    int j = 0;
    int k = 0;
    for(i = 0; i < strlen(buff); i++) {
        if(isdigit(buff[i]) != 0) {
            number[j] = buff[i];
            j++;
        }
        else {
            character[k] = buff[i];
            k++;
        }
    }
    // Copy character and number sub-string to buff
    strcpy(buff, number);
    strcat(buff, "\n");
    strcat(buff, character);
    strcat(buff, "\n");
    buff[strlen(buff)] = '\0';
    // printf("\nbuff: %s", buff);
}

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
    char character[BUFF_SIZE], number[BUFF_SIZE];

    // Step 1: Construct a UDP socket
    if((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("\nError: ");
        return 0;
    }

    // Step 2: Bind address to socket
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */
    bzero(&(server.sin_zero),8); /* zero the rest of the structure */

    if((bind(server_sock, (struct sockaddr *)&server, sizeof(server))) == -1) {
        perror("\nError: ");
        return 0;
    }

    // Step 3: Connect to client
    sin_size = sizeof(struct sockaddr_in);
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client_send, &sin_size);
    if(bytes_received < 0) {
        perror("\nError: ");
        return 0;
    }

    // Send message to client, mark this client is send client
    bytes_sent = sendto(server_sock, "send", strlen("send"), 0, (struct sockaddr *)&client_send, sin_size);
    printf("Send message\n");
    if(bytes_sent < 0) {
        perror("\nError: ");
        return 0;
    }

    // Connect to second client - client_receive
    sin_size = sizeof(struct sockaddr_in);
    bytes_received = recvfrom(server_sock, buff, BUFF_SIZE, 0, (struct sockaddr *)&client_receive, &sin_size);
    if(bytes_received < 0) {
        perror("\nError: ");
        return 0;
    }

    // Send message to client, mark this client is receive client
    bytes_sent = sendto(server_sock, "receive", strlen("receive"), 0, (struct sockaddr *)&client_receive, sin_size);
    if(bytes_sent < 0) {
        perror("\nError: ");
        return 0;
    }

    // Step 4: Communicate with clients
    while(1) {
        // Set error, character and number string to null
        memset(character, '\0', (strlen(character) + 1));
        memset(number, '\0', (strlen(number) + 1));
        sin_size = sizeof(struct sockaddr_in);
        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client_send, &sin_size);

        // Call recvfrom() to receive message from client
        if(bytes_received < 0) {
            perror("\nError: ");
        }
        else {
            buff[bytes_received] = '\0';
        }
        printf("Server received: %s\n", buff);
        // If received NULL string, send "Exit" message to receive client
        // and close connection
        if(buff[0] == '\n') {
            bytes_sent = sendto(server_sock, "Exit", strlen("Exit"), 0, (struct sockaddr *)&client_receive, sin_size);
            printf("Client disconnected!\n");
            break;
        }

        // Process received string
        string_process(buff, character, number);
        bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)&client_receive, sin_size);
        if(bytes_sent < 0) {
            perror("\nError: ");
        }
    }
    close(server_sock);
    return 0;
}