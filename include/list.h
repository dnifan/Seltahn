/*
* list.h
*
*  Created on: Oct 27, 2016
*      Author: Dennis Seller
*/

#include "stdint.h"

typedef struct list_node {
    /*!
    * Pointer to the actual data object.
    */
    void *element;

    /*!
    * Pointer to the previous entry in the linked list.
    */
    struct list_node *prev;

    /*!
    * Pointer to the next entry in the linked list.
    */
    struct list_node *next;
} list_node;

typedef struct {
    list_node *root_node;
    uint32_t length;
} linked_list;

linked_list *list_new();
void list_free(linked_list *list);
list_node *list_add(linked_list *list, void *object);
list_node *list_remove(linked_list *list, void *object);
void *list_at(linked_list *list, uint32_t idx);
list_node *list_first(linked_list *list);
list_node *list_last(linked_list *list);
