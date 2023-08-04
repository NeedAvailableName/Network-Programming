#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5
#define MAX 1024

// account node
typedef struct node
{
	char username[MAX];
	char password[MAX];
	int status;
	struct node *next;
} node_t;

enum MessageType{
	SIGNIN = 1,
	SIGNUP,
	SUCCESS,
	FAILED,
	BLOCKED
} Mess;

void messageToString(char* reply, int MessageType){
	// printf("%d\n", MessageType);
	reply[0] = '0' + MessageType;
	reply[1] = '\0';
	// printf("%s\n", reply);
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

// load data from text file
node_t *loadData(char *filename)
{
	int status, count = 0; // number of accounts
	FILE *f;
	char username[MAX], password[MAX];
	node_t *head, *current;
	head = current = NULL;

	// open file data
	printf("Loading data from file...\n");
	
	if((f = fopen(filename, "rb")) == NULL)
	{
		printf("Cannot open file %s\n", filename);
		exit(0);
	}

	// load accounts to linked list
	while(fscanf(f, "%s %s %d\n", username, password, &status) != EOF)
	{
		// create new node
		node_t *node = malloc(sizeof(node_t));
		strcpy(node->username, username);
		strcpy(node->password, password);
		node->status = status;

		// add node to list
		if (head == NULL)
			current = head = node;
		else
			current = current->next = node;
		count++;
	}
	
	fclose(f);
	printf("Loaded %d Account(s)\n", count);
	return head;
}

// find a node exist in linked list given username
node_t *findNode(node_t *head, char *username)
{
	node_t *current = head;
	while(current != NULL)
	{
		if (0 == strcmp(current->username, username))
			return current;
		current = current->next;
	}
	return NULL;
}

node_t *insertNode(node_t *head, char *username, char *password){
	node_t *current = head;
	node_t *node = malloc(sizeof(node_t));
	int status = 1;
	strcpy(node->username, username);
	strcpy(node->password, password);
	node->status = status;
	while(current->next != NULL)
	{
		current = current->next;
	}
	current->next = node;
	return current;
}

// save list to text file
void saveList(node_t *head, char *filename)
{
	FILE *f;
	f = fopen(filename, "w");
	node_t *current;
	for (current = head; current; current = current->next)
		fprintf(f, "%s %s %d\n", current->username, current->password, current->status);
	fclose(f);
}

int main(int argc, char const *argv[])
{
	char filename[] = "account.txt";

	// valid number of argument
	if (argc != 2)
	{
		printf("Format: ./server PortNumber\n\n");
		return 0;
	}

	int i, i_max;
	int listen_fd, conn_fd, sock_fd, max_fd;
	int n_ready, clients[FD_SETSIZE];
	ssize_t ret;
	fd_set read_fds, allset;
	socklen_t client_len;
	struct sockaddr_in server;
	struct sockaddr_in client;

	char username[MAX], password[MAX], receive[MAX], *reply;
	int bytes_sent, bytes_received;
	node_t *found;

	// load file txt to linked list
	node_t *account_list = loadData(filename);

	// Construct a TCP socket to listen connection request
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("\nError: ");
		return 0;
	}
	
	// Bind address to socket
	memset(&server, '\0', sizeof server);
	server.sin_family = AF_INET;         
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listen_fd, (struct sockaddr*)&server, sizeof(server))==-1){
		perror("\nError: ");
		return 0;
	}     
	
	// Listen request from client
	if (listen (listen_fd, BACKLOG) == -1){
		perror("\nError: ");
		return 0;
	}
	
	max_fd = listen_fd; /* initialize */
	i_max = -1;		  /* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		clients[i] = -1; /* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listen_fd, &allset);

	puts("Server up and running...\n");
	// Communicate with client
	while(1) 
	{
		read_fds = allset; /* structure assignment */
		n_ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		if (n_ready < 0)
		{
			perror("\nError: ");
			return 0;
		}
		
		if (FD_ISSET(listen_fd, &read_fds))
		{ /* new client connection */
			// accept request
			client_len = sizeof(client);
			if ((conn_fd = accept(listen_fd,( struct sockaddr *)&client, &client_len)) == -1) 
				perror("\nError: ");
	
			printf("\nYou got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
			for (i = 0; i < FD_SETSIZE; i++){
				if (clients[i] < 0)
				{
					clients[i] = conn_fd; /* save descriptor */
					break;
				}
			}
			if (i == FD_SETSIZE)
			{
				printf("\nToo many clients");
				close(conn_fd);
			}
			reply = malloc(sizeof(char) * 2);
			FD_SET(conn_fd, &allset); /* add new descriptor to set */
			if (conn_fd > max_fd)
				max_fd = conn_fd; /* for select */
			if (i > i_max)
				i_max = i; /* max index in client[] array */

			if (--n_ready <= 0)
				continue; /* no more readable descriptors */

		}

		for (i = 0; i <= i_max; i++) 
		{
			// receive username
			if ((sock_fd = clients[i]) < 0)
				continue;
			if (FD_ISSET(sock_fd, &read_fds))
			{
				while (messCmp(reply, SIGNIN) != 0 && messCmp(reply, SIGNUP) != 0){
					if ((ret = receiveData(conn_fd, reply, 2, 0)) <= 0) {
						printf("\nConnection closed\n");
						break;
					}
					reply[ret] = '\0';
					printf("%s\n", reply);
				}
				//if sign in
				if(messCmp(reply, SIGNIN) == 0){
					messageToString(reply, SIGNIN);
					if (0 >= (bytes_sent = sendData(conn_fd, reply, strlen(reply), 0))) {
						printf("\nConnection closed\n");
						break;
					}
					// printf("%s\n", reply);
					while (messCmp(reply, SUCCESS) != 0){
						if ((ret = receiveData(conn_fd, username, MAX - 1, 0)) <= 0) {
							printf("\nConnection closed\n");
							break;
						}
						printf("i: %d, client: %d\n", i, clients[i]);
						username[ret] = '\0';
						// check username existence
						if ((found = findNode(account_list, username))) {
							if (found->status == 1){
								messageToString(reply, SUCCESS); // username found
							}
							else{
								messageToString(reply, BLOCKED); //username blocked
							}
						}
						else{
							messageToString(reply, FAILED); //username not found
						}

						// echo to client
						if (0 >= (bytes_sent = send(conn_fd, reply, strlen(reply), 0))) {
							printf("\nConnection closed\n");
							break;
						}
					}
					int count = 0; // count password

					reply = malloc(sizeof(char) * 2);
					while (1) {
						// receive password
						memset(password, '\0', MAX);
						if (0 >= (ret = receiveData(conn_fd, password, MAX - 1, 0))) {
							printf("\nConnection closed\n");
							break;
						}
						password[ret] = '\0';

						// validate password
						if (0 == strcmp(found->password, password)){
							messageToString(reply, SUCCESS);
						}
						else {
							count++;
							if (count == 3) {
								messageToString(reply, BLOCKED); // wrong pass 3 times
								found->status = 0; // lock account
							}
							else{
								messageToString(reply, FAILED); // wrong pass < 3 times
							}
						}
						printf("%s\n", reply);
						if (0 >= (bytes_sent = sendData(conn_fd, reply, strlen(reply), 0))) {
							printf("\nConnection closed\n");
							break;
						}
					}
						
				}
				else if(messCmp(reply, SIGNUP) == 0){
					messageToString(reply, SIGNUP);
					if (0 >= (bytes_sent = sendData(conn_fd, reply, strlen(reply), 0))) {
						printf("\nConnection closed\n");
						break;
					}
					// printf("%s\n", reply);
					while (messCmp(reply, SUCCESS) != 0){
						if ((ret = receiveData(conn_fd, username, MAX - 1, 0)) <= 0) {
							printf("\nConnection closed\n");
							break;
						}
						printf("i: %d, client: %d\n", i, clients[i]);
						username[ret] = '\0';
						// check username existence
						if ((found = findNode(account_list, username))) {
							messageToString(reply, FAILED); // username found
						}
						else{
							messageToString(reply, SUCCESS); //username not found
						}

						// echo to client
						if (0 >= (bytes_sent = send(conn_fd, reply, strlen(reply), 0))) {
							printf("\nConnection closed\n");
							break;
						}
					}
					int count = 0; // count password

					// receive password
					while(1){
						reply = malloc(sizeof(char) * 2);
						memset(password, '\0', MAX);
						if (0 >= (ret = receiveData(conn_fd, password, MAX - 1, 0))) {
							printf("\nConnection closed\n");
							break;
						}
						password[ret] = '\0';
						messageToString(reply, SUCCESS);
						insertNode(account_list, username, password);
						if (0 >= (bytes_sent = sendData(conn_fd, reply, strlen(reply), 0))) {
							printf("\nConnection closed\n");
							break;
						}
					}
				}


				saveList(account_list, filename);

				close(conn_fd);
				clients[i] = -1;
				FD_CLR(sock_fd, &allset);
			}

			if (--n_ready <= 0)
				break; /* no more readable descriptors */
			// else { 
			// // on parent process
			// close(conn_fd);
			// }
		}
	}
	close(listen_fd);
	return 0;
}

