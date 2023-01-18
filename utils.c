#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <strings.h>
#include "utils.h"

int check_account_exist(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[]) {
    char *sql = "SELECT * FROM users WHERE account = ? AND password = ?";
    int check = sqlite3_prepare(db, sql, -1, &stmt, 0);
    if (check == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, strlen(username), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password, strlen(password), SQLITE_STATIC);
    } else {
        printf("Failed to prepare statement\n");
        return -1;
    }

    int step = sqlite3_step(stmt);
    if (step == SQLITE_ROW) {
        return 1;
    } else {
        return 0;
    }
}

void get_user_info(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[], int *id) {
    printf("Username: %s - password: %s\n", username, password);

    char *sql = "SELECT id, username FROM users WHERE account = ? AND password = ?";
    int check = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (check == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, strlen(username), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password, strlen(password), SQLITE_STATIC);
    } else {
        printf("Fail to prepare statement\n");
        return;
    }

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        int num_cols = sqlite3_column_count(stmt);

        for (int i = 0; i < num_cols; i++) {
			switch (sqlite3_column_type(stmt, i)) {
			case (SQLITE3_TEXT):
				strcpy(name, sqlite3_column_text(stmt, i));
				break;
			case (SQLITE_INTEGER):
				*id = sqlite3_column_int(stmt, i);
				break;
			default:
				break;
			}
        }
    }

    return;
}

void get_user_info_by_id(sqlite3 *db, sqlite3_stmt *stmt, int user_id, char *name) {
	char *sql = "SELECT username FROM users WHERE id = ?";
    int check = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (check == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
    } else {
        printf("Fail to prepare statement\n");
        return;
    }

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        int num_cols = sqlite3_column_count(stmt);

        for (int i = 0; i < num_cols; i++) {
			switch (sqlite3_column_type(stmt, i)) {
			case (SQLITE3_TEXT):
				strcpy(name, sqlite3_column_text(stmt, i));
				break;
			default:
				break;
			}
        }
    }

    return;
}

int get_user_id_by_name(sqlite3 *db, sqlite3_stmt *stmt, char *name) {
	char *sql = "SELECT id FROM users WHERE username = ?";
    int check = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (check == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
    } else {
        printf("Fail to prepare statement\n");
        return -1;
    }

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        int num_cols = sqlite3_column_count(stmt);

        for (int i = 0; i < num_cols; i++) {
			switch (sqlite3_column_type(stmt, i)) {
			case (SQLITE_INTEGER):
				int user_id = sqlite3_column_int(stmt, i);
				return user_id;
				break;
			default:
				break;
			}
        }
    }
}

int find_last_id(sqlite3 *db, sqlite3_stmt *stmt) {
	char *sql = "SELECT id FROM users ORDER BY id DESC";
    int check = sqlite3_prepare(db, sql, -1, &stmt, NULL);

    while(sqlite3_step(stmt) != SQLITE_DONE) {
        int num_cols = sqlite3_column_count(stmt);

        for (int i = 0; i < num_cols; i++) {
			// printf("%d\n", sqlite3_column_int(stmt, i));
			return sqlite3_column_int(stmt, i);
            // strcpy(name, sqlite3_column_text(stmt, i));
        }
    }

    return 0;
}

int register_new_account(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[]) {
	int new_id = find_last_id(db, stmt);
	int check = check_account_exist(db, stmt, username, password);
	if (check == 1) {
		printf("Account has existed\n");
		return 0;
	}
    char *sql = "INSERT INTO users (id, account, password, username, last_logined) VALUES (?, ?, ?, ?, '2023-11-11')";
    check = sqlite3_prepare(db, sql, -1, &stmt, 0);
    if (check == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, new_id + 1);
        sqlite3_bind_text(stmt, 2, username, strlen(username), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, password, strlen(password), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, name, strlen(name), SQLITE_STATIC);
    } else {
        printf("Failed to prepare statement\n");
        return -1;
    }

    int step = sqlite3_step(stmt);
    if (step == SQLITE_DONE) {
		printf("Register successful\n");
        return 1;
    } else {
        return 0;
    }
}

void create_new_group(sqlite3 *db, sqlite3_stmt *stmt) {
    char *sql = "INSERT INTO groups VALUES (1, 1, group_chat1)";
    int check = sqlite3_prepare(db, sql, -1, &stmt, 0);

    int step = sqlite3_step(stmt);
    if (step == SQLITE_DONE) {
		printf("Register successful\n");
        return ;
    } else {
		printf("Create group fail\n");
        return ;
    }
}

void get_group_members_id(sqlite3 *db, sqlite3_stmt *stmt, int group_id, int member_ids[], int *length) {
	char *sql = "SELECT user_id FROM groups_members WHERE id = ?";
    int check = sqlite3_prepare(db, sql, -1, &stmt, 0);
    if (check == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, group_id);
    } else {
        printf("Failed to prepare statement\n");
        return;
    }

	int count = 0;
    while(sqlite3_step(stmt) != SQLITE_DONE) {
        int num_cols = sqlite3_column_count(stmt);

        for (int i = 0; i < num_cols; i++) {
			member_ids[count++] = sqlite3_column_int(stmt, i);
        }
    }

	*length = count;
}


void send_message(int client_sock, char *packet, char *messege) {
    printf("Sending messege: %s\n", messege);
    bzero(packet, 1024);
    strcpy(packet, messege);
    send(client_sock, packet, strlen(packet), 0);
}

void receive_message(int client_sock, char *received_message) {
    bzero(received_message, 1024);
    int check = recv(client_sock, received_message, MAX_SIZE, 0);
    received_message[strlen(received_message) - 1] = '\0';
    printf("Received messege: %s\n", received_message);
}

void encode_password(char *password, char *digit_string, char *alpha_string) {
    char number[100];
    char character[100];
    int count_n = 0;
    int count_c = 0;

    for (int i = 0; i < strlen(password); i++) {
        if (isdigit(password[i])) {
            number[count_n++] = password[i];
        } else if (isalpha(password[i])) {
            character[count_c++] = password[i];
        } else {
            strcpy(number, "???");
            strcpy(character, "???");
            return;
        }
    }
    
    if (count_c == 0) {
        strcpy(character, "###");
    } else {
        character[count_c] = '\0';
    }

    if (count_n == 0) {
        strcpy(number, "###");
    } else {
        number[count_n] = '\0';
    }

    strcpy(digit_string, number);
    strcpy(alpha_string, character);
    return;
}

/* Add clients to queue */
void queue_add(pthread_mutex_t clients_mutex, client_t *clients[], client_t *cl){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(pthread_mutex_t clients_mutex, client_t *clients[], int uid){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}
	pthread_mutex_unlock(&clients_mutex);
}

client_t queue_find(pthread_mutex_t clients_mutex, client_t *clients[], int uid) {
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				return *clients[i];
			}
		}
	}
}

void traverse_queue(client_t *clients[]) {
    for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			printf("Client id: %d - client name: %s\n", clients[i]->uid, clients[i]->name);
		}
	}
}