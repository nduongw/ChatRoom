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
#include <signal.h>

sqlite3 *db;
sqlite3_stmt *stmt;
char *sql;
int rc;
static int client_count = 0;
static int uid = 0;

int server_sock, client_sock;
struct sockaddr_in server_addr, client_addr;
socklen_t addr_size;
char sent_message[1024];
char received_message[1024];
int n, is_login;
pthread_t tid;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

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

// int handle_login(char username[], char password[]) {
//     char message[MAX_SIZE] = "Input your account: ";
//     send_message(client_sock, sent_message, message);

//     receive_message(client_sock, received_message);
//     strcpy(username, received_message);

//     strcpy(message, "Input your password: ");
//     send_message(client_sock, sent_message, message);

//     receive_message(client_sock, received_message);
//     strcpy(password, received_message);

//     int check = check_account_exist(username, password);

//     if (!check) {
//         strcpy(message, "Your account does not exist or wrong password");
//         send_message(client_sock, sent_message, message);
//         receive_message(client_sock, received_message);

//         return 0;
//     } else {
//         strcpy(message, "Login successful\n");
//         send_message(client_sock, sent_message, message);
//         receive_message(client_sock, received_message);

//         return 1;
//     }
// }

void handle_logout() {
    printf("Handle logout\n");
}

// void handle_register(char username[], char password[], char name[]) {
//     char message[MAX_SIZE] = "Input new account: ";
//     send_message(client_sock, sent_message, message);
//     receive_message(client_sock, received_message);
//     strcpy(username, received_message);

//     strcpy(message, "Input new password: ");
//     send_message(client_sock, sent_message, message);
//     receive_message(client_sock, received_message);
//     strcpy(password, received_message);

//     strcpy(message, "Input name: ");
//     send_message(client_sock, sent_message, message);
//     receive_message(client_sock, received_message);
//     strcpy(name, received_message);

//     int check = register_new_account(username, password, name);

//     if (check) {
//         strcpy(message, "Register successful");
//         send_message(client_sock, sent_message, message);
//         receive_message(client_sock, received_message);
//     } else {
//         strcpy(message, "Can not register this account");
//         send_message(client_sock, sent_message, message);
//         receive_message(client_sock, received_message);
//     }
// }

void *handle_client() {
    char choice_str[MAX_SIZE];
    char username[MAX_SIZE];
    char password[MAX_SIZE];
    char name[MAX_SIZE];

    bzero(received_message, 1024);
    recv(client_sock, received_message, sizeof(received_message), 0);
    printf("%s\n", received_message);

    while(1) {
        char message[MAX_SIZE] = "Chatting App\n1.Login\n2.Logout\n3.Register\nYour choice: ";
        send_message(client_sock, sent_message, message);

        bzero(received_message, 1024);
        receive_message(client_sock, received_message);
        strcpy(choice_str, received_message);

        int choice = atoi(choice_str);

        switch(choice) {
            case 1:
                // is_login = handle_login(username, password);
                break;
            case 2:
                handle_logout();
                break;
            case 3:
                // handle_register(username, password, name);
                break;
            default:
                strcpy(message, "Invalid choice\n");
                send_message(client_sock, sent_message, message);
                receive_message(client_sock, received_message);
                break;
        }

        if (is_login) {
            printf("Client has logined!\n");
            break;
        }
    }

}

int main(int argc, char *argv[]) {
    char *server_ip = "127.0.0.1";
    int option = 1;

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

    //read database
    // rc = sqlite3_open("chat_room.db", &db);
    // if (rc) {
    //     printf("Cant open database\n");
    //     return -1;
    // } else {
    //     printf("Open database successfully\n");
    // }

    read_database(db, stmt);


    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(server_sock, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}

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
    pid_t child_pid;

    while(1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
        printf("Client connected!\n");

        pthread_create(&tid, NULL, &handle_client, NULL);
    }
    return 0;
}