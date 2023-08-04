#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024

void charFromMessage(char* mess, char* str);
void numberFromMessage(char* mess, char* str);
int validMessages(char *mess);
FILE* checkFile(char *mess);
void readFile(FILE* f, char *mess);

int main(int argc, char **argv)
{
    int port_number;
 	if(argc != 2) {
 		perror(" Error Parameter! Please input only port number\n ");
 		exit(0);
 	}
 	if((port_number = atoi(argv[1])) == 0) {
 		perror(" Please input port number\n");
 		exit(0);
 	}
	int listen_sock, conn_sock; /* file descriptors */
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;
	
	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	// server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
	server.sin_port = htons(port_number);
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: ");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: ");
		return 0;
	}
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("\nYou got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		
		//start conversation
		int choice = -1;
		int count = 0;
		int chosen = 0;
		while(1){
			//receives message from client
			if(choice < 1 || choice > 2){
				printf("[%d]\n", count++);
				char* menu = malloc(sizeof(char) * BUFF_SIZE);
				printMenu(menu);
				if(bytes_received > 0)
					send(conn_sock, menu, strlen(menu), 0);
				bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
				if (bytes_received <= 0){
					printf("\nConnection closed");
					break;
				}
				else{
					recv_data[bytes_received] = '\0';
					printf("\nReceive: %s", recv_data);
				}
				choice = checkChoice(recv_data);
				continue;
			}
			printf("[%d]\n", count++);
			//echo to client
			if(chosen == 0){
				switch (choice)
				{
					case 1:
						char *string = malloc(sizeof(char) * BUFF_SIZE);
						memset(string, 0, strlen(string));
						strcpy(string, "You chosen send string\n");
						send(conn_sock, string, strlen(string), 0);
						break;
					case 2:
						char *file = malloc(sizeof(char) * BUFF_SIZE);
						memset(file, 0, strlen(file));
						strcpy(file, "You chosen send file\n");
						send(conn_sock, file, strlen(file), 0);
						break;
					default:
						break;
				}
				chosen = 1;
			}
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
			if (bytes_received <= 0){
				printf("\nConnection closed");
				break;
			}
			else{
				recv_data[bytes_received] = '\0';
				printf("\nReceive: %s", recv_data);
			}

			printf("[%d]\n", count++);
			char *mess = malloc(sizeof(char) * BUFF_SIZE);
			memset(mess, 0, strlen(mess));
			if (bytes_sent <= 0 || strcmp(recv_data, "bye\n") == 0){
				printf("\nConnection closed");
				char* bye = malloc(sizeof(char) * BUFF_SIZE);
				strcpy(bye, "\nBye bye!\n");
				send(conn_sock, bye, strlen(bye), 0);
				close(conn_sock);	
				break;;
			}
			switch (choice){
				case 1:

					if(validMessages(recv_data) == 0){
						char warning[20] = "Invalid character!";
						send(conn_sock, warning, strlen(warning), 0);
						continue;
					}

					char number[BUFF_SIZE];
					char character[BUFF_SIZE];
					memset(number, 0, strlen(recv_data));
					memset(character, 0, strlen(recv_data));
					numberFromMessage(recv_data, number);
					charFromMessage(recv_data, character);
					strcat(mess, character); 
					strcat(mess, "\n");
					strcat(mess, number);
					strcat(mess, "\0");
					bytes_sent = send(conn_sock, mess, strlen(recv_data), 0); 
					break;
				case 2:
					
					char *path = malloc(sizeof(char) * BUFF_SIZE);
					filePathFromMessage(recv_data, path);
					FILE *f = checkFile(path);
					if(f != NULL){
						printf("Open file %s success\n", path);
						readFile(f, mess);
						bytes_sent = send(conn_sock, mess, strlen(mess), 0);
						free(mess);
					}else{
						char *failed = malloc(sizeof(char) * BUFF_SIZE);
						sprintf(failed, "Open file %s failed\n", path);
						bytes_sent = send(conn_sock, failed, strlen(failed), 0);
						printf("Open file %s failed\n", path);
					}
					break;
				
			}

		}//end conversation
		close(conn_sock);	
	}
	
	close(listen_sock);
	return 0;
}

void printMenu(char *mess){
	strcat(mess, "\n-----------MENU---------\n");
	strcat(mess, "1. Send string.\n");
	strcat(mess, "2. Send file.\n");
}

void filePathFromMessage(char* mess, char* str) {
	int i;
	int j = 0;
	for(i = 0; i < strlen(mess); i++) {
		if(isalpha(mess[i]) != 0 || isdigit(mess[i]) != 0 || mess[i] == '.') {
			str[j++] = mess[i];
		}
	}
}

void charFromMessage(char* mess, char* str) {
	int i;
	int j = 0;
	for(i = 0; i < strlen(mess); i++) {
		if(isalpha(mess[i]) != 0) {
			str[j++] = mess[i];
		}
	}
}

void numberFromMessage(char* mess, char* str) {
	int i;
	int j = 0;
	for(i = 0; i < strlen(mess); i++) {
		if(isdigit(mess[i]) != 0) {
			str[j++] = mess[i];
		}
	}
}

int validMessages(char* mess) {
	int i;
	for(i = 0; i < strlen(mess); i++) {
		if((isalpha(mess[i]) == 0) && (isdigit(mess[i]) == 0) && mess[i] != '\n') {
			return 0;
		}
	}
	return 1;
}

FILE* checkFile(char *mess){
    FILE* f = fopen(mess, "rb");
    return f;
}

void readFile(FILE* f, char *mess){
    char c;
    int i = 0;
    while(!feof(f)){
        c = fgetc(f);
        if(c != EOF)
            mess[i++] = c;
    }
    mess[i] = '\0';
}

int checkChoice(char *mess){
	for(int i = 0; i < strlen(mess); i++) {
		if(isalnum(mess[i])){
			if(isdigit(mess[i]))
				return mess[i] - '0';
			return -1;
		}
	}
	return -2;
}


