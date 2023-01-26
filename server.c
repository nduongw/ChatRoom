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
int first_join = 1;

pthread_t tid;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char buffer_out[MAX_SIZE];


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
        // printf("User information: %s - id: %d\n", name, user_id);

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

void split_buffer(char *buffer_out, char *option, char *message, char *name) {
    int count_o = 0;
    int count_n = 0;
    int count_m = 0;
    
    for (int i = 0; i < strlen(buffer_out); i++) {
        if (buffer_out[i] != ':') {
            name[count_n++] = buffer_out[i];
        } else {
            break;
        }
    }

    name[count_n] = '\0';

    for (int i = count_n + 2; i < strlen(buffer_out); i++) {
        if (buffer_out[i] != '&') {
            option[count_o++] = buffer_out[i];
        } else {
            break;
        }
    }

    option[count_o] = '\0';

    for (int i = count_n + count_o + 3; i < strlen(buffer_out); i++) {
        message[count_m++] = buffer_out[i];
    }

    message[count_m] = '\0';
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

void send_message_to_one(char *message, int uid, int to_uid) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if((clients[i]->uid != uid) && (clients[i]->uid == to_uid)) {
				if(write(clients[i]->sockfd, message, strlen(message)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void send_message_to_client(char *message, int uid) {
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(clients[i]->uid == uid) {
                printf("Client name: %s - client id: %d\n", clients[i]->name, clients[i]->uid);
                send(clients[i]->sockfd, message, strlen(message), 0);
				break;
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

int handle_login_1(char username[], char password[], client_t *client_info) {
    bzero(buffer_out, 1024);
    strcpy(buffer_out, "Input your account: ");
    send_message_to_client(buffer_out, client_info->uid);

    bzero(buffer_out, MAX_SIZE);
    int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);

    strcpy(username, buffer_out);

    bzero(buffer_out, 1024);
    strcpy(buffer_out, "Input your password: ");
    send_message_to_client(buffer_out, client_info->uid);

    bzero(buffer_out, MAX_SIZE);
    receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);
    strcpy(password, buffer_out);

    int check = check_account_exist(db, stmt, username, password);

    if (!check) {
        bzero(buffer_out, 1024);
        strcpy(buffer_out, "Your account does not exist or wrong password\n");
        send_message_to_client(buffer_out, client_info->uid);

        return 0;
    } else {
        char name[MAX_SIZE];
        int user_id;
        get_user_info(db, stmt, username, password, name, &user_id);
        printf("Name: %s - User id: %d\n", name, user_id);

        bzero(buffer_out, 1024);
        sprintf(buffer_out, "Logined@%s", name);
        printf("Buffer out: %s\n", buffer_out);

        queue_remove(clients_mutex, clients, client_info->uid);

        client_info->address = client_addr;
        client_info->sockfd = client_sock;
        client_info->uid = user_id;
        strcpy(client_info->name, name);

        queue_add(clients_mutex, clients, client_info);
        send_message_to_client(buffer_out, client_info->uid);

        return 1;
    }
}

void create_group_chat() {
    return;
}

void block_friend() {
    return;
}

// void send_message_to_group(char *message, int uid) {
//     pthread_mutex_lock(&clients_mutex);

//     for(int i = 0;i < MAX_CLIENTS; i++) {
// 		if(clients[i]) {
// 			if(clients[i]->uid != uid) {
// 				if(write(clients[i]->sockfd, message, strlen(message)) < 0){
// 					perror("ERROR: write to descriptor failed");
// 					break;
// 				}
// 			}
// 		}
// 	}

//     pthread_mutex_unlock(&clients_mutex);
// }

void *handle_client() {
    char choice_str[MAX_SIZE];
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char name[MAX_SIZE];
    char option[MAX_SIZE], message[MAX_SIZE];
    int leave_flag = 0;
    int logout_flag = 0;
    int is_chat = 0;
    int out_flag = 0;

    bzero(received_message, 1024);
    recv(client_sock, received_message, sizeof(received_message), 0);
    printf("%s\n", received_message);

    client_t *client_info = (client_t *)malloc(sizeof(client_t));

    while(1) {
        bzero(message, 1024);
        strcpy(message, "Chatting App\n1.Login\n2.Logout\n3.Register\n4.Exit\nYour choice: ");
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
                logout_flag = 1;
                break;
            case 3:
                handle_register(username, password, name);
                break;
            case 4:
                leave_flag = 1;
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
            break;
        }
    }

    while(1) {
        if (leave_flag == 1) {
            break;
        }
        
        bzero(buffer_out, MAX_SIZE);

        sprintf(buffer_out, "%s has online\n", client_info->name);
        printf("%s", buffer_out);
        printf("%s user id: %d\n", client_info->name, client_info->uid);
        send_message_to_all(buffer_out, client_info->uid);

        while(1) {
            bzero(buffer_out, 1024);
            strcpy(buffer_out, "----Chat room----\n1.Create new group chat\n2.Block user\n3.Go to chat\nYour choice: ");
            send_message_to_client(buffer_out, client_info->uid);

            bzero(buffer_out, MAX_SIZE);
            int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);

            int choice = atoi(buffer_out);

            switch(choice) {
                case 1:
                    create_group_chat();
                    break;
                case 2:
                    block_friend();
                    break;
                case 3:
                    is_chat = 1;
                    out_flag = 0;
                    break;
                default:
                    strcpy(buffer_out, "Invalid choice\n");
                    send_message_to_client(buffer_out, client_info->uid);
                    break;
            }

            if (is_chat){
                printf("Go to chat\n");
                break;
            }

            if (leave_flag == 1) {
                // printf("Bye\n");
                // close(client_info->sockfd);
                // queue_remove(clients_mutex, clients, client_info->uid);
                // free(client_info);
                // pthread_detach(pthread_self());
                // return NULL;
                break;
            }
        }

        while(1) {
            if (logout_flag == 1 || leave_flag == 1 || out_flag == 1) {
                break;
            }
            bzero(buffer_out, MAX_SIZE);
            if (first_join) {
                strcpy(buffer_out, "Welcome to Chat room\n");
                first_join = 0;
            }

            printf("First join: %d\n", first_join);
            send_message_to_client(buffer_out, client_info->uid);

            bzero(buffer_out, MAX_SIZE);
            int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);

            if (receive > 0) {
                if (strlen(buffer_out) > 0 && strcmp(buffer_out, "quit") == 0) {
                    sprintf(buffer_out, "%s has offline\n", client_info->name);
                    printf("%s", buffer_out);
                    send_message_to_all(buffer_out, client_info->uid);
                    logout_flag = 1;
                    is_login = 0;
                    first_join = 1;
                    break;
                } else if (strlen(buffer_out) > 0 && strcmp(buffer_out, "out") == 0) {
                    out_flag = 1;
                    first_join = 1;
                    break;
                } else if (strlen(buffer_out) > 0) {
                    printf("Buffer out: %s", buffer_out);
                    split_buffer(buffer_out, option, message, name);
                    printf("Name: %s - Option: %s - Message: %s\n",name, option, message);
                    char new_message[MAX_SIZE];
                    sprintf(new_message, "%s : %s", name, message);
                    printf("%s\n", new_message);
                    if (strcmp(option, "all") == 0) {
                        send_message_to_all(new_message, client_info->uid);
                    } else if (strcmp(option, "one") == 0) {
                        send_message_to_one(new_message, client_info->uid, 1);
                    }
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

        while(1) {
            if (is_login == 1) {
                break;
            }
            
            bzero(buffer_out, 1024);
            strcpy(buffer_out, "Chatting App\n1.Login\n2.Logout\n3.Register\nYour choice: ");
            send_message_to_client(buffer_out, client_info->uid);

            bzero(buffer_out, MAX_SIZE);
            int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);

            int choice = atoi(buffer_out);

            switch(choice) {
                case 1:
                    is_login = handle_login_1(username, password, client_info);
                    break;
                case 2:
                    logout_flag = 1;
                    break;
                case 3:
                    handle_register(username, password, name);
                    break;
                case 4:
                    leave_flag = 1;
                    break;
                default:
                    strcpy(buffer_out, "Invalid choice\n");
                    send_message_to_client(buffer_out, client_info->uid);
                    break;
            }

            if (is_login) {
                printf("Client has logined!\n");
                logout_flag = 0;
                break;
            }

            if (leave_flag == 1) {
                // printf("Bye\n");
                // close(client_info->sockfd);
                // queue_remove(clients_mutex, clients, client_info->uid);
                // free(client_info);
                // pthread_detach(pthread_self());
                // return NULL;
                break;
            }
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