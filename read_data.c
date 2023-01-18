#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <strings.h>

#define MAX_CLIENTS 100

sqlite3 *db;
sqlite3_stmt *stmt;

typedef struct{
	// struct sockaddr_in address;
	// int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

/* Add clients to queue */
void queue_add(client_t *cl){
	// pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}
	// pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(int uid){
	// pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}
	// pthread_mutex_unlock(&clients_mutex);
}

client_t queue_find(int uid) {
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				return *clients[i];
			}
		}
	}
}

int check_account_exist(char username[], char password[]) {
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

void get_user_info(char username[], char password[], char name[], int *id) {

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

void get_user_info_by_id(int user_id, char *name) {
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

int get_user_id_by_name(char *name) {
	char *sql = "SELECT id FROM users WHERE username = ?";
    int check = sqlite3_prepare(db, sql, -1, &stmt, NULL);
    if (check == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
    } else {
        printf("Fail to prepare statement\n");
        return;
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

int find_last_id() {
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

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int register_new_account(char username[], char password[], char name[]) {
	int new_id = find_last_id();
	int check = check_account_exist(username, password);
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
        sqlite3_bind_text(stmt, 4, password, strlen(name), SQLITE_STATIC);
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

void create_new_group() {
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

void get_group_members_id(int group_id, int member_ids[], int *length) {
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


int read_database() {
	sqlite3_open("chat_room.db", &db);

	if (db == NULL) {
		printf("Failed to open DB\n");
		return 0;
	}

	return 1;
}

int main() {
	
    char username[100];
    char password[100];
    char name[100];
	int id, num_members;
	int group_members[100];
	
	int check = read_database();
	if (!check) {
		printf("Fail to read database\n");
	} else {
		printf("Read database successful!\n");
	}


	// create_new_group();

	// printf("Input username: ");
	// scanf("%s", username);
	// printf("Input password: ");
	// scanf("%s", password);
	// printf("Input name: ");
	// scanf("%s", name);
	printf("Input group id: ");
	scanf("%d", &id);
	get_group_members_id(id, group_members, &num_members);
	printf("Total members is %d\n", num_members);

	for (int i = 0; i < num_members; i++) {
		get_user_info_by_id(group_members[i], name);
		printf("%s\n", name);
	}


	// register_new_account(username, password, name);

    // get_user_info(username, password, name, &id);
    // printf("User infomation: %d - %s\n", id, name);
	// printf("Last id : %d\n", find_last_id());
	// printf("Performing query...\n");
    // char *sql = "SELECT * FROM users";
	// sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	
	// printf("Got results:\n");
	// while (sqlite3_step(stmt) != SQLITE_DONE) {
	// 	int i;
	// 	int num_cols = sqlite3_column_count(stmt);
		
	// 	for (i = 0; i < num_cols; i++) {
	// 		switch (sqlite3_column_type(stmt, i))
	// 		{
	// 		case (SQLITE3_TEXT):
	// 			printf("%s, ", sqlite3_column_text(stmt, i));
	// 			break;
	// 		case (SQLITE_INTEGER):
	// 			printf("%d, ", sqlite3_column_int(stmt, i));
	// 			break;
	// 		default:
	// 			break;
	// 		}
	// 	}
	// 	printf("\n");
	// }
	// char *err_msg = 0;

	// char *sql = "DROP TABLE IF EXISTS groups_members;" 
	// 		"CREATE TABLE groups_members("  \
	// 		"id INT NOT NULL," \
	// 		"user_id INT NOT NULL);";

	// /* Execute SQL statement */
	// int rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

	// if( rc != SQLITE_OK ){
	// 	fprintf(stderr, "SQL error: %s\n", err_msg);
	// 	sqlite3_free(&err_msg);
	// } else {
	// 	fprintf(stdout, "Table created successfully\n");
	// }



	// char *err_msg = 0;

	// char *sql = "INSERT INTO groups_members VALUES (11, 1);" 
    //             "INSERT INTO groups_members VALUES (11, 13);";

    // int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    // if (rc != SQLITE_OK ) {
        
    //     fprintf(stderr, "SQL error: %s\n", err_msg);
        
    //     sqlite3_free(err_msg);        
    //     sqlite3_close(db);
        
    //     return 1;
    // } else {
	// 	printf("DOne\n");
	// }

	sqlite3_finalize(stmt);

	sqlite3_close(db);
	return 0;
}