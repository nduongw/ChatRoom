#ifndef _L_LIST_H
#define _L_LIST_H

typedef struct data {
	char username[50];
    char password[50];
    char name[50];
    char last_logined[50];
    int id;
    int state;
    int sign_in;
    int count_signin;
} node_data;

typedef struct Node {
  node_data data;
  struct Node* next;
} Node;

void insertAtBeginning(struct Node** head_ref, node_data new_data);
void insertAfter(struct Node* prev_node, node_data new_data);
void insertAtEnd(struct Node** head_ref, node_data new_data);
void deleteNode(struct Node** head_ref, int key);
int checkExistedNode(struct Node** head_ref, char *username, char *password);
int checkValidAccount(struct Node** head_ref, char *username, char *password);
Node* searchNode(struct Node** head_ref, char *username, char *password);
Node* blockNode(struct Node** head_ref, char *username);
void sortLinkedList(struct Node** head_ref);
void printList(struct Node* node);
int search_account(Node *linked_list, char *username);
int sign_in_account(Node *linked_list, int *is_signed_in, char *username, char *password);
void write_to_file(FILE *fptr, Node *linked_list);
int change_password(FILE *fptr, Node *linked_list, char *username, char *password);
void sign_out_account(FILE *fptr, Node *linked_list,char *username);
#endif