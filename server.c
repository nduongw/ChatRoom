#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <sqlite3.h> 
#include <signal.h>
#include "utils.c"

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

list_node *bad_words_list = NULL;

pthread_t tid;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char buffer_out[MAX_SIZE];

int full_chunk_size, offset;
char file_ext[10]; // file extension of the input file
char message[MAX_SIZE];

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
        client_info->first_join = 1;
        client_info->is_online = 1;
        client_info->is_inchat = 0;
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
			if((clients[i]->uid != uid) && clients[i]->is_online && clients[i]->is_inchat) {
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
			if((clients[i]->uid != uid) && (clients[i]->uid == to_uid) && clients[i]->is_online && clients[i]->is_inchat) {
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
                send(clients[i]->sockfd, message, strlen(message), 0);
				break;
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void send_file_to_client(FILE *fptr, char *server_file, int file_length, int full_chunk_size, int offset, int uid) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if((clients[i]->uid != uid) && clients[i]->is_online && clients[i]->is_inchat) {
                fptr = fopen(server_file, "r");
                if (fptr == NULL) {
                    printf("Cant open file to read\n");
                    return;
                }

                char flag[20] = "recvfile";
                int offset_copy = offset;
                bzero(message, MAX_SIZE);
                sprintf(message, "%s %s %d", flag, server_file, file_length);
                send(clients[i]->sockfd, message, strlen(message), 0);
                
                int count = 0;

                while(1) {
                    printf("Count: %d\n", count);

                    bzero(message, MAX_SIZE);
                    if (count < NUM_CHUNK) {
                        fread(message, 1, full_chunk_size, fptr);
                    } else if(count >= NUM_CHUNK && offset_copy != 0) {
                        fread(message, 1, offset_copy, fptr);
                        offset_copy = 0;
                    } else {
                        strcpy(message, "done");
                        send(clients[i]->sockfd, message, sizeof(message), 0);
                        break;
                    }

                    if (send(clients[i]->sockfd, message, sizeof(message), 0) == -1) {
                        printf("Fail to send file\n");
                        break;
                    }

                    count++;
                }
                printf("Send to %s done!\n", clients[i]->name);
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
        char name[100];
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
        client_info->first_join = 1;
        client_info->is_online = 1;
        client_info->is_inchat = 0;
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

int get_online_user() {
    int total_online = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] && clients[i]->is_online) {
            total_online++;
        }
    }
    return total_online;
}

void send_message_to_group(char *message, int uid, int group_uid[], int length) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
            for (int j = 0; j < length; j++) {
                if(clients[i]->uid == group_uid[j]) {
                    if(write(clients[i]->sockfd, message, strlen(message)) < 0){
                        perror("ERROR: write to descriptor failed");
                        break;
                    }
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
    char name[100];
    char option[MAX_SIZE];
    int leave_flag = 0;
    int logout_flag = 0;
    int is_chat = 0;
    int out_flag = 0;
    FILE *fptr;

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
        
        if (out_flag == 0) {
            bzero(buffer_out, MAX_SIZE);
            sprintf(buffer_out, "%s has online\n", client_info->name);
            printf("%s", buffer_out);
            printf("%s user id: %d\n", client_info->name, client_info->uid);
            send_message_to_all(buffer_out, client_info->uid);
        }
        
        while(1) {
            bzero(buffer_out, 1024);
            strcpy(buffer_out, "----Chat room----\n1.Create new group chat\n2.Block user\n3.Go to chat\n4.Online user\nYour choice: ");
            send_message_to_client(buffer_out, client_info->uid);
            bzero(buffer_out, MAX_SIZE);

            int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);

            if (strcmp(buffer_out, "quit") == 0) {
                sprintf(buffer_out, "%s has offline\n", client_info->name);
                printf("%s", buffer_out);
                send_message_to_all(buffer_out, client_info->uid);
                out_flag = 1;
                logout_flag = 1;
                is_login = 0;
                first_join = 1;
                client_info->is_online = 0;
                client_info->is_inchat = 0;
                break;
            }

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
                case 4:
                    // int total_online = get_online_user();
                    // bzero(buffer_out, MAX_SIZE);
                    // sprintf(buffer_out, "%d", total_online);
                    // send_message_to_client(buffer_out, client_info->uid);
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

            pthread_mutex_lock(&clients_mutex);
            if (client_info->first_join) {
                strcpy(buffer_out, "Welcome to Chat room\n");
                client_info->is_inchat = 1;
                client_info->first_join = 0;
            }
            pthread_mutex_unlock(&clients_mutex);

            send_message_to_client(buffer_out, client_info->uid);

            bzero(buffer_out, MAX_SIZE);
            int receive = recv(client_info->sockfd, buffer_out, MAX_SIZE, 0);
            printf("Buffer out: %s\n", buffer_out);

            if (receive > 0) {
                if (strlen(buffer_out) > 0 && strcmp(buffer_out, "quit") == 0) {
                    sprintf(buffer_out, "%s has offline\n", client_info->name);
                    printf("%s", buffer_out);
                    send_message_to_all(buffer_out, client_info->uid);
                    out_flag = 1;
                    logout_flag = 1;
                    is_login = 0;
                    first_join = 1;
                    client_info->is_online = 0;
                    client_info->is_inchat = 0;
                    break;
                } else if (strlen(buffer_out) > 0 && strcmp(buffer_out, "out") == 0) {
                    out_flag = 1;
                    client_info->first_join = 1;
                    client_info->is_inchat = 0;
                    break;
                } else if (strcmp(buffer_out, "sendfile") == 0) {
                    bzero(message, MAX_SIZE);
                    recv(client_info->sockfd, message, MAX_SIZE, 0);
                    printf("Client send: %s\n", message);

                    char server_file[2048];

                    sprintf(server_file, "server_%s", message);
                    
                    fptr = fopen(server_file, "w");
                    if (fptr == NULL) {
                        printf("Cant open file to write\n");
                        return NULL;
                    }
                    int total = 0;
                    int count = 0;
                    int file_length;

                    bzero(message, MAX_SIZE);
                    recv(client_info->sockfd, message, MAX_SIZE, 0);
                    file_length = atoi(message);
                    full_chunk_size = file_length / NUM_CHUNK;
                    offset = file_length % NUM_CHUNK;

                    while(1) {
                        bzero(message, MAX_SIZE);
                        n = recv(client_info->sockfd, message, MAX_SIZE, 0);
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
                    fclose(fptr);
                    printf("Get file!\n");

                    send_file_to_client(fptr, server_file, file_length, full_chunk_size, offset, client_info->uid);
                } else if (strlen(buffer_out) > 0) {
                    char option_name[100];
                    char option_message[1000];
                    printf("Buffer out: %s", buffer_out);
                    split_buffer(buffer_out, option, message, name);
                    printf("Name: %s - Option: %s - Message: %s\n",name, option, message);  
                    if (strcmp(option, "one") == 0 || strcmp(option, "group") == 0) {
                        get_sending_option(message, option_message, option_name);
                        bzero(message, MAX_SIZE);
                        strcpy(message, option_message);
                        printf("Want to send to %s with message: %s\n", option_name, option_message);
                    }
                    filter_message(bad_words_list, message);
                    char new_message[2048];
                    sprintf(new_message, "%s : %s", name, message);
                    printf("%s\n", new_message);
                    if (strcmp(option, "all") == 0) {
                        send_message_to_all(new_message, client_info->uid);
                    } else if (strcmp(option, "one") == 0) {
                        // int uid = get_user_id_by_name(db, stmt, option_name);
                        // printf("User id: %d\n", uid);
                        // send_message_to_one(new_message, client_info->uid, uid);
                    } else if (strcmp(option, "group") == 0) {
                        int group_uid[20];
                        int length;
                        int group_id = atoi(option_name);
                        get_group_members_id(db, stmt, group_id, group_uid, &length);
                        // for (int i = 0; i < length; i++) {
                        //     printf("Member id: %d\t", group_uid[i]);
                        // }
                        send_message_to_group(new_message, client_info->uid, group_uid, length);
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
    FILE *file;

    //check number of parameters
    if (argc != 2) {
        printf("Invalid parameters");
        return -1;
    }

    if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65536) {
        printf("Invalid port number\n");
        return -1;
    }

    file = fopen("demo_bad_words.txt", "r");
    if (!file) {
        printf("Cant open file to read\n");
        return -1;
    }

    int first = 1;
    char word[100];
    while (1) {
        fgets(word, 100, file);
        word[strlen(word) - 1] = '\0';

        if (first) {
            bad_words_list = list_create(word);
            first = 0;
        }
        
        list_node *new_node = list_insert_end(bad_words_list, word);
        
        if (feof(file)) {
            break;
        }
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

    while(1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
        printf("Client connected! %d\n", client_sock);

        pthread_create(&tid, NULL, &handle_client, NULL);
    }

    sqlite3_finalize(stmt);
	sqlite3_close(db);
    close(server_sock);

    return 0;
}