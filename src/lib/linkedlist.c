/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  linkedlist.c                           */
/*  PRINCIPAL AUTHOR      :  qcLiu                                  */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/02/03                             */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *//*}}}*/

#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main/vm.h"

static void throwException(char* error_info) {
    printf("Exception: %s\n", error_info);
    exit(0);
}

/* head          last
 * ----------    ---------
 * |head    |--> |dummy  |-->NULL
 * ----------    ---------
 * Init the linkedlist. And add a dummy node to make
 * the linkedlist op easy.
 *
 * @qcliu 2015/02/03
 */
LinkedList* initLinkedList() {
    LinkedList* head = (LinkedList*)malloc(sizeof(LinkedList));
    LNode* node = (LNode*)malloc(sizeof(LNode));

    head->next = node;
    head->length = 0;
    head->Objsize = 0;

    node->next = NULL;
    node->data = NULL;

    head->last = node;
    return head;
}

/*
 * Add a node in the last. Succ return 1, else return 0.
 *
 * @qcliu 2015/02/03
 */
int addLast(LinkedList* head, void* data) {
    if(data == NULL||head == NULL)
      throwException("head is NULL!!@addLast");
    LNode* node = (LNode*)malloc(sizeof(LNode));
    node->data = data;

    head->last->next = node;
    node->next = NULL;
    head->last = node;
    head->length++;

    return 1;
}

/*
 * Return the first node of the list
 * @qcliu 2015/02/03
 */
void* getFirst(LinkedList* head) {
    if(head == NULL)
      throwException("head is NULL!!@getFirst");
    if(head->next == NULL)
      throwException("dummy is NULL!!@getFirst");
    return head->next->next->data;
}

/*
 * Return the last node of the list
 * @qcliu 2015/02/03
 */
void* getLast(LinkedList* head) {
    if (head == NULL)
      throwException("head is NULL!! @getLast");

    return head->last->data;
}


/*
 * Return the node that index refer
 * @qcliu 2015/02/03
 */
void* getIndexOf(LinkedList* head, int index) {
    if (head == NULL)
      throwException("head is NULL!!");
    if ((index >= head->length) || (index < 0))
      throwException("index > head->length || index <= 0 @getIndexOf");

    LNode* ptr = GETLINK(head)->next;

    while (index > 0) {
        ptr = ptr->next;
        index--;
    }

    return ptr->data;

}

/*
 * Delet the first LNode.
 * @qcliu 2015/02/03
 */
int removeFirst(LinkedList* head) {
    if (head == NULL)
      throwException("head is NULL!!@removeFirst");
    if (head->length <= 0)
      throwException("length <= 0!@removeFirst");

    LNode* ptr = GETLINK(head);
    LNode* remove_node = ptr->next;
    ptr->next = ptr->next->next;
    free(remove_node);
    head->length--;

    return 1;
}

/*
 * Delet the node that index refer to.
 * NOTE: the last note is significate.
 * @qcliu 2015/02/03
 */
int removeIndexOf(LinkedList* head, int index) {
    if (head == NULL)
      throwException("head is NULL!!@removeIndexOf");
    if (head->length <= 0)
      throwException("length <= 0! @removeIndexOf");
    if ((index >= head->length) || (index < 0))
      throwException("index > head->length || index <= 0 @getIndexOf");

    LNode* ptr = GETLINK(head)->next;
    LNode* prev = GETLINK(head);
    int i;

    for (; index > 0; index--) {
        ptr = ptr->next;
        prev = prev->next;
    }

    prev->next = ptr->next;
    free(ptr);
    head->length--;

    return 1;
}

/*
 * Delet the data<T> from the linkedlist. First find the 
 * data<T> in the list.
 * NOTE: The data in this method is refer to VNode.
 * @qcliu 2015/02/04
 */
int removeFromList(LinkedList* head, void* data) {
    if (head == NULL || data == NULL)
      throwException("head or data is NULL! @removeFromList");

    LNode* ptr = GETLINK(head)->next;
    LNode* prev = GETLINK(head);

    while (ptr != NULL) {
        if (ptr->data == data)
          break;
        ptr = ptr->next;
        prev = prev->next;
    }

    if (ptr) {
        prev->next = ptr->next;
        free(ptr);
        head->length--;
        return 1;
    }
    else {
        printf("data hasn't in the LinkedList!\n");
        return 0;
    }
}

/*
 * Return 0 or 1.
 * @qcliu 2015/02/03
 */
int isEmpty(LinkedList* head) {
    if (head == NULL)
      throwException("head == NULL! @isEmpty");

    if (head->length == 0)
      return TRUE;
    else
      return FALSE;
}

/*
 * Return length of the linkedlist.
 * @qcliu 2015/02/03
 */
int getLength(LinkedList* head) {
    if (head == NULL)
      throwException("head == NULL!! @ getLength");

    return head->length;
}

/*
 * Find the certain node from the linkedlist.
 * note: ptr point to the dummy node. If found the data,
 *      return the address of the node, else return NULL.
 * @qcliu 2015/02/04
 */

void* findLNodeinList(LinkedList* head, void* data) {
    if (head == NULL || data == NULL)
      throwException("head is NULL! @lookupLNode");


    LNode* ptr = GETLINK(head);

    while (ptr->next) {
        LNode* node = ptr->next;
        if (node->data == data)
          return node->data;
        ptr = ptr->next;
    }

    return NULL;

}

Class* findClassInTable(LinkedList* head, char* classname) {
    if (head == NULL || classname == NULL)
      throwException("head is NULL! @ findClassInTable");

    LNode* ptr = GETLINK(head);

    while (ptr->next) {
        LNode* node = ptr->next;
        Class* class = (Class*)node->data;
        ClassBlock* cb = CLASS_CB(class);
        if (strcmp(cb->this_classname, classname) == 0)
          return class;
        ptr = ptr->next;
    }
    return NULL;
}

int addClass2Table(LinkedList* head, void* data) {
    if (findLNodeinList(head, data))
      return 0;

    return addLast(head, data);
}

DllEntry* findDllInTable(LinkedList* head, char* dllname) {
    if (head == NULL || dllname == NULL)
      throwException("head is NULL");

    LNode* ptr = GETLINK(head);

    while (ptr->next) {
        LNode* node = ptr->next;
        DllEntry* de = node->data;
        if (0 == strcmp(de->name, dllname))
          return de;
        ptr = ptr->next;
    }
    return NULL;
}

int addDll2Table(LinkedList* head, void* data) {
    return addClass2Table(head, data);
}
