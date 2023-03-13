#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <sqlite3.h> 
#include "utils.c"


client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
list_node *bad_words_list = NULL;

sqlite3 *db;
sqlite3_stmt *stmt;

void handle_login(int client_sock, int *login) {
    char username[1024];
    char password[1024];
    char message[1024];

    send_message(client_sock, message, "Input username: ");
    receive_message(client_sock, username);
    send_message(client_sock, message, "Input password: ");
    receive_message(client_sock, password);

    client_t *client_info = (client_t *)malloc(sizeof(client_t));
    
    printf("Username: %s - password: %s\n", username, password);

    int check = check_account_exist(db, stmt, username, password);

    if (!check) {
        send_message(client_sock, message, "Your account does not exist or wrong password");
        return;
    } else {
        *login = 1;
        char name[MAX_SIZE];
        int user_id;
        get_user_info(db, stmt, username, password, name, &user_id);
        printf("User information: %s - id: %d\n", name, user_id);

        strcpy(message, name);
        send_message(client_sock, message, "Login successful");

        client_info->sockfd = client_sock;
        client_info->uid = user_id;
        client_info->first_join = 1;
        client_info->is_online = 1;
        client_info->is_inchat = 0;
        strcpy(client_info->name, name);

        queue_add(clients_mutex, clients, client_info);
        traverse_queue(clients);

        return;
    }

    return;
}

void handle_logout(int client_sock, int *login) {
    char message[MAX_SIZE];
    if (*login == 1) {
        queue_remove(clients_mutex, clients, client_sock);
        send_message(client_sock, message, "Log out successful");
        *login = 0;
    } else {
        send_message(client_sock, message, "You haven't logined yet");
    }
}

void handle_register(int client_sock) {
    char username[1024];
    char password[1024];
    char name[1024];
    char message[1024];

    send_message(client_sock, message, "Input username: ");
    receive_message(client_sock, username);
    send_message(client_sock, message, "Input password: ");
    receive_message(client_sock, password);
    send_message(client_sock, message, "Input display name: ");
    receive_message(client_sock, name);

    int check = register_new_account(db, stmt, username, password, name);

    if (check) {
        send_message(client_sock, message, "Register successful");
    } else {
        send_message(client_sock, message, "Can not register this account");
    }
}

void handle_online_user(int client_sock) {
    char message[MAX_SIZE];
    char client_name[MAX_SIZE];
    bzero(client_name, MAX_SIZE);
    int count = 0;

    for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
            if (clients[i]->sockfd != client_sock) {
                if (count == 0) {
                    strcat(client_name, clients[i]->name);
                } else {
                    strcat(client_name, " - ");
                    strcat(client_name, clients[i]->name);
                }                
                count++;
                printf("Clients name: %s\n", client_name);
            }
			
		}
	}
    strcat(client_name, "\n");
    send_message(client_sock, message, client_name);
}

void handle_block(int client_sock) {
    char name[MAX_SIZE] = {0};
    char message[MAX_SIZE] = {0};
    int block_uid;
    int uid;

    send_message(client_sock, message, "Input username to block: ");
    receive_message(client_sock, name);
    get_user_id(db, stmt, name, &block_uid);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i]) {
			if(clients[i]->sockfd == client_sock) {
                uid = clients[i]->uid;
            }
        }
    }

    int check = add_to_block_table(db, stmt, uid, block_uid);

    send_message(client_sock, message, "Block done!");
}

void handle_unblock(int client_sock) {
    char name[MAX_SIZE] = {0};
    char message[MAX_SIZE] = {0};
    int block_uid;
    int uid;

    send_message(client_sock, message, "Input username to unblock: ");
    receive_message(client_sock, name);
    printf("Input name: %s\n", name);
    get_user_id(db, stmt, name, &block_uid);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i]) {
			if(clients[i]->sockfd == client_sock) {
                uid = clients[i]->uid;
            }
        }
    }
    printf("Uid : %d - Block uid: %d\n", uid, block_uid);
    int check = delete_from_block(db, stmt, uid, block_uid);

    send_message(client_sock, message, "Unblock done!");
}

void handle_txt_message_one(int client_sock, char *buffer) {
    char name[100];
    char message[900];
    char send_message[MAX_SIZE];
    int length;
    int block_uids[50];
    int uid;


    sscanf(buffer, "%s %[^\n]s", name, message);
    filter_message(bad_words_list, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->sockfd == client_sock) {
                uid = clients[i]->uid;
                sprintf(send_message, "%s : %s", clients[i]->name, message);
            }
        }
    }

    get_block_uids(db, stmt, uid, block_uids, &length);

    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(clients[i]->sockfd != client_sock) {
                if ((strcmp(clients[i]->name, name) == 0) && clients[i]->is_inchat) {
                    for (int j = 0; j < length; j++) {
                        if (clients[i]->uid != block_uids[j]) {
                            if(send(clients[i]->sockfd, send_message, strlen(send_message), 0) < 0){
                                perror("ERROR: write to descriptor failed");
                                break;
                            }       
                        }
                    }
                }
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void handle_file_message_one(int client_sock, char *buffer, char *server_file, int file_length, int full_chunk_size, int offset) {
    char name[100];
    char message[MAX_SIZE];
    FILE *fptr;

    sscanf(buffer, "%s %[^\n]s", name, message);

    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(clients[i]->sockfd != client_sock) {
                if ((strcmp(clients[i]->name, name) == 0) && (clients[i]->is_inchat)) {
                    fptr = fopen(server_file, "r");
                    if (fptr == NULL) {
                        printf("Cant open file to read\n");
                        return;
                    }

                    char flag[20] = "RECV";
                    int offset_copy = offset;
                    bzero(message, MAX_SIZE);
                    sprintf(message, "%s %s %s %d", flag, clients[i]->name,server_file, file_length);
                    send(clients[i]->sockfd, message, strlen(message), 0);
                    
                    int count = 0;

                    while(1) {
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
                        printf("Message: %s\n", message);
                        if (send(clients[i]->sockfd, message, sizeof(message), 0) == -1) {
                            printf("Fail to send file\n");
                            break;
                        }

                        count++;
                    }
                    printf("Send to %s done!\n", clients[i]->name);
                    fclose(fptr);
                }
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void handle_file_message_all(int client_sock, char *buffer, char *server_file, int file_length, int full_chunk_size, int offset) {
    char name[100];
    char message[MAX_SIZE];
    FILE *fptr;

    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if((clients[i]->sockfd != client_sock) && clients[i]->is_inchat) {
                fptr = fopen(server_file, "r");
                if (fptr == NULL) {
                    printf("Cant open file to read\n");
                    return;
                }

                char flag[20] = "RECV";
                int offset_copy = offset;
                bzero(message, MAX_SIZE);
                sprintf(message, "%s %s %s %d", flag, clients[i]->name,server_file, file_length);
                send(clients[i]->sockfd, message, strlen(message), 0);
                
                int count = 0;

                while(1) {
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
                    printf("Message: %s\n", message);
                    if (send(clients[i]->sockfd, message, sizeof(message), 0) == -1) {
                        printf("Fail to send file\n");
                        break;
                    }

                    count++;
                }
                fclose(fptr);
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void handle_txt_message_all(int client_sock, char *buffer) {
    char name[100];
    char message[900];
    char send_message[MAX_SIZE];
    int uid, length;
    int block_uids[50];

    strcpy(message, buffer);
    filter_message(bad_words_list, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->sockfd == client_sock) {
                uid = clients[i]->uid;
                sprintf(send_message, "%s : %s", clients[i]->name, message);
            }
        }
    }

    get_block_uids(db, stmt, uid, block_uids, &length);

    for (int i = 0; i < length; i++) {
        printf("Block user id: %d\n", block_uids[i]);
    }

    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if((clients[i]->sockfd != client_sock) && clients[i]->is_inchat) {
                if (length) {
                    for (int j = 0; j < length; j++) {
                        if (clients[i]->uid != block_uids[j]) {
                            if(send(clients[i]->sockfd, send_message, strlen(send_message), 0) < 0){
                                perror("ERROR: write to descriptor failed");
                                break;
                            }       
                        }
                    }
                } else {
                    if(send(clients[i]->sockfd, send_message, strlen(send_message), 0) < 0){
                        perror("ERROR: write to descriptor failed");
                        break;
                    }   
                }
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void handle_txt_message_group(int client_sock, char *buffer) {
    char group_name[100], name[100];
    int group_id;
    int group_uid[50];
    int block_uids[50];
    int length, blength, uid;
    char message[900];
    char send_message[MAX_SIZE];

    sscanf(buffer, "%d %[^\n]s", &group_id, message);
    get_group_members_id(db, stmt, group_id, group_uid, &length);
    get_block_uids(db, stmt, uid, block_uids, &blength);
    filter_message(bad_words_list, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i]) {
            if (clients[i]->sockfd == client_sock) {
                uid = clients[i]->uid;
                sprintf(send_message, "%s : %s", clients[i]->name, message);
            }
        }
    }

    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(clients[i]->sockfd != client_sock) {
                for (int j = 0; j < length; j++) {
                    if (clients[i]->uid == group_uid[j]) {
                        for (int k = 0; k < blength; k++) {
                            if ((clients[i]->uid != block_uids[k]) && clients[i]->is_inchat) {
                                if(send(clients[i]->sockfd, send_message, strlen(send_message), 0) < 0){
                                    perror("ERROR: write to descriptor failed");
                                    break;
                                }       
                            }
                        }
                    }
                }
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void handle_file_message_group(int client_sock, char *buffer, char *server_file, int file_length, int full_chunk_size, int offset) {
    char group_name[100], name[100];
    int group_id;
    int group_uid[50];
    int length;
    char message[MAX_SIZE];
    FILE *fptr;

    sscanf(buffer, "%d %[^\n]s", &group_id, message);
    get_group_members_id(db, stmt, group_id, group_uid, &length);

    pthread_mutex_lock(&clients_mutex);

    for(int i = 0;i < MAX_CLIENTS; i++) {
		if(clients[i]) {
			if(clients[i]->sockfd != client_sock) {
                for (int j = 0; j < length; j++) {
                    if ((clients[i]->uid == group_uid[j]) && clients[i]->is_inchat) {
                        fptr = fopen(server_file, "r");
                        if (fptr == NULL) {
                            printf("Cant open file to read\n");
                            return;
                        }

                        char flag[20] = "RECV";
                        int offset_copy = offset;
                        bzero(message, MAX_SIZE);
                        sprintf(message, "%s %s %s %d", flag, clients[i]->name,server_file, file_length);
                        send(clients[i]->sockfd, message, strlen(message), 0);
                        
                        int count = 0;

                        while(1) {
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
                            printf("Message: %s\n", message);
                            if (send(clients[i]->sockfd, message, sizeof(message), 0) == -1) {
                                printf("Fail to send file\n");
                                break;
                            }

                            count++;
                        }
                        fclose(fptr);
                    }
                }
			}
		}
	}

    pthread_mutex_unlock(&clients_mutex);
}

void handle_chat(int client_sock) {
    char flag1[5];
    char flag2[4];
    char message[MAX_SIZE] = {0};
    char buffer[MAX_SIZE] = {0};

    send_message(client_sock, message, "----Chat room----");
    while(1) {
        receive_message(client_sock, message);
        printf("Message: %s\n", message);
        sscanf(message, "%s %s %[^\n]s", flag1, flag2, buffer);
        printf("Flag1:%s - Flag2:%s - Buffer:%s\n", flag1, flag2, buffer);
        if (strncmp(flag1, "TEXT", 4) == 0) {
            if (strncmp(flag2, "ONE", 3) == 0) {
                handle_txt_message_one(client_sock, buffer);
            } else if(strncmp(flag2, "ALL", 3) == 0) {
                handle_txt_message_all(client_sock, buffer);
            } else if(strncmp(flag2, "GRP", 3) == 0) {
                handle_txt_message_group(client_sock, buffer);
            }
        } else if(strncmp(flag1, "FILE", 4) == 0) {
            char server_file[200];
            char file_name[100];
            int count = count_space(message);

            printf("Count: %d\n", count);
            if (count == 2) {
                sscanf(message, "%*s %*s %s", file_name);
            } else if (count == 3) {
                sscanf(message, "%*s %*s %*s %s", file_name);
            }

            sprintf(server_file, "server_%s", file_name);
            
            FILE *fptr;
            fptr = fopen(server_file, "w");
            if (fptr == NULL) {
                printf("Cant open file to write\n");
                return ;
            }
            int total = 0;
            count = 0;
            int file_length;

            bzero(message, MAX_SIZE);
            recv(client_sock, message, MAX_SIZE, 0);
            file_length = atoi(message);
            printf("File length: %d\n", file_length);
            int full_chunk_size = file_length / NUM_CHUNK;
            int offset = file_length % NUM_CHUNK;
            // int total = 0;

            while(1) {
                bzero(message, MAX_SIZE);
                int n = recv(client_sock, message, MAX_SIZE, 0);
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

            if (strncmp(flag2, "ONE", 3) == 0) {
                handle_file_message_one(client_sock, buffer, server_file, file_length, full_chunk_size, offset);
            } else if(strncmp(flag2, "ALL", 3) == 0) {
                handle_file_message_all(client_sock, buffer, server_file, file_length, full_chunk_size, offset);
            } else if(strncmp(flag2, "GRP", 3) == 0) {
                handle_file_message_group(client_sock, buffer, server_file, file_length, full_chunk_size, offset);
            }
        } else if(strncmp(flag1, "QUIT", 4) == 0) {
            change_inchat_status(clients, client_sock);
            break;
        }

        bzero(message, MAX_SIZE);
        bzero(buffer, MAX_SIZE);
    }
}

void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    printf("Client sock: %d\n", client_sock);
    free(arg);
    char buffer[MAX_SIZE] = {0};
    int login = 0;


    while(1) {
        if (login == 0) {
            bzero(buffer, MAX_SIZE);
            send_message(client_sock, buffer, "Chatting App\n1.Login\n2.Register\n3.Exit\nYour choice: ");
            int r = recv(client_sock, buffer, sizeof(buffer), 0);
            if (r > 0) {
                if (strncmp(buffer, "1", 1) == 0) {
                    handle_login(client_sock, &login);
                } else if (strncmp(buffer, "2", 1) == 0) {
                    handle_register(client_sock);
                } else if (strncmp(buffer, "3", 1) == 0) {
                    return NULL;
                }
            }
        } else {
            bzero(buffer, MAX_SIZE);
            send_message(client_sock, buffer, "Chatting App\n1.Block user\n2.Unblock user\n3.Go to chat\n4.Show online users\n5.Logout\nYour choice: ");
            int r = recv(client_sock, buffer, sizeof(buffer), 0);
            if (r > 0) {
                if (strncmp(buffer, "1", 1) == 0) {
                    handle_block(client_sock);
                } else if (strncmp(buffer, "2", 1) == 0) {
                    handle_unblock(client_sock);
                } else if (strncmp(buffer, "3", 1) == 0) {
                    change_inchat_status(clients, client_sock);
                    handle_chat(client_sock);
                } else if (strncmp(buffer, "4", 1) == 0) {
                    handle_online_user(client_sock);
                } else if (strncmp(buffer, "5", 1) == 0) {
                    handle_logout(client_sock, &login);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    char *server_ip = "127.0.0.1";
    int n, option = 1;
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

    file = fopen("vn_bad_words.txt", "r");
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

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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
        pthread_t pid;
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = client_sock;
        pthread_create(&pid, NULL, handle_client, arg);
    }

    close(server_sock);
    return 0;
}