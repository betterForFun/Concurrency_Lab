#include "channel.h"
#include "linked_list.h"

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
channel_t* channel_create(size_t size)
{
    /* IMPLEMENT THIS */

    //if the size is 0 or less
    //would not be iplemented(unbeffered)
    if(size <= 0){
        return NULL;
    }

    channel_t *newChannel = (channel_t*) malloc(sizeof(channel_t));
    newChannel -> buffer = buffer_create(size);
    newChannel -> flag = 1;
    newChannel -> size = size;
    pthread_mutex_init(&newChannel->mutex,NULL);
	pthread_cond_init(&newChannel->send,NULL);
	pthread_cond_init(&newChannel->receive,NULL);
    newChannel -> selectcond_list = list_create();
    return newChannel;

}

// Writes data to the given channel
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_send(channel_t *channel, void* data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);

    //check if the channel is full
    //if so wait untill it has space to receive
    while (buffer_capacity(channel->buffer) == buffer_current_size(channel->buffer) && channel->flag == 1)
    {
        pthread_cond_wait(&channel->send, &channel->mutex);
    }

    // check if the channel is closed
    if(channel->flag == 0){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }

    buffer_add(channel->buffer,data);
    pthread_cond_signal(&channel->receive);
    
    //wake up all the CV in the list
    list_node_t *iterator;
    iterator = list_begin(channel->selectcond_list);
    while(iterator){
        pthread_cond_signal(iterator->data);
        iterator = list_next(iterator);
    }


    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);

    //if the channel is empty
    //wait untill it has something to send
    while (buffer_current_size(channel->buffer) == 0 && channel->flag == 1){
        pthread_cond_wait(&channel->receive,&channel->mutex);
    }
    // check if the channel is closed
    if(channel->flag == 0){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }

    buffer_remove(channel->buffer,data);
    pthread_cond_signal(&channel->send);

    //wake up all the CV in the list
    list_node_t *iterator;
    iterator = list_begin(channel->selectcond_list);
    while(iterator){
        pthread_cond_signal(iterator->data);
        iterator = list_next(iterator);
    }

    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Writes data to the given channel
// This is a non-blocking call i.e., the function simply returns if the channel is full
// Returns SUCCESS for successfully writing data to the channel,
// CHANNEL_FULL if the channel is full and the data was not added to the buffer,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_send(channel_t* channel, void* data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    // check if the channel is closed
    if(channel->flag == 0){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    if(buffer_capacity(channel->buffer) == buffer_current_size(channel->buffer)){
        pthread_mutex_unlock(&channel->mutex);
        return CHANNEL_FULL;
    }

    buffer_add(channel->buffer,data);
    pthread_cond_signal(&channel->receive);

    //wake up all the CV in the list
    list_node_t *iterator;
    iterator = list_begin(channel->selectcond_list);
    while(iterator){
        pthread_cond_signal(iterator->data);
        iterator = list_next(iterator);
    }

    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function’s input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);

    // check if the channel is closed
    if(channel->flag == 0){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }

    if(buffer_current_size(channel->buffer) == 0){
        pthread_mutex_unlock(&channel->mutex);
        return CHANNEL_EMPTY;
    }

    buffer_remove(channel->buffer,data);
    pthread_cond_signal(&channel->send);

    //wake up all the CV in the list
    list_node_t *iterator;
    iterator = list_begin(channel->selectcond_list);
    while(iterator){
        pthread_cond_signal(iterator->data);
        iterator = list_next(iterator);
    }

    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// GEN_ERROR in any other error case
enum channel_status channel_close(channel_t* channel)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);

    if(channel->flag == 0){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    channel->flag = 0;
    pthread_cond_broadcast(&channel->send);
    pthread_cond_broadcast(&channel->receive);


    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GEN_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);

    if(channel->flag == 1){
        pthread_mutex_unlock(&channel->mutex);
        return DESTROY_ERROR;
    }
    else{
        buffer_free(channel->buffer);
        pthread_mutex_unlock(&channel->mutex);
        pthread_cond_destroy(&channel->receive);
        pthread_cond_destroy(&channel->send);

        //destroy the list
        list_destroy(channel->selectcond_list);

        pthread_mutex_destroy(&channel->mutex);
        free(channel);
        return SUCCESS;
    }
    return GEN_ERROR;
}

// Takes an array of channels, channel_list, of type select_t and the array length, channel_count, as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum channel_status channel_select(select_t* channel_list, size_t channel_count, size_t* selected_index)
{
    /* IMPLEMENT THIS */
    //create a lock for this function alone
    pthread_cond_t select;
    pthread_mutex_t selectMutex;
    pthread_mutex_init(&selectMutex,NULL);
    pthread_cond_init(&select,NULL);

    pthread_mutex_lock(&selectMutex);


    //insert one CV to all channel
    for(size_t y = 0; y < channel_count; y++){
        pthread_mutex_lock(&channel_list[y].channel->mutex);
        list_insert(channel_list[y].channel->selectcond_list,&select);
        pthread_mutex_unlock(&channel_list[y].channel->mutex);
    }


    while(1){
        for (size_t x = 0; x < channel_count; x++)
        {
            //keep track of the index of current channel
            *selected_index = x;        
            pthread_mutex_lock(&channel_list[x].channel->mutex);

            //handle this SEND operation
            if(channel_list[x].dir == SEND){

                //if the channel is closed return error
                if(channel_list[x].channel->flag == 0){
                    pthread_mutex_unlock(&channel_list[x].channel->mutex);
                    pthread_mutex_unlock(&selectMutex);

                    //remove one CV from all channel
                    for(size_t z = 0; z < channel_count; z++){
                        pthread_mutex_lock(&channel_list[z].channel->mutex);
                        list_remove(channel_list[z].channel->selectcond_list,list_find(channel_list[z].channel->selectcond_list,&select));
                        pthread_mutex_unlock(&channel_list[z].channel->mutex);
                    }

                    return CLOSED_ERROR;
                }
                //if the oepration cannot be performed
                if(buffer_capacity(channel_list[x].channel->buffer) == buffer_current_size(channel_list[x].channel->buffer)){
                    pthread_mutex_unlock(&channel_list[x].channel->mutex);

                    pthread_mutex_unlock(&selectMutex);

                }
                else{
                    buffer_add(channel_list[x].channel->buffer,channel_list[x].data);
                    pthread_cond_signal(&channel_list[x].channel->receive);
                    pthread_mutex_unlock(&channel_list[x].channel->mutex);   
                    pthread_mutex_unlock(&selectMutex);


                    //remove one CV from all channel
                    for(size_t z = 0; z < channel_count; z++){
                        pthread_mutex_lock(&channel_list[z].channel->mutex);
                        list_remove(channel_list[z].channel->selectcond_list,list_find(channel_list[z].channel->selectcond_list,&select));
                        pthread_mutex_unlock(&channel_list[z].channel->mutex);
                    }


                    return SUCCESS;  
                }
            }


            //handle the RECV operation
            else{
                //if the channel is closed return error
                if(channel_list[x].channel->flag == 0){
                    pthread_mutex_unlock(&channel_list[x].channel->mutex);

                    pthread_mutex_unlock(&selectMutex);

                    //remove one CV from all channel
                    for(size_t z = 0; z < channel_count; z++){
                        pthread_mutex_lock(&channel_list[z].channel->mutex);
                        list_remove(channel_list[z].channel->selectcond_list,list_find(channel_list[z].channel->selectcond_list,&select));
                        pthread_mutex_unlock(&channel_list[z].channel->mutex);
                    }

                    
                    return CLOSED_ERROR;
                }

                //if the operation cannot be performed
                if(buffer_current_size(channel_list[x].channel->buffer) == 0){
                    pthread_mutex_unlock(&channel_list[x].channel->mutex);

                    pthread_mutex_unlock(&selectMutex);

                }
                else {
                    buffer_remove(channel_list[x].channel->buffer, &(channel_list[x].data));
                    pthread_cond_signal(&channel_list[x].channel->send);

                    //pthread_mutex_unlock(&selectMutex);
                    pthread_mutex_unlock(&channel_list[x].channel->mutex);
                    pthread_mutex_unlock(&selectMutex);


                    //remove one CV from all channel
                    for(size_t z = 0; z < channel_count; z++){
                        pthread_mutex_lock(&channel_list[z].channel->mutex);
                        list_remove(channel_list[z].channel->selectcond_list,list_find(channel_list[z].channel->selectcond_list,&select));
                        pthread_mutex_unlock(&channel_list[z].channel->mutex);
                    }


                    return SUCCESS;
                }
            }
        } 
        pthread_cond_wait(&select,&selectMutex);
    }
    return GEN_ERROR;
}
