#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node_user{
    int id;
    char username[20];
    char password[100];
    int status;
    char last_login[20];
    char name[32];
    struct node_user *next;
};

typedef struct node_user *node;

node creatNode(int id, char username[], char password[], int status, char last_login[], char name[]){
    node tmp;
    tmp = (node)malloc(sizeof(struct node_user));
    tmp->id = id;
    strcpy(tmp->username, username);
    strcpy(tmp->password, password);
    tmp->status = status;
    strcpy(tmp->last_login, last_login);
    strcpy(tmp->name, name);
    tmp->next = NULL;
    return tmp;
}

node addHead(node head, int id, char username[], char password[], int status, char last_login[], char name[]){
    if(head == NULL){
        head = creatNode(id, username, password, status, last_login, name);
    }
    else{
        node tmp = creatNode(id, username, password, status, last_login, name);
        tmp->next = head;
        head = tmp;
    }
    return head;
}

node addTail(node head, int id, char username[], char password[], int status, char last_login[], char name[]){
    if(head == NULL){
        head = creatNode(id, username, password, status, last_login, name);
    }
    else{
        node tmp = head;
        while(tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = creatNode(id, username, password, status, last_login, name);
    }
    return head;
}

node insert(node head, int id, char username[], char password[], int status, char last_login[], char name[], int index){
   if(head == NULL || index == 0){
        head = creatNode(id, username, password, status, last_login, name);
   }
   else{
    node tmp = head;
    while(head->next != NULL && index != 1){
        tmp = tmp->next;
        index--;
    }
    node tmp1 = creatNode(id, username, password, status, last_login, name);
    tmp1->next = tmp->next;
    tmp->next = tmp1;
   }
   return head;
}

node delHead(node head){
    if(head == NULL){
        printf("List is empty !\n");
        return head;
    }
    else{
        node tmp = head;
        head = head->next;
        free(tmp);
    }
    return head;
}

node delTail(node head){
    node tmp = head;
    while(tmp->next->next != NULL){
        tmp = tmp->next;
    }
    node tmp1 = tmp->next;
    tmp->next = NULL;
    free(tmp1);
    return head;
}

node delnode(node head, int index){
    if(head == NULL || index == 0){
        head = delHead(head);
    }
    else{
        node tmp = head;
        while(tmp->next->next != NULL && index != 1){
            tmp = tmp->next;
            index--;
        }
        if(index != 1){
            head = delTail(head);
        }
        else{
            node tmp1 = tmp->next;
            tmp->next = tmp->next->next;
            free(tmp1);
        }
    }
    return head;
}

// element at in index: tra ve gia tri cua node ow vi tri index
// search node

void printList(node head){
    node tmp = head;
    while(tmp != NULL){
        printf("%d %s %s %d %s %s \n", tmp->id, tmp->username, tmp->password, tmp->status, tmp->last_login, tmp->name);
        tmp = tmp->next;
    }
}

// void sortLinkedList(node head){
//     node tmp, tmp1;

// }