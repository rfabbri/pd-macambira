/* pmutil.h -- some helpful utilities for building midi 
               applications that use PortMidi 
 */

typedef void PmQueue;

/*
    A single-reader, single-writer queue is created by
    Pm_QueueCreate(), which takes the number of messages and
    the message size as parameters. The queue only accepts
    fixed sized messages. Returns NULL if memory cannot be allocated.

    Pm_QueueDestroy() destroys the queue and frees its storage.
 */

PmQueue *Pm_QueueCreate(long num_msgs, long bytes_per_msg);
PmError Pm_QueueDestroy(PmQueue *queue);

/* 
    Pm_Dequeue() removes one item from the queue, copying it into msg.
    Returns 1 if successful, and 0 if the queue is empty.
    Returns pmBufferOverflow and clears the overflow flag if 
    the flag is set.
 */
PmError Pm_Dequeue(PmQueue *queue, void *msg);


/*
    Pm_Enqueue() inserts one item into the queue, copying it from msg.
    Returns pmNoError if successful and pmBufferOverflow if the queue was 
    already full. If pmBufferOverflow is returned, the overflow flag is set.
 */
PmError Pm_Enqueue(PmQueue *queue, void *msg);


/*
    Pm_QueueFull() returns non-zero if the queue is full
    Pm_QueueEmpty() returns non-zero if the queue is empty

    Either condition may change immediately because a parallel
    enqueue or dequeue operation could be in progress.
 */
int Pm_QueueFull(PmQueue *queue);
#define Pm_QueueEmpty(m) (m->head == m->tail)
