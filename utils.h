#ifndef _L_LIST_H
#define _L_LIST_H

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

typedef struct list_node {
	char data[100];
	struct list_node *next;
} list_node;

#define MAX_SIZE 1024
#define MAX_CLIENTS 100
#define NUM_CHUNK 20

int check_account_exist(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[]);
void get_user_info(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[], int *id);
void get_user_info_by_id(sqlite3 *db, sqlite3_stmt *stmt, int user_id, char *name);
int get_user_id_by_name(sqlite3 *db, sqlite3_stmt *stmt, char *name);
int find_last_id(sqlite3 *db, sqlite3_stmt *stmt);
int register_new_account(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[]);
void create_new_group(sqlite3 *db, sqlite3_stmt *stmt);
void get_group_members_id(sqlite3 *db, sqlite3_stmt *stmt, int group_id, int member_ids[], int *length);

void send_message(int client_sock, char *packet, char *messege);
void receive_message(int client_sock, char *received_message);

void queue_add(pthread_mutex_t clients_mutex, client_t *clients[], client_t *cl);
void queue_remove(pthread_mutex_t clients_mutex, client_t *clients[], int uid);
client_t queue_find(pthread_mutex_t clients_mutex, client_t *clients[], int uid);
void traverse_queue(client_t *clients[]);

list_node* list_create(char *data);
void list_destroy(list_node **list);
list_node* list_insert_after(list_node *node, char *data);
list_node* list_insert_beginning(list_node *list, void *data);
list_node* list_insert_end(list_node *list, char *data);
void list_remove(list_node **list, list_node *node);
list_node* list_find_node(list_node *list, list_node *node);
int list_find_by_data(list_node *list, char *data);
list_node* list_find(list_node *list, int(*func)(list_node*,void*), void *data);
void traverse_list(list_node *list);

void filter_message(list_node *bad_words_list, char *message);

void split_buffer(char *buffer_out, char *option, char *message, char *name);
void get_sending_option(char *buffer, char *message, char *name);
const char *get_filename_ext(const char *filename);

#endif