/*-----------------------------------------------------------------------

File  : clb_pqueues.c

Author: Stephan Schulz

Contents

  LIFO-Lists of pointers and (long) integers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jun 30 17:34:19 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "clb_pqueue.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PQueueGrow()
//
//   Increase the size of queue.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void PQueueGrow(PQueue_p queue)
{
   long   new_size, i;
   IntOrP *new_mem;

   new_size = queue->size*2;
   new_mem  = SizeMalloc(new_size*sizeof(IntOrP));

   for(i=0; i<queue->head; i++)
   {
      new_mem[i] = queue->queue[i];
   }
   for(i=queue->head; i<queue->size; i++)
   {
      new_mem[i+queue->size] = queue->queue[i];
   }
   queue->tail+= queue->size;
   SizeFree(queue->queue, queue->size*sizeof(IntOrP));
   queue->queue = new_mem;
   queue->size  = new_size;
}


/*-----------------------------------------------------------------------
//
// Function: PQueueCardinality()
//
//   Return the number of elements in the queue.
//
// Global Variables: -
//
// Side Effects    : -
/
/----------------------------------------------------------------------*/

long PQueueCardinality(PQueue_p queue)
{
   long res;
   if(queue->head>=queue->tail)
   {
      res = queue->head-queue->tail;
   }
   else
   {
      res = queue->head + (queue->size - queue->tail);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PQueueElement()
//
//   Retutn the entry at absolute index index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IntOrP PQueueElement(PQueue_p queue, long index)
{
   return queue->queue[index];
}


/*-----------------------------------------------------------------------
//
// Function: PQueueTailIndex()
//
//   Return the index of the tail (oldest, last) element (or -1 if the
//   queue is empty).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long PQueueTailIndex(PQueue_p queue)
{
   if(PQueueEmpty(queue))
   {
      return -1;
   }
   return queue->tail;
}

/*-----------------------------------------------------------------------
//
// Function: PQueueIncIndex()
//
//   Given an index to a (used) element in the queue, return a similar
//   index to to next element (or -1 if there is no next element).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long PQueueIncIndex(PQueue_p queue, long index)
{
   index = (index+1) % queue->size;
   if(index == queue->head)
   {
      return -1;
   }
   return index;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


