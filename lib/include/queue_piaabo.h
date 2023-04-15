#ifndef __QUEUE_PIAABO
#define __QUEUE_PIAABO

#include "stdlib.h"
#include "util_piaabo.h"


/* function pointers */
typedef void free_fn_pointer(void *ptr); // void (*free_fn)(void *)

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/** __queue_item_t:
 * A queue item is a node in the queue.
 * 
 * [__up] contains a pointer to the next __queue_item_t.
 * [__down] contains a pointer to the previous __queue_item_t.
 * [__data] contains a pointer to the data.
 * [__free_data_fn] contains a pointer to a function to free the data.
 */
typedef struct queue_item_t {
  void *data;                    /* pointer to the data item */
  size_t __data_size;             /* size of the data item */
  struct queue_item_t *__up;      /* pointer to the next queue item */
  struct queue_item_t *__down;    /* pointer to the previous queue item */
  free_fn_pointer *__free_data_fn;/* pointer to a function to free the __queue_item->data */
} __queue_item_t;
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/** __queue_t:
 * [__load_size] contains the total number of items in the queue.
 * [__load_index] contains the current index of the queue.
 * [__head] contains a pointer to the current __queue_item_t.
 * 
 * The queue can hold a colection for any and multiple data types. 
 * 
 * The Head can Go Up or Go Down.
 *    [__down] <-- [__head] --> [__up]
 * Going Down and then going Up retrives the initial Head.
 *    [__down->__up] --> [__head]
 * Going Up and then going Down retrives the initial Head.
 *    [__up->__down] --> [__head]
 * On the last element the __up pointer is NULL.
 *    [__up,__up,__up,...,__up]  --> [NULL]
 * On the first element the __down pointer is NULL.
 *    [__down,__down,...,__down] --> [NULL]
 * 
 * [__head] contains a pointer to the current __queue_item_t.
 * [__head->__up] contains a pointer to the next __queue_item_t.
 * [__head->__down] contains a pointer to the previous __queue_item_t.
 * 
 * __queue_item_t contains the data and a convenient pointer free the data.
 */
typedef struct queue_t {
  int load_size;                  /* total items count in the queue */
  int load_index;                 /* current index (starts at zero) ends at load_size-1 */
  __queue_item_t *__head;    /* pointer to the current (head) __queue_item */
} __queue_t;
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
static inline bool queue_is_empty(__queue_t *Q);
static inline bool queue_throw_unhealty(const char *msg);
static inline bool queue_is_healty(__queue_t *Q);
static inline __queue_item_t *queue_item_fabric(void *data, size_t data_size, free_fn_pointer *free_fn);
static void queue_item_destructor(__queue_item_t *item);
static inline __queue_item_t *queue_to_next(__queue_t *Q);
static inline __queue_item_t *queue_to_back(__queue_t *Q);
static inline __queue_item_t *queue_to_index(__queue_t *Q, int index);
static inline void queue_to_top(__queue_t *Q);
static inline void queue_to_base(__queue_t *Q);
static inline void queue_insert_item_on_top(__queue_t *Q, void *data, size_t data_size, free_fn_pointer *free_fn);
static inline void queue_insert_item_on_base(__queue_t *Q, void *data, size_t data_size, free_fn_pointer *free_fn);
static __queue_t *queue_fabric();
static void queue_destructor(__queue_t *Q);
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method to determine if __queue_t is empty */
static inline bool queue_is_empty(__queue_t *Q){
  return Q->load_size == 0 ? true : false;
}
/* method to throw case when __queue_t is not haelty */
static inline bool queue_throw_unhealty(const char *msg){
  log_warn(msg);
  return false;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method to determine if __queue_t is haelty */
static inline bool queue_is_healty(__queue_t *Q){
  if(Q == NULL) /* queue is null */
    return queue_throw_unhealty("Unhealty queue: queue == NULL\n");
  if(queue_is_empty(Q)){ /* queue is empty */
    if(Q->load_size != 0)
      return queue_throw_unhealty("Unhealty queue: [empty queue] with load_size != 0\n");
    if(Q->load_index != -1)
      return queue_throw_unhealty("Unhealty queue: [empty queue] with load_index != -1\n");
    if (Q->__head != NULL)
      return queue_throw_unhealty("Unhealty queue: [empty queue] with __head != NULL\n");
  } else { /*queue is not empty */
    if(Q->load_size < 1)
      return queue_throw_unhealty("Unhealty queue: [unempty queue] load_size < 1\n");
    if(Q->load_index < 0)
      return queue_throw_unhealty("Unhealty queue: [unempty queue] load_index < 0\n");
    if(Q->load_index >= Q->load_size)
      return queue_throw_unhealty("Unhealty queue: [unempty queue] load_index >= load_size\n");
    /* guard the current index */
    int current_index = Q->load_index;
    /* start check from the base */
    queue_to_base(Q);
    /* check the base __down is NULL */
    if(Q->__head->__down != NULL)
      return queue_throw_unhealty("Unhealty queue: [unempty queue] on base __down is not NULL\n");
    while(queue_to_next(Q) != NULL){
      /* check the __down pointer __up points to the head pointer */
      if(Q->__head->__down->__up != Q->__head)
        return queue_throw_unhealty("Unhealty queue: [unempty queue] Q->__head->__down->__up != Q->__head\n");
      /* check the __up pointer __down points to the head pointer */
      if(Q->__head->__up != NULL)
        if(Q->__head->__up->__down != Q->__head)
          return queue_throw_unhealty("Unhealty queue: [unempty queue] Q->__head->__up->__down != Q->__head\n");
    }
    /* check the top __up is NULL */
    if(Q->__head->__up != NULL)
      return queue_throw_unhealty("Unhealty queue: [unempty queue] on top __up is not NULL\n");

    /* return to the current index */
    queue_to_index(Q, current_index);
  }
  return true;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method fabric for queue item */
static inline __queue_item_t *queue_item_fabric(void *data, size_t data_size, free_fn_pointer *free_fn){
  __queue_item_t *new_queue_item = (__queue_item_t*) malloc(sizeof(__queue_item_t));
  if(new_queue_item == NULL)
    log_fatal("Unable to allocate memory for a new queue item\n");
  new_queue_item->data = data;
  new_queue_item->__data_size = data_size;
  new_queue_item->__up = NULL;
  new_queue_item->__down = NULL;
  new_queue_item->__free_data_fn = free_fn;
  
  return new_queue_item;
}
/* method clear queue item */
static void queue_item_destructor(__queue_item_t *item){
  if(item==NULL || item->data == NULL)
    return;
  /* free the data if required */
  if(item->__free_data_fn != NULL)
    item->__free_data_fn(item->data);
  /* free the item */
  free(item);
}
/* method to retrive the next __queue_item, returns NULL when head at top */
static inline __queue_item_t *queue_to_next(__queue_t *Q){
  if(Q == NULL || Q->__head == NULL || Q->__head->__up == NULL)
    return NULL;
  Q->load_index++;
  Q->__head = Q->__head->__up;
  
  return Q->__head;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method to retrive the next __queue_item, returns NULL when head at base */
static inline __queue_item_t *queue_to_back(__queue_t *Q){
  if(Q == NULL || Q->__head == NULL || Q->__head->__down == NULL)
    return NULL;
  Q->load_index--;
  Q->__head = Q->__head->__down;
  
  return Q->__head;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method to retrive a __queue_item by index, returns NULL if the index is not in range */
static inline __queue_item_t *queue_to_index(__queue_t *Q, int index){
  if(Q == NULL || Q->__head == NULL)
    return NULL;
  if(index < 0 || index >= Q->load_size)
    return NULL;
  /* navigate the head to the index */
  while(abs(Q->load_index - index) != 0)
    if(index < Q->load_index)
      queue_to_back(Q);
    else 
      queue_to_next(Q);
  
  return Q->__head;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method for taking a queue to the top, the top means the last element of the queue */
static inline void queue_to_top(__queue_t *Q){
  while(queue_to_next(Q) != NULL);
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method for taking a queue to the base, the base means the start of the queue */
static inline void queue_to_base(__queue_t *Q){
  while(queue_to_back(Q) != NULL);
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method for appending __queue_item_t to __queue_t top */
static inline void queue_insert_item_on_top(__queue_t *Q, void *data, size_t data_size, free_fn_pointer *free_fn){
  /* go to the top */
  queue_to_top(Q);
  /* create new queue item */
  __queue_item_t *new_item = queue_item_fabric(data, data_size, free_fn);
  /* assing the item to the queue */
  if(queue_is_empty(Q)){
    Q->__head = new_item;
  } else { /* avoiding assertions is correct becouse the queue is healty */
    Q->__head->__up = new_item;
    new_item->__down = Q->__head;
    Q->__head = new_item;
  }
  /* increment the queue size and index */
  Q->load_size ++;
  Q->load_index ++;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* method for appending __queue_item_t to __queue_t base */
static inline void queue_insert_item_on_base(__queue_t *Q, void *data, size_t data_size, free_fn_pointer *free_fn){
  /* go to the base */
  queue_to_base(Q);
  /* create new queue item */
  __queue_item_t *new_item = queue_item_fabric(data, data_size, free_fn);
  /* assing the item to the queue */
  if(queue_is_empty(Q)){
    Q->__head = new_item;
  } else { /* avoiding assertions is correct becouse the queue is healty */
    Q->__head->__down = new_item;
    new_item->__up = Q->__head;
    Q->__head = new_item;
  }
  /* increment only the queue size */
  Q->load_size ++;
}
/* fabric for a general porpouse queue */
static __queue_t *queue_fabric(){
  log_warn("Missing queue mutex for a thread safe implementation\n");
  /* allocate memory for the queue */
  __queue_t *new_queue = (__queue_t*)malloc(sizeof(__queue_t));
  if(new_queue == NULL)
    log_fatal("Unable to allocate memory for a new queue\n");
  /* initialize the queue values */
  new_queue->load_size = 0;
  new_queue->load_index = -1;
  new_queue->__head = NULL;
  /* assert the queue is healty */
  ASSERT(queue_is_healty(new_queue));
  
  /* return the new queue */
  return new_queue;
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
/* destructor for a general porpouse queue */
static void queue_destructor(__queue_t *Q){
  if(Q==NULL)
    return;
  /* assert the queue is healty */
  ASSERT(queue_is_healty(Q));
  /* go to the top */
  queue_to_top(Q);
  /* free all the items */
  while(queue_to_back(Q) != NULL)
    queue_item_destructor(Q->__head->__up);
  if(Q->__head!=NULL)
    queue_item_destructor(Q->__head);
  /* free the queue */
  free(Q);
}
/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */
#endif