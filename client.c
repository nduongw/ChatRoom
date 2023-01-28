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
#include <sys/ioctl.h>

#define NUM_CHUNK 20

#define MAX_SIZE 1024
volatile sig_atomic_t flag = 0;

int client_sock;
struct sockaddr_in server_addr;
socklen_t addr_size;
char received_message[1024];
char send_message[1024];
char message[1024];
char name[1024];
int n;
int is_login = 0;
int check = 0;

char *chunk_buffer;
int full_chunk_size, offset;
int chunk_count = 0;
char file_ext[10];
char file_name[50];
FILE *file;

int is_valid_address(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result;
}

void split_buffer(char *buffer_out, char *message, char *name) {
    int count_n = 0;
    int count_m = 0;
    for (int i = 0; i < strlen(buffer_out); i++) {
        if (buffer_out[i] != '@') {
            message[count_m++] = buffer_out[i];
        } else {
            break;
        }
    }

    message[count_m] = '\0';

    for (int i = count_m + 1; i < strlen(buffer_out); i++) {
        name[count_n++] = buffer_out[i];
    }

    name[count_n] = '\0';
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
    
}

void recv_msg_handler() {
    while (1) {
        memset(received_message, 0, MAX_SIZE);
        int receive = recv(client_sock, received_message, MAX_SIZE, 0);
        printf("Message: %s\n", received_message);
        if (strcmp(received_message, "recvfile") == 0) {
            FILE *fptr;
            fptr = fopen("client_cat.jpeg", "w");
            if (fptr == NULL) {
                printf("Cant open file to write\n");
                return NULL;
            }
            int total = 0;
            int count = 0;
            int file_length;
            
            recv(client_sock, message, MAX_SIZE, 0);
            file_length = atoi(message);
            printf("File length: %d\n", file_length);
            full_chunk_size = file_length / NUM_CHUNK;
            offset = file_length % NUM_CHUNK;

            while(1) {
                bzero(message, MAX_SIZE);
                n = recv(client_sock, message, MAX_SIZE, 0);
                if (strcmp(message, "done") == 0) {
                    break;
                }
                if (n <= 0) {
                    break;
                }

                if (count < NUM_CHUNK ) {
                    fwrite(message, 1, full_chunk_size, fptr);
                    total += full_chunk_size;
                } else if (count >= NUM_CHUNK && offset != 0) {
                    fwrite(message, 1, offset, fptr);
                    total += offset;
                }

                count++;
                printf("Total: %d\n", total);
            }
            printf("\nGet file done\n");
            printf("Total: %d\n", total);
            fclose(fptr);
        } else {
            for (int i = 0; i < strlen(received_message); i++) {
            if (received_message[i] == '@') {
                check = 1;
                break;
            }
            }
            if (check) {
                split_buffer(received_message, message, send_message);
            } else {
                strcpy(message, received_message);
            }
            if (strcmp(message, "Logined") == 0) {
                printf("User has logined!\n");
                is_login = 1;
                strcpy(name, send_message);
            }
            printf("%s\n", message);
        }
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

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf("Error: pthread\n");
        return -1;
    }
    char message[MAX_SIZE];
    char buffer[MAX_SIZE];

    while(1){
        if(flag) {
            printf("Bye\n");
            break;
        }

        while(1) {
            fgets(message, MAX_SIZE, stdin);
            str_trim_lf(message, MAX_SIZE);

            if (strcmp(message, "exit") == 0) {
                break;
            } else if (strlen(message) > 1 && strcmp(message, "quit") == 0) {
                is_login = 0;
                check = 0;
                send(client_sock, message, strlen(message), 0);
            } else if (strlen(message) > 1 && strcmp(message, "out") == 0) {
                check = 0;
                send(client_sock, message, strlen(message), 0);
            } else if (strlen(message) == 1){
                send(client_sock, message, strlen(message), 0);
            } else if (strcmp(message, "sendfile") == 0) {
                FILE *fptr;
                fptr = fopen("cat.jpeg", "r");
                if (fptr == NULL) {
                    printf("Cant open file to read\n");
                    break;
                } else {
                    printf("Opened file!\n");
                }

                send(client_sock, message, sizeof(message), 0);
                size_t pos = ftell(fptr);
                fseek(fptr, 0, SEEK_END);
                size_t file_length = ftell(fptr);
                fseek(fptr, pos, SEEK_SET);

                full_chunk_size = file_length / NUM_CHUNK;
                offset = file_length % NUM_CHUNK;
                printf("file_length: %d - full chunk size: %d - offset: %d\n", file_length, full_chunk_size, offset);
                sprintf(message, "%d", file_length);
                send(client_sock, message, sizeof(message), 0);
                int count = 0;
                int total = 0;
                int count2;
                while(1) {
                    printf("Count: %d\n", count);

                    bzero(message, MAX_SIZE);
                    if (count < NUM_CHUNK) {
                        count2 = fread(message, 1, full_chunk_size, fptr);
                        total += count2;
                    } else if(count >= NUM_CHUNK && offset != 0) {
                        count2 = fread(message, 1, offset, fptr);
                        printf("Message length - offset: %d\n", strlen(message));
                        total += count2;
                        offset = 0;
                    } else {
                        strcpy(message, "done");
                        send(client_sock, message, sizeof(message), 0);
                        break;
                    }

                    if (send(client_sock, message, sizeof(message), 0) == -1) {
                        printf("Fail to send file\n");
                        break;
                    }
                    printf("Message length: %d - Total: %d\n", strlen(message), total);

                    count++;
                }
                printf("Send file to server done!\n");
                fclose(fptr);
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

    close(client_sock);
    return 0;
}