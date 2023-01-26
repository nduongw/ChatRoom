#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_SIZE 1024
volatile sig_atomic_t flag = 0;

int client_sock;
struct sockaddr_in server_addr;
socklen_t addr_size;
char received_message[1024];
char send_message[1024];
char name[1024];
int n;
int is_login = 0;

int is_valid_address(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

void handle_login(char username[], char password[]) {
    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s", received_message);

    fgets(username, MAX_SIZE, stdin);
    fflush(stdin);
    bzero(send_message, 1024);
    strcpy(send_message, username);
    send(client_sock, send_message, strlen(send_message), 0);

    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s", received_message);

    fgets(password, MAX_SIZE, stdin);
    fflush(stdin);
    bzero(send_message, 1024);
    strcpy(send_message, password);
    send(client_sock, send_message, strlen(send_message), 0);

    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s\n", received_message);
    strcpy(name, received_message);

    if (strlen(received_message) > 2) {
        is_login = 1;
    }

    bzero(send_message, 1024);
    strcpy(send_message, "Done\n");
    send(client_sock, send_message, strlen(send_message), 0);
}

void handle_register(char username[], char password[], char name[]) {
    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s", received_message);

    fgets(username, MAX_SIZE, stdin);
    fflush(stdin);
    bzero(send_message, 1024);
    strcpy(send_message, username);
    send(client_sock, send_message, strlen(send_message), 0);

    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s", received_message);

    fgets(password, MAX_SIZE, stdin);
    fflush(stdin);
    bzero(send_message, 1024);
    strcpy(send_message, password);
    send(client_sock, send_message, strlen(send_message), 0);

    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s", received_message);

    fgets(name, MAX_SIZE, stdin);
    fflush(stdin);
    bzero(send_message, 1024);
    strcpy(send_message, name);
    send(client_sock, send_message, strlen(send_message), 0);

    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s\n", received_message);

    bzero(send_message, 1024);
    strcpy(send_message, "Done\n");
    send(client_sock, send_message, strlen(send_message), 0);
}

void handle_invalid_input() {
    bzero(received_message, 1024);
    recv(client_sock, received_message, MAX_SIZE, 0);
    printf("%s: ", received_message);

    bzero(send_message, 1024);
    strcpy(send_message, "Done\n");
    send(client_sock, send_message, strlen(send_message), 0);
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void send_msg_handler() {
    char message[MAX_SIZE];
	char buffer[MAX_SIZE];

    while(1) {
        fgets(message, MAX_SIZE, stdin);
        str_trim_lf(message, MAX_SIZE);

        if (strcmp(message, "exit") == 0) {
            break;
        } else if (strcmp(message, "quit") == 0 || strlen(message) == 1 || strcmp(message, "out") == 0) {
            is_login = 0;
            send(client_sock, message, strlen(message), 0);
        } else {
            if (is_login) {
                sprintf(buffer, "%s: %s\n", name, message);
                send(client_sock, buffer, strlen(buffer), 0);
            } else {
                send(client_sock, message, strlen(message), 0);
            }
        }

        bzero(message, MAX_SIZE);
        bzero(buffer, MAX_SIZE);
    }
    catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
    while (1) {
        memset(received_message, 0, MAX_SIZE);
        int receive = recv(client_sock, received_message, MAX_SIZE, 0);
        printf("%s\n", received_message);

    }

    return;
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
    char name[MAX_SIZE];
    char choice[MAX_SIZE];

    bzero(send_message, 1024);
    strcpy(send_message, "Hello from client");
    send(client_sock, send_message, strlen(send_message), 0);
    
    while(1) {
        bzero(received_message, 1024);
        recv(client_sock, received_message, MAX_SIZE, 0);
        printf("%s: ", received_message);

        fgets(choice, MAX_SIZE, stdin);
        fflush(stdin);

        bzero(send_message, 1024);
        strcpy(send_message, choice);
        send(client_sock, send_message, strlen(send_message), 0);
        
        if (atoi(send_message) == 1) {
            handle_login(username, password);
        } else if (atoi(send_message) == 3) {
            handle_register(username, password, name);
        } else {
            handle_invalid_input();
        }

        if (is_login) {
            printf("Login done\n");
            break;
        }
    }

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf("Error: pthread\n");
        return -1;
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf("Error: pthread\n");
        return -1;
    }

    while(1){
        if(flag) {
            printf("Bye\n");
            break;
        }
    }

    close(client_sock);
    return 0;
}