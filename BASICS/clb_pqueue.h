/*-----------------------------------------------------------------------

File  : clb_pqueue.h

Author: Stephan Schulz

Contents

  Functions for LIFO-lists.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jun 30 17:14:42 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_PQUEUE

#define CLB_PQUEUE


#include <clb_memory.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define PQUEUE_DEFAULT_SIZE 128 /* Queues grow exponentially (and never
                                   shrink unless explicitly freed) -
                                   take care */

typedef struct pqueuecell
{
   long   size;   /* Of allocateted memory */
   long   head;   /* Where the next element will be put */
   long   tail;   /* Where the next element will come from */
   IntOrP *queue; /* Memory */
}PQueueCell, *PQueue_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PQueueCellAlloc() (PQueueCell*)SizeMalloc(sizeof(PQueueCell))
#define PQueueCellFree(junk)         SizeFree(junk, sizeof(PQueueCell))

static inline PQueue_p PQueueAlloc(void);
static inline void     PQueueFree(PQueue_p junk);
void     PQueueGrow(PQueue_p queue);
#define  PQueueEmpty(queue) ((queue)->head == (queue)->tail)
static inline void     PQueueReset(PQueue_p queue);

static inline void     PQueueStoreInt(PQueue_p queue, long val);
static inline void     PQueueStoreP(PQueue_p queue, void* val);

static inline void     PQueueBuryInt(PQueue_p queue, long val);
static inline void     PQueueBuryP(PQueue_p queue, void* val);

static inline IntOrP   PQueueGetNext(PQueue_p queue);
#define  PQueueGetNextInt(Queue) (PQueueGetNext(Queue).i_val)
#define  PQueueGetNextP(Queue)   (PQueueGetNext(Queue).p_val)

static inline IntOrP   PQueueGetLast(PQueue_p queue);
#define  PQueueGetLastInt(Queue) (PQueueGetLast(Queue).i_val)
#define  PQueueGetLastP(Queue)   (PQueueGetLast(Queue).p_val)

static inline IntOrP   PQueueLook(PQueue_p queue);
#define  PQueueLookInt(Queue) (PQueueLook(Queue).i_val)
#define  PQueueLookP(Queue)   (PQueueLook(Queue).p_val)
static inline IntOrP   PQueueLookLast(PQueue_p queue);
#define  PQueueLookLastInt(Queue) (PQueueLookLast(Queue).i_val)
#define  PQueueLookLastP(Queue)   (PQueueLookLast(Queue).p_val)
long     PQueueCardinality(PQueue_p queue);

IntOrP   PQueueElement(PQueue_p queue, long index);
#define  PQueueElementInt(Queue, index) (PQueueElement(Queue,index).i_val)
#define  PQueueElementP(Queue, index) (PQueueElement(Queue,index).p_val)

long     PQueueTailIndex(PQueue_p queue);
long     PQueueIncIndex(PQueue_p queue, long index);

/*---------------------------------------------------------------------*/
/*                       Inline-Functions                              */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: pqueue_store()
//
//   Put an element in the queue.
//
// Global Variables: -
//
// Side Effects    : memory operations, changes queue
//
/----------------------------------------------------------------------*/

static inline void pqueue_store(PQueue_p queue, IntOrP val)
{
   queue->queue[queue->head] = val;
   queue->head++;
   if(queue->head == queue->size)
   {
      queue->head = 0;
   }

   if(queue->head == queue->tail)
   {
      PQueueGrow(queue);
   }
}


/*-----------------------------------------------------------------------
//
// Function: pqueue_bury()
//
//   Put an element at the front of the queue (i.e. "bury" it under
//   all the other elements in a stack-view of the queue).
//
// Global Variables: -
//
// Side Effects    : memory operations, changes queue
//
/----------------------------------------------------------------------*/

static inline void pqueue_bury(PQueue_p queue, IntOrP val)
{
   queue->tail = queue->tail? (queue->tail-1):queue->size-1;
   queue->queue[queue->tail] = val;

   if(queue->head == queue->tail)
   {
      PQueueGrow(queue);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PQueueAlloc()
//
//   Allocate an empty, initialized Queue.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline PQueue_p PQueueAlloc(void)
{
   PQueue_p handle = PQueueCellAlloc();

   handle->size  = PQUEUE_DEFAULT_SIZE;
   handle->head  = 0;
   handle->tail  = 0;
   handle->queue = SizeMalloc(PQUEUE_DEFAULT_SIZE*sizeof(IntOrP));

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PQueueFree()
//
//   Free a Queue.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline void PQueueFree(PQueue_p junk)
{
   SizeFree(junk->queue, junk->size*sizeof(IntOrP));
   PQueueCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: PQueueReset()
//
//   Reset a queue to empty state.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void PQueueReset(PQueue_p queue)
{
   queue->head = 0;
   queue->tail = 0;
}


/*-----------------------------------------------------------------------
//
// Function: PQueueStoreInt()
//
//   Store an integer in the queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void PQueueStoreInt(PQueue_p queue, long val)
{
   IntOrP handle;

   handle.i_val = val;
   pqueue_store(queue, handle);
}

/*-----------------------------------------------------------------------
//
// Function: PQueueStoreP()
//
//   Store a pointer in the queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void PQueueStoreP(PQueue_p queue, void* val)
{
   IntOrP handle;

   handle.p_val = val;
   pqueue_store(queue, handle);
}

/*-----------------------------------------------------------------------
//
// Function: PQueueBuryInt()
//
//   Store an integer at the front of the the queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void PQueueBuryInt(PQueue_p queue, long val)
{
   IntOrP handle;

   handle.i_val = val;
   pqueue_bury(queue, handle);
}

/*-----------------------------------------------------------------------
//
// Function: PQueueBuryP()
//
//   Store a pointer at the front of the queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void PQueueBuryP(PQueue_p queue, void* val)
{
   IntOrP handle;

   handle.p_val = val;
   pqueue_bury(queue, handle);
}


/*-----------------------------------------------------------------------
//
// Function: PQueueGetNext()
//
//   Extract the next value from the queue and return it.
//
// Global Variables: -
//
// Side Effects    : Changes queue.
//
/----------------------------------------------------------------------*/

static inline IntOrP PQueueGetNext(PQueue_p queue)
{
   IntOrP res;

   assert(!PQueueEmpty(queue));

   res = queue->queue[queue->tail];
   queue->tail++;
   if(queue->tail == queue->size)
   {
      queue->tail = 0;
   }

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PQueueGetLast()
//
//   Extract the last value from the queue (i.e. pop from the queue
//   viewed as a stack) and return it.
//
// Global Variables: -
//
// Side Effects    : Changes queue.
//
/----------------------------------------------------------------------*/

static inline IntOrP PQueueGetLast(PQueue_p queue)
{
   IntOrP res;

   assert(!PQueueEmpty(queue));

   queue->head = queue->head ? (queue->head-1):queue->size-1;
   res = queue->queue[queue->head];

   return res;
}


/*-----------------------------------------------------------------------
//
// Function:  PQueueLook()
//
//   Return the next element from the queue without changing the
//   queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline IntOrP PQueueLook(PQueue_p queue)
{
   assert(!PQueueEmpty(queue));

   return queue->queue[queue->tail];
}



/*-----------------------------------------------------------------------
//
// Function: PQueueLookLast()
//
//   Return the last (youngest) value from the queue without modifyin
//   the queue.
//
// Global Variables: -
//
// Side Effects    : Changes queue.
//
/----------------------------------------------------------------------*/

static inline IntOrP PQueueLookLast(PQueue_p queue)
{
   IntOrP res;
   long   index;

   assert(!PQueueEmpty(queue));

   index = queue->head? (queue->head-1):queue->size-1;
   res = queue->queue[index];

   return res;
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





