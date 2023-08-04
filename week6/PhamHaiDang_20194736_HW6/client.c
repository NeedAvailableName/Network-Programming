#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {

	if(argc != 3) {
		printf("Error\n");
		exit(1);
	}

	int client_sock;
	struct sockaddr_in server_addr; /* server's address information */
	char buff[BUFF_SIZE];
	int msg_len, bytes_sent, bytes_received;
	char SERVER_ADDR[16];
	int SERVER_PORT;
	char username[BUFF_SIZE], password[BUFF_SIZE];

	strcpy(SERVER_ADDR, argv[1]);
	SERVER_PORT = atoi(argv[2]);
	
	//Step 1: Construct socket
	if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        // Call socket() to create a socket
        perror("Error: ");
        return 0;
    }
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	while(1) {
		//send message
		// Get username and password from user
        printf("\nInsert username\n");
		fgets(username, BUFF_SIZE, stdin);
        username[strlen(username) - 1] = '\0';
        printf("Insert password\n");
        fgets(password, BUFF_SIZE, stdin);
        password[strlen(password) - 1] = '\0';
		
		strcpy(buff, username);
		strcat(buff, " ");
		strcat(buff, password);

		msg_len = strlen(buff);
		if(msg_len == 0) break;
		
		bytes_sent = send(client_sock, buff, msg_len, 0);
		if(bytes_sent < 0) {
			perror("Error: ");
            close(client_sock);
            return 0;
		}
		
		// Receive message from server
		bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
		if(bytes_received < 0) {
			printf("\nError!Cannot receive data from sever!\n");
			break;
		}
		
		buff[bytes_received] = '\0';
		printf("Reply from server: %s", buff);
	}
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
