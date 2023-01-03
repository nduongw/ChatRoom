#include <stdio.h>
#include <sqlite3.h> 
#include <string.h>

static int callback(void *data, int num_cols, char **rows, char **col_name){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i = 0; i < num_cols; i++){
      printf("%s = %s\n", col_name[i], rows[i] ? rows[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main() {
    sqlite3 *db;
    char *err_message = 0;
    char *sql;
    int rc;
    const char* data = "Callback function called";

    rc = sqlite3_open("chat_room.db", &db);
    if (rc) {
        printf("Cant open database\n");
        return -1;
    } else {
        printf("Open database successfully\n");
    }

    sql = "SELECT * FROM users";

    rc = sqlite3_exec(db, sql, callback, (void *)data, &err_message);
    if (rc != SQLITE_OK) {
        printf("sql error\n");
        sqlite3_free(err_message);
        return -1;
    } else {
        printf("Execute succesfully\n");
    }

    
}