#include "linked_list.h"

// Creates and returns a new list
list_t* list_create()
{

    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_t *new_list = malloc(sizeof(list_t));
    new_list -> head = NULL;
    new_list -> count = 0;
    return new_list;

}

// Destroys a list
void list_destroy(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t *iterator = list->head;

    while(iterator){
        list_node_t *temp = iterator;
        iterator = iterator->next;
        free(temp);
    }
    free(list);
}

// Returns beginning of the list
list_node_t* list_begin(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return list -> head;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return node->next;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return node -> data;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return list -> count;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t *iterator;
    iterator = list_begin(list);

    while(iterator){
        if((iterator -> data) == data){
            return iterator;
        }
        iterator = list_next(iterator);
    }

    return NULL;
}

// Inserts a new node in the list with the given data
void list_insert(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t* newNode = malloc(sizeof(list_node_t));
    newNode -> prev = NULL;
    newNode -> next = list -> head;
    newNode -> data = data;
    if(list_begin(list)){
        list -> head -> prev = newNode;
        list -> head = newNode;
        list -> count += 1;
    }
    list->head = newNode;
    list -> count += 1;

}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    if(node == NULL){
        return;
    }

    //head and tail
    if((node->prev) == NULL && (node->next) == NULL){
        list -> head = NULL;
        free(node);
        return;
    }

    //head
    if(node->prev == NULL){
        node -> next -> prev = NULL;
        list -> head = node -> next;
        node -> next = NULL;
        free(node);
        return;
    }

    //tail
    if(node -> next == NULL){
        node -> prev -> next = NULL;
        node -> prev = NULL;
        free(node);
        return;
    }

    //middle
    node -> prev -> next = node -> next;
    node -> next -> prev = node -> prev;
    node -> prev = NULL;
    node -> next = NULL;
    free(node);
    return;
}

// Executes a function for each element in the list
void list_foreach(list_t* list, void (*func)(void* data))
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
}
