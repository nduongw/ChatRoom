#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include "utils.c"
#include <sqlite3.h> 
#include <signal.h>

sqlite3 *db;
sqlite3_stmt *stmt;
static int client_count = 0;

int server_sock, client_sock;
struct sockaddr_in server_addr, client_addr;
socklen_t addr_size;
char sent_message[1024];
char received_message[1024];
int n, is_login = 0;
pthread_t tid;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


int handle_login(char username[], char password[], client_t *client_info) {
    char message[MAX_SIZE] = "Input your account: ";
    send_message(client_sock, sent_message, message);

    receive_message(client_sock, received_message);
    strcpy(username, received_message);

    strcpy(message, "Input your password: ");
    send_message(client_sock, sent_message, message);

    receive_message(client_sock, received_message);
    strcpy(password, received_message);

    int check = check_account_exist(db, stmt, username, password);

    if (!check) {
        strcpy(message, "Your account does not exist or wrong password");
        send_message(client_sock, sent_message, message);
        receive_message(client_sock, received_message);

        return 0;
    } else {
        char name[MAX_SIZE];
        int user_id;
        get_user_info(db, stmt, username, password, name, &user_id);
        printf("User information: %s - id: %d\n", name, user_id);

        strcpy(message, name);
        send_message(client_sock, sent_message, message);
        receive_message(client_sock, received_message);

        client_info->address = client_addr;
        client_info->sockfd = client_sock;
        client_info->uid = user_id;
        strcpy(client_info->name, name);

        queue_add(clients_mutex, clients, client_info);
        traverse_queue(clients);

        return 1;
    }
}

void handle_logout() {
    printf("Handle logout\n");
}

void handle_register(char username[], char password[], char name[]) {
    char message[MAX_SIZE] = "Input new account: ";
    send_message(client_sock, sent_message, message);
    receive_message(client_sock, received_message);
    strcpy(username, received_message);

    strcpy(message, "Input new password: ");
    send_message(client_sock, sent_message, message);
    receive_message(client_sock, received_message);
    strcpy(password, received_message);

    strcpy(message, "Input name: ");
    send_message(client_sock, sent_message, message);
    receive_message(client_sock, received_message);
    strcpy(name, received_message);

    int check = register_new_account(db, stmt, username, password, name);

    if (check) {
        strcpy(message, "Register successful");
        send_message(client_sock, sent_message, message);
        receive_message(client_sock, received_message);
    } else {
        strcpy(message, "Can not register this account");
        send_message(client_sock, sent_message, message);
        receive_message(client_sock, received_message);
    }
}

void send_message_to_all(char *message, int uid) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(clients[i]->uid != uid) {
				if(write(clients[i]->sockfd, message, strlen(message)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client() {
    char choice_str[MAX_SIZE];
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char name[MAX_SIZE];
    int leave_flag = 0;

    bzero(received_message, 1024);
    recv(client_sock, received_message, sizeof(received_message), 0);
    printf("%s\n", received_message);

    client_t *client_info = (client_t *)malloc(sizeof(client_t));

    while(1) {
        char message[MAX_SIZE] = "Chatting App\n1.Login\n2.Logout\n3.Register\nYour choice: ";
        send_message(client_sock, sent_message, message);

        bzero(received_message, 1024);
        receive_message(client_sock, received_message);
        strcpy(choice_str, received_message);

        int choice = atoi(choice_str);

        switch(choice) {
            case 1:
                is_login = handle_login(username, password, client_info);
                break;
            case 2:
                leave_flag = 1;
                break;
            case 3:
                handle_register(username, password, name);
                break;
            default:
                strcpy(message, "Invalid choice\n");
                send_message(client_sock, sent_message, message);
                receive_message(client_sock, received_message);
                break;
        }

        if (is_login) {
            printf("Client has logined!\n");
            break;
        }

        if (leave_flag == 1) {
            printf("Bye\n");
            close(client_info->sockfd);
            queue_remove(clients_mutex, clients, client_info->uid);
            free(client_info);
            pthread_detach(pthread_self());
            return NULL;
        }
    }
    
    char buffer_out[MAX_SIZE];
    bzero(buffer_out, MAX_SIZE);

    sprintf(buffer_out, "%s has online\n", client_info->name);
    printf("%s", buffer_out);
    send_message_to_all(buffer_out, client_info->uid);

    while(1) {
        if (leave_flag == 1) {
            break;
        }

        bzero(buffer_out, MAX_SIZE);
        int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);
        if (receive > 0) {
            if (strlen(buffer_out) > 0) {
                send_message_to_all(buffer_out, client_info->uid);
                printf("%s", buffer_out);
            }
        } else if (receive == 0 || strcmp(buffer_out, "exit") == 0){
			sprintf(buffer_out, "%s has left\n", client_info->name);
			printf("%s", buffer_out);
			send_message_to_all(buffer_out, client_info->uid);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

    }

    close(client_info->sockfd);
    queue_remove(clients_mutex, clients, client_info->uid);
    free(client_info);
    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char *argv[]) {
    char *server_ip = "127.0.0.1";
    int option = 1;

    //check number of parameters
    if (argc != 2) {
        printf("Invalid parameters");
        return -1;
    }

    if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65536) {
        printf("Invalid port number\n");
        return -1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket error");
        exit(1);
    }
    printf("TCP Socket created!\n");

    sqlite3_open("chat_room.db", &db);

	if (db == NULL) {
		printf("Failed to open DB\n");
		return 0;
	}

    printf("Read database successful\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(server_sock, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
        return EXIT_FAILURE;
	}

    n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0) {
        perror("Bind error");
        exit(1);
    }

    printf("Bind to port number : %d\n", atoi(argv[1]));

    listen(server_sock, 10);
    printf("Listening...\n\n");

    int sin_size = sizeof(client_addr);

    char choice_str[MAX_SIZE];

    int is_signin = 0;
    pid_t child_pid;

    while(1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
        printf("Client connected!\n");

        pthread_create(&tid, NULL, &handle_client, NULL);
    }

    sqlite3_finalize(stmt);

	sqlite3_close(db);

    return 0;
}