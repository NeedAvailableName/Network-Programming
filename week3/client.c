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
	int bytes_sent,bytes_received, sin_size;
	char task[20];
    char SERV_IP[20];
    int SERV_PORT;
    strcpy(SERV_IP, argv[1]);
    SERV_PORT = atoi(argv[2]);
	//Step 1: Construct a UDP socket
	if ((client_sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){  /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	//Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    sin_size = sizeof(struct sockaddr_in);
    bytes_sent = sendto(client_sock, "Hello", strlen("Hello") - 1, 0, (struct sockaddr *)&server_addr, sin_size);
    if(bytes_sent < 0) {
        perror("\nError: ");
        return 0;
    }
    bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
    buff[bytes_received] = '\0';
    if(bytes_received < 0) {
        perror("\nError: ");
        return 0;
    }

    // Receive task from server
    if(strcmp(buff, "send") == 0) {
        strcpy(task, "send");
    }
    else {
        strcpy(task, "receive");
    }
	//Step 3: Communicate with server
    if(strcmp(task, "send") == 0) {
        printf("sent client\n");
        while(1) {
            printf("\nInsert string to send:");
            memset(buff,'\0',(strlen(buff)+1));
            fgets(buff, BUFF_SIZE, stdin);
            
            sin_size = sizeof(struct sockaddr);
            
            bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
            if(bytes_sent < 0) {
                perror("Error: ");
                close(client_sock);
                return 0;
            }
            if(buff[0] == '\n') {
                break;
            }
        }
    }
    else {
        printf("received client\n");
        while(1) {
            sin_size = sizeof(struct sockaddr_in);
            strcpy(buff, "");
            bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
            if(bytes_received < 0){
                perror("Error: ");
                close(client_sock);
                return 0;
            }
            buff[bytes_received] = '\0';
            if(strcmp(buff, "Exit") == 0) {
                break;
            }
            printf("%s\n", buff);    
        }
    }
    close(client_sock);
	return 0;
}