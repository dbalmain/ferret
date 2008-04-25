#ifndef FRT_PRIORITYQUEUE_H
#define FRT_PRIORITYQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

typedef bool (*frt_lt_ft)(const void *p1, const void *p2);

/**
 * A PriorityQueue has a fixed size and contains a less_than function and a
 * free_elem function specific to the data type to be stored in the queue.
 */
typedef struct FrtPriorityQueue
{
    int size;
    int capa;
    int mem_capa;
    void **heap;
    frt_lt_ft less_than_i;
    frt_free_ft free_elem_i;
} FrtPriorityQueue;

/**
 * Create a new PriorityQueue setting the less_than and free_elem for this
 * specific PriorityQueue.
 *
 * @param capa the capacity of the PriorityQueue. As more than the capacity is
 *   added to the queue the least valued elements drop off the bottom.
 * @param less_than the function to determine whether one value is less than
 *   another for this particular PriorityQueue
 * @param free_elem the function to free the elements in the PriorityQueue
 *   when it is destroyed or there is insertion overflow
 * @return a newly allocated PriorityQueue
 */
extern FrtPriorityQueue *frt_pq_new(int capa,
                                       frt_lt_ft less_than,
                                       frt_free_ft free_elem);

/**
 * Allocate a clone of the PriorityQueue. This can be used if you want to scan
 * through all elements of the PriorityQueue but you don't want to have to
 * remove the all and add them all again.
 *
 * @param pq the priority queue to clone
 * @return a clone of the original priority queue
 */
extern FrtPriorityQueue *frt_pq_clone(FrtPriorityQueue *pq);

/**
 * Clear all elements from the PriorityQueue and reset the size to 0. When
 * the elements are removed from the PriorityQueue, free_elem is used to free
 * them, unless it was set to NULL in which case nothing will happen to them.
 *
 * @param self the PriorityQueue to clear
 */
extern void frt_pq_clear(FrtPriorityQueue *self);

/**
 * Free the memory allocated to the PriorityQueue. This function does nothing
 * to the elements in the PriorityQueue itself. To destroy them also, use
 * pq_destroy.
 *
 * @param self the PriorityQueue to free
 */
extern void frt_pq_free(FrtPriorityQueue *self);

/**
 * Destroy the PriorityQueue, freeing all memory allocated to it and also
 * destroying all the elements contained by it. This method is equivalent to
 * calling pq_clear followed by pq_free.
 *
 * @param the PriorityQueue to destroy
 */
extern void frt_pq_destroy(FrtPriorityQueue *self);

/**
 * Reorder the PriorityQueue after the top element has been modified. This
 * method is used especially when the PriorityQueue contains a queue of
 * iterators. When the top iterator is incremented you should call this
 * method.
 *
 * @param self the PriorityQueue to reorder
 */
extern void frt_pq_down(FrtPriorityQueue *self);

/**
 * Add another element to the PriorityQueue. This method should only be used
 * when the PriorityQueue has enough space allocated to hold all elements
 * added. If there is a chance that you will add more than the amount you have
 * allocated then you should use pq_insert. pq_insert will handle insertion
 * overflow.
 *
 * @param self the PriorityQueue to add the element to
 * @param elem the element to add to the PriorityQueue
 */
extern void frt_pq_push(FrtPriorityQueue *self, void *elem);

typedef enum {
    FRT_PQ_DROPPED = 0,
    FRT_PQ_ADDED,
    FRT_PQ_INSERTED
} FrtPriorityQueueInsertEnum;

/**
 * Add another element to the PriorityQueue. Unlike pq_push, this method
 * handles insertion overflow. That is, when you insert more elements than the
 * capacity of the PriorityQueue, the elements are dropped off the bottom and
 * freed using the free_elem function.
 *
 * @param self the PriorityQueue to add the element to
 * @param elem the element to add to the PriorityQueue
 * @returns one of three values;
 *   <pre>
 *     0 == PQ_DROPPED  the element was too small (according to the less_than
 *                      function) so it was destroyed
 *     1 == PQ_ADDED    the element was successfully added
 *     2 == PQ_INSERTED the element was successfully added after another
 *                      element was dropped and destroyed
 *   </pre>
 */
extern FrtPriorityQueueInsertEnum frt_pq_insert(FrtPriorityQueue *self,
                                                   void *elem);

/**
 * Get the top element in the PriorityQueue.
 *
 * @param self the PriorityQueue to get the top from
 * @return the top element in the PriorityQueue
 */
extern void *frt_pq_top(FrtPriorityQueue *self);

/**
 * Remove and return the top element in the PriorityQueue.
 *
 * @param self the PriorityQueue to get the top from
 * @return the top element in the PriorityQueue
 */
extern void *frt_pq_pop(FrtPriorityQueue *self);

/**
 * Return true if the PriorityQueue is full.
 *
 * @param self the PriorityQueue to test
 * @return true if the PriorityQueue is full.
 */
#define frt_pq_full(pq) ((pq)->size == (pq)->capa)

#ifdef __cplusplus
} // extern "C"
#endif

#endif
