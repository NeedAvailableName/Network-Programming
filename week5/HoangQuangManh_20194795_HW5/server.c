#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define PORT 5550 /* Port that will be opened */
#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    int listen_sock, conn_sock;
    char recv_data[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int sin_size, isWrong;

    if (argc != 2)
    {
        printf("Usage: %s PortNumber\n", argv[0]);
        return 0;
    }

    int port = atoi(argv[1]);

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("\nError: ");
        return 0;
    }

    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
            perror("\nError: ");

        printf("You got a connection from %s\n", inet_ntoa(client.sin_addr));

        while (1)
        {
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0);
            if (bytes_received <= 0)
            {
                printf("\nConnection closed");
                break;
            }
            recv_data[bytes_received] = '\0';

            if (strcmp(recv_data, "1") == 0)
            {
                bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("\nConnection closed");
                    break;
                }
                recv_data[bytes_received] = '\0';
                printf("Receive from client: %s\n", recv_data);

                char alpha[BUFF_SIZE] = "";
                char digit[BUFF_SIZE] = "";
                int alpha_count = 0, digit_count = 0;
                isWrong = 1;
                for (int i = 0; i < bytes_received - 1; i++)
                {
                    if (isalpha(recv_data[i]))
                    {
                        alpha[alpha_count] = recv_data[i];
                        alpha_count++;
                    }
                    else if (isdigit(recv_data[i]))
                    {
                        digit[digit_count] = recv_data[i];
                        digit_count++;
                    }
                    else
                    {
                        isWrong = 0;
                        if (send(conn_sock, "ERROR", strlen("ERROR"), 0) <= 0)
                        {
                            printf("\nConnection closed");
                            break;
                        }
                        break;
                    }
                }
                if (isWrong == 0)
                    continue;
                char temp[BUFF_SIZE];
                memset(temp, 0, sizeof(temp));
                if (strlen(alpha) > 0)
                {
                    strcat(temp, alpha);
                    strcat(temp, " ");
                }
                if (strlen(digit) > 0)
                {
                    strcat(temp, digit);
                }
                if (send(conn_sock, temp, strlen(temp), 0) <= 0)
                {
                    printf("\nConnection closed");
                    break;
                }
            }
            else
            {
                bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0);
                if (bytes_received <= 0)
                {
                    printf("\nConnection closed");
                    break;
                }
                recv_data[bytes_received] = '\0';
                printf("Receive from client: %s\n", recv_data);

                FILE *fp = fopen(recv_data, "r");
                if (fp == NULL)
                {
                    printf("\nError opening file. Please try again.\n");
                    continue;
                }

                while (fgets(recv_data, sizeof(recv_data), fp))
                {                
                    //strtok(recv_data, "\n");
                    if (send(conn_sock, recv_data, strlen(recv_data), 0) <= 0)
                    {
                        fclose(fp);
                        printf("\nConnection closed");
                        break;
                    }
                }
                fclose(fp);
                if (send(conn_sock, "~", strlen("~"), 0) <= 0)
                {
                    printf("\nConnection closed");
                    break;
                }
            }
        }
    }
    close(listen_sock);
    return 0;
}
