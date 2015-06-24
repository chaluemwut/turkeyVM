/*------------------------------------------------------------------*//*{{{*/
/* Copyright (C) SSE-USTC, 2014-2015                                */
/*                                                                  */
/*  FILE NAME             :  linkedlist.h                           */
/*  PRINCIPAL AUTHOR      :  qcLiu                                  */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  ANY                                    */
/*  DATE OF FIRST RELEASE :  2015/02/03                             */
/*------------------------------------------------------------------*/

/*
 * Revision log:
 *//*}}}*/
#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include "vm.h"

#define FALSE 0
#define TRUE 1
#define GETLINK(head) head->next

typedef struct node
{
    void* data;
    struct node* next;
}LNode;

typedef struct head
{
    LNode* next;
    int length;
    LNode* last;
    int Objsize;
}LinkedList;

/* head          last
 * ----------    ---------
 * |head    |--> |dummy  |-->NULL
 * ----------    ---------
 * Init the linkedlist. And add a dummy node to make
 * the linkedlist op easy.
 *
 * @qcliu 2015/02/03
 */
LinkedList* initLinkedList();

/*
 * Add a node in the last. Succ return 1, else return 0.
 *
 * @qcliu 2015/02/03
 */
int addLast(LinkedList* head, void* data);

/*
 * Return the first node of the list
 * @qcliu 2015/02/03
 */
void* getFirst(LinkedList* head);

/*
 * Return the last node of the list
 * @qcliu 2015/02/03
 */
void* getLast(LinkedList* head);

/*
 * Return the node that index refer
 * @qcliu 2015/02/03
 */
void* getIndexOf(LinkedList* head, int index);

/*
 * Delet the first LNode.
 * @qcliu 2015/02/03
 */
int removeFirst(LinkedList* head);

/*
 * Delet the node that index refer to.
 * note: the last note is significate.
 * @qcliu 2015/02/03
 */
int removeIndexOf(LinkedList* head, int index);

/*
 * Return length of the linkedlist.
 * @qcliu 2015/02/03
 */
int getLength(LinkedList* head);

/*
 * Return 0 or 1.
 * @qcliu 2015/02/03
 */
int isEmpty(LinkedList* head);

/*
 * Find the certain node from the linkedlist.
 * note: ptr point to the dummy node. If found the data,
 *      return the address of the node, else return NULL.
 * @qcliu 2015/02/04
 */
void* findLNodeinList(LinkedList* head, void* data);

/*
 * Delet the data<T> from the linkedlist. First find the 
 * data<T> in the list.
 * @qcliu 2015/02/04
 */
int removeFromList(LinkedList* head, void* data);

Class*  findClassInTable(LinkedList* head, char* classname);

int addClass2Table(LinkedList* head, void* data);

DllEntry* findDllInTable(LinkedList* head, char* dllname);

int addDll2Table(LinkedList* head, void* data);
#endif
