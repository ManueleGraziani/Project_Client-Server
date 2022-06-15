#ifndef SERVER_TCP_C_MANUELEGRAZIANI_LIST_H
#define SERVER_TCP_C_MANUELEGRAZIANI_LIST_H

#include <sys/param.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 *
 *     Developed by Manuele Graziani 5H
 *
 */


typedef struct {
    void *next;
    void *data;
} ListNode;

typedef struct {

    int size;
    ListNode *head;
    ListNode *tail;

} List;

ListNode *allocateListNode(void *data) {

    ListNode *output = malloc(sizeof(ListNode));
    if (output == NULL)
        exit(EXIT_FAILURE);
    else {

        output->data = data;
        output->next = NULL;

        return output;
    }
}

List *allocateList() {

    List *list = malloc(sizeof(List));
    if (list == NULL)
        exit(EXIT_FAILURE);
    else {

        list->head = NULL;
        list->tail = NULL;
        list->size = 0;

        return list;
    }
}

void insert(List* list, void* data) {

    ListNode *newNode = allocateListNode(data);

    if (list->head == NULL) {

        list->head = newNode;
        list->tail = newNode;

    } else {

        list->tail->next = newNode;
        list->tail = newNode;
    }

    list->size++;
}



#endif //SERVER_TCP_C_MANUELEGRAZIANI_LIST_H
