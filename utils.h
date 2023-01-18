#ifndef _L_LIST_H
#define _L_LIST_H

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

#define MAX_SIZE 1024
#define MAX_CLIENTS 100

int check_account_exist(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[]);
void get_user_info(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[], int *id);
void get_user_info_by_id(sqlite3 *db, sqlite3_stmt *stmt, int user_id, char *name);
int get_user_id_by_name(sqlite3 *db, sqlite3_stmt *stmt, char *name);
int find_last_id(sqlite3 *db, sqlite3_stmt *stmt);
int register_new_account(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[]);
void create_new_group(sqlite3 *db, sqlite3_stmt *stmt);
void get_group_members_id(sqlite3 *db, sqlite3_stmt *stmt, int group_id, int member_ids[], int *length);
int read_database(sqlite3 *db, sqlite3_stmt *stmt);
void send_message(int client_sock, char *packet, char *messege);
void receive_message(int client_sock, char *received_message);
#endif