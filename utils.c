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
    // printf("Sending messege: %s\n", messege);
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

list_node* list_create(char *data)
{
	list_node *l = malloc(sizeof(list_node));
	if (l != NULL) {
		l->next = NULL;
		strcpy(l->data, data);
	}

	return l;
}

/* Completely destroys a list
 * Arguments: A pointer to a pointer to a list
 */
void list_destroy(list_node **list)
{
	if (list == NULL) return;
	while (*list != NULL) {
		list_remove(list, *list);
	}
}

/* Creates a list node and inserts it after the specified node
 * Arguments: A node to insert after and the data the new node will contain
 */
list_node* list_insert_after(list_node *node, char *data)
{
    // printf("Word: %s\n", data);
	list_node *new_node = list_create(data);
	if (new_node) {
		new_node->next = node->next;
		node->next = new_node;
	}
	return new_node;
}

/* Creates a new list node and inserts it in the beginning of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node* list_insert_beginning(list_node *list, void *data)
{
	list_node *new_node = list_create(data);
	if (new_node != NULL) { new_node->next = list; }
	return new_node;
}

/* Creates a new list node and inserts it at the end of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node* list_insert_end(list_node *list, char *data)
{
	list_node *new_node = list_create(data);
	if (new_node != NULL) {
		for(list_node *it = list; it != NULL; it = it->next) {
			if (it->next == NULL) {
				it->next = new_node;
				break;
			}
		}
	}
	return new_node;
}

/* Removes a node from the list
 * Arguments: The list and the node that will be removed
 */
void list_remove(list_node **list, list_node *node)
{
	list_node *tmp = NULL;
	if (list == NULL || *list == NULL || node == NULL) return;

	if (*list == node) {
		*list = (*list)->next;
		free(node);
		node = NULL;
	} else {
		tmp = *list;
		while (tmp->next && tmp->next != node) tmp = tmp->next;
		if (tmp->next) {
			tmp->next = node->next;
			free(node);
			node = NULL;
		}
	}
}

/* Removes an element from a list by comparing the data pointers
 * Arguments: A pointer to a pointer to a list and the pointer to the data
 */
// void list_remove_by_data(list_node **list, void *data)
// {
// 	if (list == NULL || *list == NULL || data == NULL) return;
// 	list_remove(list, list_find_by_data(*list, data));
// }

/* Find an element in a list by the pointer to the element
 * Arguments: A pointer to a list and a pointer to the node/element
 */
list_node* list_find_node(list_node *list, list_node *node)
{
	while (list) {
		if (list == node) break;
		list = list->next;
	}
	return list;
}

/* Finds an elemt in a list by the data pointer
 * Arguments: A pointer to a list and a pointer to the data
 */
int list_find_by_data(list_node *list, char *data)
{
	while (list) {
		if (strcmp(list->data, data) == 0) {
            return 1;
        } 
		list = list->next;
	}
	return 0;
}

/* Finds an element in the list by using the comparison function
 * Arguments: A pointer to a list, the comparison function and a pointer to the
 * data
 */
list_node* list_find(list_node *list, int(*func)(list_node*,void*), void *data)
{
	if (!func) return NULL;
	while(list) {
		if (func(list, data)) break;
		list = list->next;
	}
	return list;
}

void traverse_list(list_node *list) {
    while (list->next != NULL) {
        list = list->next;
	}
}

void filter_message(list_node *bad_words_list, char *message) {
    message[strlen(message) - 1] = '\0';

    int check = 0;
    int count = 0;
    char word_list[100][100];
    char filtered_message[1024];
    int flag[100] = {0};
    char *token = strtok(message, " ");
    
    while(token != NULL) {
        strcpy(word_list[count], token);
        count++;
        token = strtok(NULL, " ");
    }

    int i = 0;
    while(i < count) {
        char search_word[1024];
        int node;

        strcpy(search_word, word_list[i]);
        node = list_find_by_data(bad_words_list, search_word);
        if (node) {
            flag[i] = 1;
            i++;
        } else {
            if (i >= count) {
                break;
            }
            bzero(search_word, 1024);
            sprintf(search_word, "%s %s", word_list[i], word_list[i + 1]);
            node = list_find_by_data(bad_words_list, search_word);
            if (node) {
                flag[i] = 1;
                flag[i+1] = 1;
                i += 2;
            } else {
                if (i >= count) {
                    break;
                }
                bzero(search_word, 1024);
                sprintf(search_word, "%s %s %s", word_list[i], word_list[i + 1], word_list[i + 2]);
                node = list_find_by_data(bad_words_list, search_word);
                if (node) {
                    flag[i] = 1;
                    flag[i+1] = 1;
                    flag[i+2] = 1;
                    i += 3;
                } else {
                    if (i >= count) {
                        break;
                    }
                    bzero(search_word, 1024);
                    sprintf(search_word, "%s %s %s %s", word_list[i], word_list[i + 1], word_list[i + 2], word_list[i + 3]);
                    node = list_find_by_data(bad_words_list, search_word);
                    if (node) {
                        flag[i] = 1;
                        flag[i+1] = 1;
                        flag[i+2] = 1;
                        flag[i+3] = 1;
                        i += 4;
                    } else {
                        i++;
                    }
                }
            }
        }
    }

    for (i = 0; i < count; i++) {
        if (flag[i]) {
            bzero(word_list[i], 100);
            strcpy(word_list[i], "***");
        }
    }

    for (i = 0; i < count; i++) {
        if (i == 0) {
            strcpy(filtered_message, word_list[i]);
        } else {
            strcat(filtered_message, word_list[i]);
        }

        if (i != count - 1) {
            strcat(filtered_message, " ");
        }

    }

    strcpy(message, filtered_message);
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

void get_sending_option(char *buffer, char *message, char *name) {
    int count_n = 0;
    int count_m = 0;

    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] != '&') {
            name[count_n++] = buffer[i];
        } else {
            break;
        }
    }

    name[count_n] = '\0';

    for (int i = count_n + 1; i < strlen(buffer); i++) {
        message[count_m++] = buffer[i];
    }

    message[count_m] = '\0';
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}