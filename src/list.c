/*
* list.c
*
* Simple implementation of a linked list.
*
*  Created on: Oct 27, 2016
*      Author: Dennis Seller
*/

#include "list.h"
#include <stdlib.h>

/*!
* Creates a new linked list.
*/
linked_list *list_new() {
    linked_list *list = malloc(sizeof(linked_list));
    list->length = 0;
    list->root_node = malloc(sizeof(list_node));
    list->root_node->element = NULL;
    list->root_node->next = NULL;
    list->root_node->prev = NULL;
    return list;
}

void list_free(linked_list *list) {
    if (list->root_node == NULL)
        return;

    for (uint32_t i = 0; i < list->length; i++) {
        free(list_at(list, i));
    }

    free(list->root_node);
    free(list);
}

list_node *list_add(linked_list *list, void *object) {
    list_node *last = list_last(list);
    list_node *new = malloc(sizeof(list_node));
    new->element = object;
    new->next = NULL;
    new->prev = last;
    last->next = new;

    list->length++;
    return new;
}

list_node *list_remove(linked_list *list, void *object) {
    list_node *cur = list->root_node;
    for (uint32_t i = 0; cur != NULL; i++) {
        if (cur->element == object) {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            list->length--;
            return cur;
        }

        cur = cur->next;
    }
    return NULL;
}

void *list_at(linked_list *list, uint32_t idx) {
    if (list->root_node == NULL || list->root_node->next == NULL)
        return NULL;

    list_node *cur = list->root_node->next;
    for (uint32_t i = 0; cur != NULL && i < idx; i++)
        cur = cur->next;

    if (cur == NULL)
        return NULL;
    return cur->element;
}

list_node *list_last(linked_list *list) {
    list_node *node = list->root_node;
    while (node->next != NULL)
        node = node->next;
    return node;
}

void **list_toarray(linked_list *list) {
    void **result = (void**)malloc(list->length * sizeof(void*));
    list_node *node = list->root_node->next;
    for (uint32_t i = 0;
        i < list->length && node != NULL;
        i++, node = node->next) {
        result[i] = node->element;
    }
    return result;
}

void list_remove_node(linked_list *list, list_node *node) {
	node->prev->next = node->next;
	if (node->next != NULL)
		node->next->prev = node->prev;
	list->length--;
}