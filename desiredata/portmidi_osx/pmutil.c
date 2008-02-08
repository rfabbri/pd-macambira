/* pmutil.c -- some helpful utilities for building midi
               applications that use PortMidi
 */
#include "stdlib.h"
#include "memory.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"


PmQueue *Pm_QueueCreate(long num_msgs, long bytes_per_msg)
{
    PmQueueRep *queue = (PmQueueRep *) malloc(sizeof(PmQueueRep));
    if (!queue) return NULL;
    queue->len = num_msgs * bytes_per_msg;
    queue->buffer = malloc(queue->len);
    if (!queue->buffer) {
        free(queue);
        return NULL;
    }
    queue->head = 0;
    queue->tail = 0;
    queue->msg_size = bytes_per_msg;
    queue->overflow = FALSE;
    return queue;
}


PmError Pm_QueueDestroy(PmQueue *q)
{
    PmQueueRep *queue = (PmQueueRep *) q;
    if (!queue || !queue->buffer) return pmBadPtr;
    free(queue->buffer);
    free(queue);
    return pmNoError;
}


PmError Pm_Dequeue(PmQueue *q, void *msg)
{
    long head;
    PmQueueRep *queue = (PmQueueRep *) q;
    if (queue->overflow) {
        queue->overflow = FALSE;
        return pmBufferOverflow;
    }
    head = queue->head; /* make sure this is written after access */
    if (head == queue->tail) return 0;
    memcpy(msg, queue->buffer + head, queue->msg_size);
    head += queue->msg_size;
    if (head == queue->len) head = 0;
    queue->head = head;
    return 1; /* success */
}


/* source should not enqueue data if overflow is set */
/**/
PmError Pm_Enqueue(PmQueue *q, void *msg)
{
    PmQueueRep *queue = (PmQueueRep *) q;
    long tail = queue->tail;
    memcpy(queue->buffer + tail, msg, queue->msg_size);
    tail += queue->msg_size;
    if (tail == queue->len) tail = 0;
    if (tail == queue->head) {
        queue->overflow = TRUE;
        /* do not update tail, so message is lost */
        return pmBufferOverflow;
    }
    queue->tail = tail;
    return pmNoError;
}


int Pm_QueueFull(PmQueue *q)
{
    PmQueueRep *queue = (PmQueueRep *) q;
    long tail = queue->tail;
    tail += queue->msg_size;
    if (tail == queue->len) {
        tail = 0;
    }
    return (tail == queue->head);
}

