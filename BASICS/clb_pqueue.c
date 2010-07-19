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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


