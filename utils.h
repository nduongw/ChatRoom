#ifndef _L_LIST_H
#define _L_LIST_H

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
	int first_join;
	int is_online;
	int is_inchat;
} client_t;

typedef struct list_node {
	char data[100];
	struct list_node *next;
} list_node;

#define MAX_SIZE 1024
#define MAX_CLIENTS 100
#define NUM_CHUNK 20

void send_message(int client_sock, char *packet, char *messege);
void receive_message(int client_sock, char *received_message);

void queue_add(pthread_mutex_t clients_mutex, client_t *clients[], client_t *cl);
void queue_remove(pthread_mutex_t clients_mutex, client_t *clients[], int client_sock);
int queue_find(pthread_mutex_t clients_mutex, client_t *clients[], char *username);
client_t *queue_find_by_sockfd(pthread_mutex_t clients_mutex, client_t *clients[], int client_sock);
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

int check_account_exist(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[]);
void get_user_info(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[], int *id);
int find_last_id(sqlite3 *db, sqlite3_stmt *stmt);
int register_new_account(sqlite3 *db, sqlite3_stmt *stmt, char username[], char password[], char name[]);
void get_group_id(sqlite3 *db, sqlite3_stmt *stmt, char *group_name, int *group_id);
void get_group_members_id(sqlite3 *db, sqlite3_stmt *stmt, int group_id, int member_ids[], int *length);

int count_space(char *buffer);
int add_to_block_table(sqlite3 *db, sqlite3_stmt *stmt, int uid, int buid);
int delete_from_block(sqlite3 *db, sqlite3_stmt *stmt, int uid, int buid);
#endif