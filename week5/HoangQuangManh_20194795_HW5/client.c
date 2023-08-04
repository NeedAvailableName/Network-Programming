#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5550
#define BUFF_SIZE 8192

int main(int argc, char *argv[])
{
    int client_sock;
    char buff[BUFF_SIZE];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately! ");
        return 0;
    }

    // Step 4: Communicate with server
    while (1)
    {
        printf("\nMENU\n");
        printf("1. Send any string\n");
        printf("2. Send the contents of a file\n");
        printf("Enter your choice: ");

        int choice;
        scanf("%d", &choice);
        getchar();

        sprintf(buff, "%d", choice);
        bytes_sent = send(client_sock, buff, sizeof(choice), 0);

        if (choice == 1)
        {
            printf("\nEnter a string to send: ");
            fgets(buff, BUFF_SIZE, stdin);
            msg_len = strlen(buff);
            if (msg_len == 0)
                break;

            bytes_sent = send(client_sock, buff, msg_len, 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
                break;
            }

            bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
            if (bytes_received <= 0)
            {
                printf("\nError! Cannot receive data from server!\n");
                break;
            }
            buff[bytes_received] = '\0';
            printf("%s \n", buff);
        }
        else if (choice == 2)
        {
            printf("\nEnter the path of the file to send: ");
            memset(buff, '\0', (strlen(buff) + 1));
            fgets(buff, BUFF_SIZE, stdin);
            strtok(buff, "\n");

            bytes_sent = send(client_sock, buff, strlen(buff), 0);
            if (bytes_sent <= 0)
            {
                printf("\nConnection closed!\n");
                break;
            }

            while (1)
            {                
                char *result;
                bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("\nError! Cannot receive data from server!\n");
                    break;
                }
                buff[bytes_received] = '\0';
                result = strchr(buff, '~');
                if (result !=NULL)
                {
                    buff[bytes_received-1]='\0';
                    printf("%s",buff);
                    break;
                }else {
                    printf("%s",buff);
                }             
            }
        }
        else
        {
            printf("\nInvalid choice. Please try again.\n");
        }
    }

    // Step 4: Close socket
    close(client_sock);
    return 0;
}

void check(char string[])
{
}