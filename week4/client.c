/*UDP Echo Client*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Error argument\n");
        return 0;
    }

    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr;
    int bytes_sent, bytes_received, sin_size;
    char task[20];
    char SERV_IP[20];
    int SERV_PORT;
    strcpy(SERV_IP, argv[1]);
    SERV_PORT = atoi(argv[2]);
    char number[BUFF_SIZE], character[BUFF_SIZE];
    char username[BUFF_SIZE], password[BUFF_SIZE], message[BUFF_SIZE];
    //Step 1: Construct a UDP socket
    if((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error: \n");
        return 0;
    }

    // Step 2: Define the address of the server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    sin_size = sizeof(struct sockaddr_in);
    bytes_sent = sendto(client_sock, "Hello", strlen("Hello") - 1, 0, (struct sockaddr *)&server_addr, sin_size);
    if(bytes_sent < 0) {
        perror("Error: \n");
        return 0;
    }
    bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
    if(bytes_received < 0) {
        perror("Error: \n");
        return 0;
    }

    while(1) {
        // get username and password
        fgets(username, BUFF_SIZE, stdin);
        printf("Insert password\n");
        fgets(password, BUFF_SIZE, stdin);

        // concatinate username and password and send to server
        strcpy(buff, username);
        strcat(buff, "-");
        strcat(buff, password);

        bytes_sent = sendto(client_sock, buff, strlen(buff) - 1, 0, (struct sockaddr *)&server_addr, sin_size);
        if(bytes_sent < 0) {
            perror("Error: \n");
            return 0;
        }

        // Receive message from server
        bytes_received = recvfrom(client_sock, message, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
        message[bytes_received] = '\0';
        if(bytes_received < 0) {
            perror("Error: \n");
            return 0;
        }
        
        printf("%s\n", message);
        if(strcmp(message, "OK") == 0) {
            break;
        }
    }

    // change password
    while(1) {
        // get new password
        fgets(password, BUFF_SIZE, stdin);
        bytes_sent = sendto(client_sock, password, strlen(password) - 1, 0, (struct sockaddr *)&server_addr, sin_size);
        if(bytes_sent < 0) {
            perror("Error: \n");
            return 0;
        }

        // Receive message from server
        bytes_received = recvfrom(client_sock, message, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
        message[bytes_received] = '\0';
        if(bytes_received < 0) {
            perror("Error: \n");
            return 0;
        }
        // Search sub-string "Goodbye" in message, if found, close connection
        if(strstr(message, "Goodbye") != NULL) {
            printf("%s\n", message);
            break;
        }

        // Search sub-string "Error" in message, if found, continue
        if(strstr(message, "Error") != NULL) {
            printf("%s\n", message);
            continue;
        }

        // Search sub-string "OK" in message, if found, continue
        if(strstr(message, "OK") != NULL) {
            char *token;
            int i = 0, j = 0, k = 0;
            token = strtok(message, "-");
            while(token != NULL) {
                if(i == 1) {
                    strcpy(number, token);
                }
                else if(i == 2) {
                    strcpy(character, token);
                }
                token = strtok(NULL, "-");
                i++;
            }

            // If Number include @ character, this string is equal NULL
            if(strcmp(number, "@") != 0) {
                printf("%s\n", number);
            }
            
            // If Character include @ character, this string is equal NULL
            if(strcmp(character, "@") != 0) {
                printf("%s\n", character);
            }
            continue;
        }
    }

    // Close socket
    close(client_sock);
    return 0;
}