#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024

enum MessageType{
	SIGNIN = 1,
	SIGNUP,
	SUCCESS,
	FAILED,
	BLOCKED
} Mess;

void messageToString(char* reply, int MessageType){
	reply[0] = '0' + MessageType;
	reply[1] = '\0';
}

int messCmp(char* reply, int MessageType){
	char* mess = malloc(sizeof(char) * 2);
	mess[0] = '0' + MessageType;
	mess[1] = '\0';
	int result = strcmp(reply, mess);
	free(mess);
	return result;
}

void processData(char*in, char*out){
	strcpy (out, in);
}

int receiveData(int s, char*buff, int size, int flags){
	int n;
	n = recv(s, buff, size, flags);
	if(n < 0)
		perror("Error: ");
	return n;
}

int sendData(int s, char*buff, int size, int flags){
	int n;
	n = send(s, buff, size, flags);
	if(n < 0)
		perror("Error: ");
	return n;
}

void printMenu(){
	char *mess = (char*)malloc(sizeof(char) * MAX);
	strcpy(mess, "\n-----------MENU---------\n");
	strcat(mess, "1. Sign in.\n");
	strcat(mess, "2. Sign up.\n");
	printf("%s\n", mess);
	free(mess);
}

int main(int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 3)
	{
		printf("Format: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	int client_sock;
	int choice;
	char username[MAX], password[MAX], buff[MAX], *mess;
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received;
	// Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	// Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit! ");
		return 0;
	}
	mess = malloc(sizeof(char) * 2);
	// Communicate with server
	//Menu sign up/sign in
	while(messCmp(mess, SIGNIN) != 0 && messCmp(mess, SIGNUP) != 0){
		printMenu();
		printf("Please enter your choice: ");
		scanf("%d", &choice);
		switch(choice){
			case SIGNIN:
				messageToString(mess, SIGNIN);
				break;
			case SIGNUP:
				messageToString(mess, SIGNUP);
				break;
		}
	}
	//send
	// printf("-%s-%ld\n", mess, strlen(mess));
	if (0 >= (bytes_sent = sendData(client_sock, mess, strlen(mess), 0))){
		{
			printf("\nConnection closed!\n"); 
			return 0;
		}
	}
	memset(mess, '\0', bytes_sent);
	// receive
	if (0 >= (bytes_received = receiveData(client_sock, mess, bytes_sent, 0)))
	{
		printf("\nError!Cannot receive data from sever!\n");
		return 0;
	}
	fgetc(stdin);
	// printf("%s-%d\n", mess, messCmp(mess, SIGNIN));
	if(messCmp(mess, SIGNIN) == 0){
		// username
		while(messCmp(buff, SUCCESS) != 0){
			puts("Please enter username and password");
			printf("Username: ");
			fgets(username, MAX, stdin);
			username[strcspn(username, "\n")] = '\0';
			// send
			if (0 >= (bytes_sent = sendData(client_sock, username, strlen(username), 0)))
			{
				printf("\nConnection closed!\n"); 
				return 0;
			}
			memset(buff, '\0', MAX);
			// receive
			if (0 >= (bytes_received = receiveData(client_sock, buff, MAX - 1, 0)))
			{
				printf("\nError!Cannot receive data from sever!\n");
				return 0;
			}

			buff[bytes_received] = '\0';
			if (messCmp(buff, FAILED) == 0)
			{
				puts("Username not found\n");
				continue;
			}
			else if (messCmp(buff, BLOCKED) == 0)
			{
				puts("Your account has been locked\n");
				continue;
			}
		}

		while (1)
		{
			// get password
			printf("Password: ");
			fgets(password, MAX, stdin);
			password[strcspn(password, "\n")] = '\0';

			// send password
			if (0 >= (bytes_sent = sendData(client_sock, password, strlen(password), 0)))
			{
				printf("\nConnection closed!\n");
				return 0;
			}

			// receive
			memset(buff, '\0', MAX);
			if (0 >= (bytes_received = receiveData(client_sock, buff, MAX - 1, 0)))
			{
				printf("\nError!Cannot receive data from sever!\n");
				return 0;
			}
			printf("%s\n", buff);
			buff[bytes_received] = '\0';

			// analyze 
			if (messCmp(buff, FAILED) == 0)
			{ // if pass is wrong < 3 times
				puts("\nPassword is wrong, please input again\n");
				continue;
			}
			else if (messCmp(buff, BLOCKED) == 0)
			{ // if pass is wrong 3 times
				puts("\nPassword is wrong 3rd time, your account has been locked\n");
				return 0;
			}
			else if (messCmp(buff, SUCCESS) == 0)
			{ // if pass is true
				puts("\nLogged in, press any key to log out and exit\n");
				getchar();
				return 0;
			}
			else
			{
				// debug section
				puts(buff);
				puts("Server error\n");
				return 1;
			}
		}
	}
	else if(messCmp(mess, SIGNUP) == 0){
		// username
		while(messCmp(buff, SUCCESS) != 0){
			puts("Please enter username and password");
			printf("Username: ");
			fgets(username, MAX, stdin);
			username[strcspn(username, "\n")] = '\0';
			// send
			if (0 >= (bytes_sent = sendData(client_sock, username, strlen(username), 0)))
			{
				printf("\nConnection closed!\n"); 
				return 0;
			}
			memset(buff, '\0', MAX);
			// receive
			if (0 >= (bytes_received = receiveData(client_sock, buff, MAX - 1, 0)))
			{
				printf("\nError!Cannot receive data from sever!\n");
				return 0;
			}

			buff[bytes_received] = '\0';
			if (messCmp(buff, FAILED) == 0)
			{
				puts("Username exists\n");
				continue;
			}
		}

		while (1)
		{
			// get password
			printf("Password: ");
			fgets(password, MAX, stdin);
			password[strcspn(password, "\n")] = '\0';

			// send password
			if (0 >= (bytes_sent = sendData(client_sock, password, strlen(password), 0)))
			{
				printf("\nConnection closed!\n");
				return 0;
			}

			// receive
			memset(buff, '\0', MAX);
			if (0 >= (bytes_received = receiveData(client_sock, buff, MAX - 1, 0)))
			{
				printf("\nError!Cannot receive data from sever!\n");
				return 0;
			}
			printf("%s\n", buff);
			buff[bytes_received] = '\0';

			// analyze 
			if (messCmp(buff, SUCCESS) == 0)
			{ // if pass is true
				puts("\nSign up complete, press any key to log out and exit\n");
				getchar();
				return 0;
			}
			else
			{
				// debug section
				puts(buff);
				puts("Server error\n");
				return 1;
			}
		}
	}
	// Close socket
	close(client_sock);
	return 0;
}