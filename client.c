#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_SIZE 1024

int is_valid_address(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

void decode_password(char *password, char *digit_string, char *alpha_string) {
    int index = 0;
    int j = 0;
    for (int i = 0; i < strlen(password); i++) {
        if (password[i] == '-') {
            index = i;
            break;
        }
    }

    for (int i = 0; i < index; i++) {
        digit_string[i] = password[i];
    }

    digit_string[index] = '\0';

    for (int i = index + 1; i < strlen(password); i++) {
        alpha_string[j++] = password[i];
    }

    alpha_string[j] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Invalid parameters\n");
        return -1;
    }

    char ip_address[50];
    strcpy(ip_address, argv[1]);
    if (is_valid_address(ip_address) == 0 || is_valid_address((ip_address)) == -1) {
        printf("Invalid IP address!");
        return -1;
    }

    if (atoi(argv[2]) < 0 || atoi(argv[2]) > 65536) {
        printf("Invalid port number");
        return -1;
    }

    int client_sock;
    struct sockaddr_in server_addr;
    socklen_t addr_size;
    char received_message[1024];
    char send_message[1024];
    int n;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    printf("TCP Socket created!\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Connected to server!\n");

    char username[MAX_SIZE];
    char password[MAX_SIZE];

    bzero(send_message, 1024);
    strcpy(send_message, "Hello from client");
    send(client_sock, send_message, strlen(send_message), 0);

    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s: ", received_message);
    
    while(1) {
        while(1) {
            fgets(username, MAX_SIZE, stdin);
            fflush(stdin);

            bzero(send_message, 1024);
            strcpy(send_message, username);
            send(client_sock, send_message, strlen(send_message), 0);
            
            bzero(received_message, 1024);
            recv(client_sock, received_message, MAX_SIZE, 0);
            if (strlen(received_message) < 30) {
                printf("%s: ", received_message);
                break;
            } else {
                printf("%s: ", received_message);
            }
        }

        fgets(password, MAX_SIZE, stdin);
        fflush(stdin);

        bzero(send_message, 1024);
        strcpy(send_message, password);
        send(client_sock, send_message, strlen(send_message), 0);

        bzero(received_message, 1024);
        recv(client_sock, received_message, MAX_SIZE, 0);
        if (strcmp(received_message, "OK") == 0) {
            printf("%s\n", received_message);
            break;
        } else {
            printf("%s: ", received_message);
        }
    }    
    char digit_string[MAX_SIZE];
    char alpha_string[MAX_SIZE]; 
    //change password part
    while(1) {
        fgets(password, MAX_SIZE, stdin);
        fflush(stdin);

        bzero(send_message, 1024);
        strcpy(send_message, password);
        send(client_sock, send_message, strlen(send_message), 0);

        bzero(received_message, 1024);
        recv(client_sock, received_message, MAX_SIZE, 0);
        if (strcmp(received_message, "Goodbye") == 0) {
            printf("Goodbye %s\n", username);
            break;
        } else if (strcmp(received_message, "Invalid") == 0) {
            printf("Error\n");
        } else if (strcmp(received_message, "Same") == 0) {
            printf("Old password and new password is the same\n");
        } else {
            decode_password(received_message, digit_string, alpha_string);
            if (strcmp(digit_string, "###") != 0) {
                printf("%s\n", digit_string);
            }
            if (strcmp(alpha_string, "###") != 0) {
                printf("%s\n", alpha_string);
            }
        }

    }

    close(client_sock);
    printf("Close port, disconect to server!\n");

    return 0;
}