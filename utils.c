#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Insert at the beginning
void insertAtBeginning(struct Node** head_ref, node_data new_data) {
  // Allocate memory to a node
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));

  // insert the data
  new_node->data = new_data;

  new_node->next = (*head_ref);

  // Move head to new node
  (*head_ref) = new_node;
}

// Insert a node after a node
void insertAfter(struct Node* prev_node, node_data new_data) {
  if (prev_node == NULL) {
  printf("the given previous node cannot be NULL");
  return;
  }

  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
  new_node->data = new_data;
  new_node->next = prev_node->next;
  prev_node->next = new_node;
}

// Insert the the end
void insertAtEnd(struct Node** head_ref, node_data new_data) {
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
  struct Node* last = *head_ref; /* used in step 5*/

  new_node->data = new_data;
  new_node->next = NULL;

  if (*head_ref == NULL) {
  *head_ref = new_node;
  return;
  }

  while (last->next != NULL) last = last->next;

  last->next = new_node;
  return;
}

// Delete a node
void deleteNode(struct Node** head_ref, int key) {
  struct Node *temp = *head_ref, *prev;

  if (temp != NULL && temp->data.state == key) {
  *head_ref = temp->next;
  free(temp);
  return;
  }
  // Find the key to be deleted
  while (temp != NULL && temp->data.state != key) {
  prev = temp;
  temp = temp->next;
  }

  // If the key is not present
  if (temp == NULL) return;

  // Remove the node
  prev->next = temp->next;

  free(temp);
}

// Search a node
int checkExistedNode(struct Node** head_ref, char *username, char *password) {
  struct Node* current = *head_ref;

  while (current != NULL) {
  if (strcmp(current->data.username, username) == 0 && strcmp(current->data.password, password) == 0)  return 1;
  current = current->next;
  }
  return 0;
}

int checkValidAccount(struct Node** head_ref, char *username, char *password) {
  struct Node* current = *head_ref;

  while (current != NULL) {
  if (strcmp(current->data.username, username) == 0 && strcmp(current->data.password, password) == 0)  return 1;
  else if (strcmp(current->data.username, username) == 0 && strcmp(current->data.password, password) != 0) return 2;
  current = current->next;
  }
  return 0;
}

Node* searchNode(struct Node** head_ref, char *username, char *password) {
  struct Node* current = *head_ref;

  while (current != NULL) {
  if (strcmp(current->data.username, username) == 0 && strcmp(current->data.password, password) == 0)  return current;
  current = current->next;
  }
  return NULL;
}

Node* blockNode(struct Node** head_ref, char *username) {
  struct Node* current = *head_ref;

  while (current != NULL) {
  if (strcmp(current->data.username, username) == 0)  return current;
  current = current->next;
  }
  return NULL;
}

// Sort the linked list
void sortLinkedList(struct Node** head_ref) {
  struct Node *current = *head_ref, *index = NULL;
  node_data temp;

  if (head_ref == NULL) {
  return;
  } else {
  while (current != NULL) {
    // index points to the node next to current
    index = current->next;

    while (index != NULL) {
    if (current->data.state > index->data.state) {
      temp = current->data;
      current->data = index->data;
      index->data = temp;
    }
    index = index->next;
    }
    current = current->next;
  }
  }
}

// Print the linked list
void printList(struct Node* node) {
  while (node != NULL) {
  printf(" %s ", node->data.username);
  printf(" %s ", node->data.password);
  printf(" %d", node->data.state);
  printf(" %d\n", node->data.sign_in);
  node = node->next;
  }
}

int search_account(Node *linked_list, char *username) {
    Node *requested_node = blockNode(&linked_list, username);
    if (requested_node == NULL) {
        return 0;
    }
    return 1;
}

int sign_in_account(Node *linked_list, int *is_signed_in, char *username, char *password) {
    Node *normal_node = blockNode(&linked_list, username);

    if (normal_node->data.state == 0) {
        printf("Account is blocked\n");
        return -1;
    }

    if (normal_node->data.sign_in == 1) {
        printf("Account has been signed in!\n");
        return -2;
    }

    int check = checkValidAccount(&linked_list, username, password);

    if (check == 2) {
        normal_node->data.count_signin++;
        if (normal_node->data.count_signin % 3 == 0) {
          normal_node->data.state = 0;
          return -1;
        }
        return -3;
    } else if (check == 1) {
        Node *signed_in_node = searchNode(&linked_list, username, password);

        signed_in_node->data.sign_in = 1;
        signed_in_node->data.count_signin = 0;
        *is_signed_in += 1;
        
        return 0;
    }
}

void write_to_file(FILE *fptr, Node *linked_list)
{
    fptr = fopen("nguoidung.txt", "w");
    while (linked_list != NULL)
    {
        fprintf(fptr, "%s %s %d\n", linked_list->data.username, linked_list->data.password, linked_list->data.state);
        linked_list = linked_list->next;
    }

    fclose(fptr);
}

int change_password(FILE *fptr, Node *linked_list, char *username, char *password)
{
    Node *signed_in_user = blockNode(&linked_list, username);
    
    if (strcmp(password, signed_in_user->data.password) == 0) {
        return 0;
    }
    else {
        strcpy(signed_in_user->data.password, password);
        return 1;
    }
}

void sign_out_account(FILE *fptr, Node *linked_list,char *username)
{
    Node *signed_in_user = blockNode(&linked_list, username);
    signed_in_user->data.sign_in = 0;
}

