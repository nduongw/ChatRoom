#include <sqlite3.h> 
#include <stdio.h>
#include <string.h>

#define MAX_SIZE 1024

int main() {
    sqlite3 *db;
    sqlite3_stmt *res;
    char *err_message = 0;
    char *sql;
    int rc;
    const char* data = "Callback function called";

    // read_database(db, sql, err_message, rc);
    rc = sqlite3_open("chat_room.db", &db);
    if (rc) {
        printf("Cant open database\n");
        return -1;
    } else {
        printf("Open database successfully\n");
    }

    char username[MAX_SIZE];
    char password[MAX_SIZE];
    int is_signin = 0;

    printf("Input account: ");
    scanf("%s", username);
    printf("Input password: ");
    scanf("%s", password);

    sql = "SELECT * FROM users WHERE account = ? AND password = ?";
    rc = sqlite3_prepare(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(res, 1, username, strlen(username), SQLITE_STATIC);
        sqlite3_bind_text(res, 2, password, strlen(password), SQLITE_STATIC);
    } else {
        printf("Failed to prepare statement\n");
        return 0;
    }

    int step = sqlite3_step(res);
    if (step == SQLITE_ROW) {
        printf("%s: ", sqlite3_column_text(res, 0));
        printf("%s\n", sqlite3_column_text(res, 1));
    } else {
        printf("Your account does not exist\n");
        // return 0;
    }
}