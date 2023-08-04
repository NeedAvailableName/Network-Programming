#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/wait.h>
#include <errno.h>

#define BACKLOG 20
#define BUFF_SIZE 1024

typedef struct account {
    char username[50];
    char password[50];
    short status;
} *account;

typedef struct Node {
    account account;
    struct Node* next;
} Node;

Node *head = NULL;  //define linked list
char preSection[50]; //section controll
int signInCounter = 0;
Node *Section = NULL;


Node* makeNode(char u[], char p[], short s) {
    
    Node* newNode = (Node*)malloc(sizeof(Node));
    account acc = (account)calloc(100,sizeof(account));

    strcpy(acc->username,u);
    strcpy(acc->password,p);

    newNode->account= acc;
    newNode->account->status = s;
    newNode->next = NULL;
    return newNode;
}


void writeFile(){ // Write file into text file
    FILE *fptr;
    Node*ptr =head;
 
    fptr = fopen("account.txt", "w");
    if(fptr == NULL) {
        printf("Error!");
        exit(1);
    }
    
    while(ptr!= NULL) {
        fprintf(fptr, "%s %s %d\n", ptr->account->username, ptr->account->password, ptr->account->status);
        ptr = ptr->next;
    }
    fclose(fptr);
}

void insertAccount(char u[], char p[], short s) { //insert an account to the top of linked list
    Node *tmp = makeNode(u,p,s);
    if(head == NULL) {
        head = tmp;
    }
    else {
        tmp->next = head;
        head = tmp;
    }
}

void readFile(){ // read the text file
    char un[50];
    char pw[50];
    char st[2];
    int status;
    FILE *fptr;
    char k;
    int i = 0;
	head = NULL;
    if((fptr = fopen("account.txt", "r")) == NULL) {
        printf("Error! opening file");
        exit(1);         
    }
    while(fscanf(fptr, "%s %s %s", un, pw, st) != EOF) {
        status = atoi(st); //convert to integer
        //insert readed line into linked list
        insertAccount(un, pw, status);
    };
}

Node* findNode(char u[], char p[]) {// find a node using username and password
    if(head == NULL) return NULL;

    Node* ptr = head;
    
    while(ptr != NULL) {
        if(strcmp(ptr->account->username, u) == 0 && strcmp(ptr->account->password, p) == 0) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}
Node* findAccountName(char u[]) { //finds a node using name only
    if(head == NULL) return NULL;
    Node* ptr = head;
    
    while(ptr != NULL) {
        if(strcmp(ptr->account->username, u) == 0) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}
char preUser[50];
int inputCount = 0;
int SignIn(char msg[], char sent_msg[]) {
    char username[50];
    char password[50];
    int i = 0;
	if(Section != NULL) {
		strcpy(sent_msg,"You have to logout first\n");
		return 5;
	}
    readFile();
    while(msg[i] != ' ') {
        username[i] = msg[i];
        i++;
    }
    username[i] ='\0';
    strncpy(password, msg+i+1, strlen(msg) - i);
    Node* acc = findAccountName(username);
    if(acc == NULL) {
        strcpy(sent_msg, "Your account is not registed\n");
        return 0;
    }
    else {
        Node *acc2 = findNode(username, password);
        if(acc2 == NULL) {
			if(strcmp(username, preUser) == 0) {
				inputCount += 1;
			}
			if(inputCount >= 2) {
				strcpy(sent_msg,"Wrong password 3 times. Your account is blocked.\n");
				acc->account->status = 0;
				writeFile();
				return 4;
			}
			strcpy(sent_msg, "Wrong password\n");
			strcpy(preUser, username);
            return 1;
        }
        else {
            if(acc2->account->status == 0) {
                strcpy(sent_msg,"Account is not activated!\n");
				inputCount=0;
                return 2;
            }
            Section = acc2;
			char c[50];
			strcpy(c, "Hello ");
			strcat(c, acc2->account->username);
			inputCount = 0;
            strcpy(sent_msg, c);
        }
    }

    return 3;
}

/* Handler process signal*/
void sig_chld(int signo);

/*
* Receive and echo message to client
* [IN] sockfd: socket descriptor that connects to client 	
*/
void echo(int sockfd);

int main(int argc, char* argv[]) {
	
	int server_sock, client_sock;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	pid_t pid;
	socklen_t sin_size;
	
	if(argc != 2) {
		printf("ERROR\n");
		exit(1);
	}

	int PORT = atoi(argv[1]);

	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {  /* calls socket() */
		printf("Error\n");
		return 0;
	}
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(server_sock, (struct sockaddr*)&server, sizeof(server)) == -1){ 
		perror("\nError: ");
		return 0;
	}     

	if(listen(server_sock, BACKLOG) == -1) {  
		perror("\nError: ");
		return 0;
	}
	
	/* Establish a signal handler to catch SIGCHLD */
	signal(SIGCHLD, sig_chld);

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if((client_sock = accept(server_sock, (struct sockaddr *)&client, &sin_size)) == -1){
			if(errno == EINTR)
				continue;
			else {
				perror("\nError: ");			
				return 0;
			}
		}
		
		/* For each client, fork spawns a child, and the child handles the new client */
		pid = fork();
		
		/* fork() is called in child process */
		if(pid  == 0) {
			close(server_sock);
			printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
			echo(client_sock);					
			exit(0);
		}
		
		/* The parent closes the connected socket since the child handles the new client */
		close(client_sock);
	}
	close(server_sock);
	return 0;
}

void sig_chld(int signo) {
	pid_t pid;
	int stat;
	
	/* Wait the child process terminate */
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("\nChild %d terminated\n",pid);
}

void echo(int sockfd) {
	char buff[BUFF_SIZE];
	char sent_msg[50];
	int bytes_sent, bytes_received;
	
	while(1) {
			//receives username and password from client
			bytes_received = recv(sockfd, buff, BUFF_SIZE-1, 0); //blocking
			if (bytes_received <= 0){
				printf("\nConnection closed");
				break;
			}
			else {
				buff[bytes_received] = '\0';
				printf("\nReceive: %s ", buff);
                SignIn(buff,sent_msg);
			}

			//echo to client
			bytes_sent = send(sockfd, sent_msg, 50, 0); /* send to the client welcome message */
			if(bytes_sent <= 0){
				printf("\nConnection closed");
				break;
			}
		}//end conversation
		close(sockfd);		
}