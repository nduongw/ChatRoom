#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include "utils.c"
#include <sqlite3.h> 

#define MAX_SIZE 1024
Node *linked_list = NULL;


static int callback(void *data, int num_cols, char **rows, char **col_name){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i = 0; i < num_cols; i++){
      printf("%s = %s\n", col_name[i], rows[i] ? rows[i] : "NULL");
   }
   printf("\n");
   return 0;
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

void read_database(sqlite3 *db, char *sql, char *err_message, int rc) {
    rc = sqlite3_open("chat_room.db", &db);
    if (rc) {
        printf("Cant open database\n");
        return;
    } else {
        printf("Open database successfully\n");
    }
}

int main(int argc, char *argv[]) {
    //check number of parameters
    if (argc != 2) {
        printf("Invalid parameters");
        return -1;
    }

    if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65536) {
        printf("Invalid port number\n");
        return -1;
    }

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

    char *server_ip = "127.0.0.1";
    
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char sent_message[1024];
    char received_message[1024];
    int n;
    FILE *fptr;
    int count_signin = 0;

    // read_file(fptr, &linked_list);
    // update_file_copy();

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    printf("TCP Socket created!\n");

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

    char username[MAX_SIZE];
    char password[MAX_SIZE];
    int is_signin = 0;
    pid_t child_pid;

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
        printf("Your account doesnt exist\n");
        return 0;
    }

    // while(1) {
    //     client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
    //     printf("Client connected!\n");

    //     if ((child_pid = fork()) == 0) {
    //         close(server_sock);

    //         bzero(received_message, 1024);
    //         recv(client_sock, received_message, sizeof(received_message), 0);
    //         printf("%s\n", received_message);

    //         char messege[MAX_SIZE] = "Please input your username";
    //         send_message(client_sock, sent_message, messege);

    //         while(1) {
    //             receive_message(client_sock, received_message);
    //             strcpy(username, received_message);
    //             int check;
                
    //             while (1){
    //                 check = search_account(linked_list, username);
    //                 if (!check) {
    //                     strcpy(messege, "Account doesn't exist, please input your username again");
    //                     send_message(client_sock, sent_message, messege);

    //                     bzero(received_message, 1024);
    //                     receive_message(client_sock, received_message);
    //                     strcpy(username, received_message);
    //                 } else {
    //                     break;
    //                 }
    //             }
                
    //             strcpy(messege, "Please input your password");
    //             send_message(client_sock, sent_message, messege);

    //             receive_message(client_sock, received_message);
    //             strcpy(password, received_message);

    //             check = sign_in_account(linked_list, &count_signin, username, password);
    //             update_file();

    //             switch (check) {
    //                 case 0:
    //                     printf("Sending ok...\n");
    //                     strcpy(messege, "OK");
    //                     send_message(client_sock, sent_message, messege);
    //                     is_signin = 1;
    //                     break;
    //                 case -1:
    //                     printf("Sending block...\n");
    //                     write_to_file(fptr, linked_list);
    //                     strcpy(messege, "Account is blocked\nPlease input different account");
    //                     send_message(client_sock, sent_message, messege);
    //                     break;
    //                 case -2:
    //                     strcpy(messege, "Account has been signed in\nPlease input different account");
    //                     send_message(client_sock, sent_message, messege);
    //                     break;
    //                 case -3:
    //                     strcpy(messege, "Not OK\nPlease input different account");
    //                     send_message(client_sock, sent_message, messege);
    //                     break;
    //                 default:
    //                     break;
    //             }

    //             if (is_signin) {
    //                 printf("Move to the next part\n");
    //                 break;
    //             }
    //         }

    //         char digit_string[MAX_SIZE] = "*";
    //         char alpha_string[MAX_SIZE] = "*";

    //         while(1) {
    //             receive_message(client_sock, received_message);
    //             strcpy(password, received_message);

    //             if (strcmp(password, "bye") == 0 || strlen(password) == 0) {
    //                 strcpy(messege, "Goodbye");
    //                 send_message(client_sock, sent_message, messege);
    //                 sign_out_account(fptr, linked_list, username);
    //                 is_signin = 0;
    //                 break;
    //             } else {
    //                 encode_password(password, digit_string, alpha_string);
    //                 printf("Digit string: %s\nAlpha string: %s\n", digit_string, alpha_string);
                    
    //                 if (strcmp(digit_string, "*") == 0) {
    //                     strcpy(messege, "Invalid");
    //                     send_message(client_sock, sent_message, messege);
    //                 } else {
    //                     int check = change_password(fptr, linked_list, username, password);
    //                     if (check) {
    //                         write_to_file(fptr, linked_list);
    //                         strcat(digit_string, "-");
    //                         strcat(digit_string, alpha_string);
    //                         strcpy(messege, digit_string);
    //                         send_message(client_sock, sent_message, messege);
    //                     } else {
    //                         strcpy(messege, "Same");
    //                         send_message(client_sock, sent_message, messege);
    //                     }
    //                 }
    //             }

    //             update_file();
    //         }
    //     }

    // }
    return 0;
}