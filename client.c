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
#include <ctype.h>

#define MAX_SIZE 1024
#define NUM_CHUNK 20

void *recv_msg_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char message[1024];
    char flag[10];
    char server_file[200];
    int file_length;
    char client_name[100];

    while(1) {
        bzero(message, 1024);
        int n = recv(client_sock, message, 1024, 0);

        sscanf(message, "%s %s %s %d", flag, client_name, server_file, &file_length);
        if (strcmp(flag, "RECV") == 0) {
            FILE *fptr;
            char client_file[500];
            
            sprintf(client_file, "%s_%s", client_name, server_file);

            fptr = fopen(client_file, "w");
            if (fptr == NULL) {
                printf("Cant open file to write\n");
                return NULL;
            }
            
            int full_chunk_size = file_length / NUM_CHUNK;
            int offset = file_length % NUM_CHUNK;
            int count = 0;

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
                } else if (count >= NUM_CHUNK && offset != 0) {
                    fwrite(message, 1, offset, fptr);
                }

                count++;
            }
            printf("\nGet file done\n");
            fclose(fptr);
        } else {
            printf("%s\n", message);
        }
    }
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

int count_space(char *buffer) {
    int count = 0;
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ' ') {
            count += 1;
        }
    }

    return count;
}

int main(int argc, char *argv[]) {
    int client_sock;
    struct sockaddr_in server_addr;
    socklen_t addr_size;
    char message[1024];

    if (argc != 3) {
        printf("Invalid parameters\n");
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

    int n = connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0) {
        printf("Connect failed\n");
        return -1;
    }

    printf("Connected to server!\n");

    pthread_t pid;
    int *arg = (int *)calloc(1, sizeof(int));
    *arg = client_sock;
    if (pthread_create(&pid, NULL, recv_msg_handler, arg) != 0) {
        printf("Error: pthread\n");
        return -1;
    }

    while(1) {
        fgets(message, 1024, stdin);
        str_trim_lf(message, 1024);
        fflush(stdin);

        if (strncmp(message, "OUT", 3) == 0) {
            send(client_sock, message, strlen(message), 0);
            break;
        } else if (strncmp(message, "FILE", 4) == 0) {
            char file_name[100];
            int count = count_space(message);

            if (count == 2) {
                sscanf(message, "%*s %*s %s", file_name);
            } else if (count == 3) {
                sscanf(message, "%*s %*s %*s %s", file_name);
            }

            FILE *fptr;
            fptr = fopen(file_name, "r");
            if (fptr == NULL) {
                printf("Cant open file to read\n");
                break;
            }
            send(client_sock, message, sizeof(message), 0);

            size_t pos = ftell(fptr);
            fseek(fptr, 0, SEEK_END);
            size_t file_length = ftell(fptr);
            fseek(fptr, pos, SEEK_SET);

            int full_chunk_size = file_length / NUM_CHUNK;
            int offset = file_length % NUM_CHUNK;
            sprintf(message, "%d", (int)file_length);
            send(client_sock, message, sizeof(message), 0);
            count = 0;
            int count2;
            while(1) {
                bzero(message, MAX_SIZE);
                if (count < NUM_CHUNK) {
                    fread(message, 1, full_chunk_size, fptr);
                } else if(count >= NUM_CHUNK && offset != 0) {
                    fread(message, 1, offset, fptr);
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
                count++;
            }
            fclose(fptr);
            printf("Sent!\n");
        } else {
            send(client_sock, message, strlen(message), 0);
        }

        bzero(message, 1024);
    }

    close(client_sock);
    return 0;
}