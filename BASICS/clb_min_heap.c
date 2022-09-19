/*-----------------------------------------------------------------------

  File  : clb_min_heap.c

  Author: Petar Vukmirovic

  Contents

  Simple minimum heap implementation.

  Copyright 2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jan 4 14:29:14 CET 2022

-----------------------------------------------------------------------*/

#include "clb_min_heap.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#define CALL_SETTER(h, el, idx) if (UNLIKELY((h)->setter)) (h)->setter(el, idx)
#define PARENT(idx) (((idx)-1)/2)
#define IS_ROOT(idx) ((idx) <= 0)
#define LEFT(idx) (2*(idx)+1)
#define RIGHT(idx) (LEFT(idx)+1)
#define IS_LEAF(idx, size) (LEFT(idx) >= (size))

/*-----------------------------------------------------------------------
//
// Function: bubble_up()
//
//   If an element at child_idx was just inserted or its value has been
//   decreased then bring the element up as necessary.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void bubble_up(MinHeap_p h, PStackPointer child_idx)
{
   while(!IS_ROOT(child_idx))
   {
      PStackPointer parent_idx = PARENT(child_idx);
      IntOrP* child_ref = PStackElementRef(h->arr, child_idx);
      IntOrP* parent_ref = PStackElementRef(h->arr, parent_idx);
      if(h->cmp(child_ref, parent_ref) < 0)
      {
         CALL_SETTER(h, child_ref->p_val, parent_idx);
         CALL_SETTER(h, parent_ref->p_val, child_idx);
         SWAP(*child_ref, *parent_ref);

         child_idx = parent_idx;
      }
      else
      {
         child_idx = 0;
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: bubble_up()
//
//   If an element at child_idx was just increased in value, drop it down
//   to its position in heap.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void drop_down(MinHeap_p h, PStackPointer curr_idx)
{
   PStackPointer size = PStackGetSP(h->arr);
   while(!IS_LEAF(curr_idx, size))
   {
      PStackPointer min_c_idx = curr_idx;
      PStackPointer l = LEFT(curr_idx);
      PStackPointer r = RIGHT(curr_idx);
      if(h->cmp(PStackElementRef(h->arr, l),
                PStackElementRef(h->arr, min_c_idx)) < 0)
      {
         min_c_idx = l;
      }
      if(r < size &&
         h->cmp(PStackElementRef(h->arr, r),
                PStackElementRef(h->arr, min_c_idx)) < 0)
      {
         min_c_idx = r;
      }

      if(min_c_idx != curr_idx)
      {
         IntOrP* parent_ref = PStackElementRef(h->arr, curr_idx);
         IntOrP* child_ref = PStackElementRef(h->arr, min_c_idx);

         CALL_SETTER(h, parent_ref->p_val, min_c_idx);
         CALL_SETTER(h, child_ref->p_val, curr_idx);
         SWAP(*child_ref, *parent_ref);

         curr_idx = min_c_idx;
      }
      else
      {
         curr_idx = size; // breaking
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: add()
//
//   Internal function for inserting a key.
//
// Global Variables: -
//
// Side Effects    : Output
//
/-----------------------------------------------------------------------*/

void add(MinHeap_p h, IntOrP key)
{
   push(h->arr, key);
   CALL_SETTER(h, key.p_val, PStackGetTopSP(h->arr));
   bubble_up(h, PStackGetTopSP(h->arr));
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: MinHeapAllocWithIndex()
//
//   Allocate and initialize a min heap. Setter function is used to
//   notify calling code that the index of stored element is changed.
//   Setter is only necessary if we want heap to be able to increase/
//   decrease a key.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

MinHeap_p MinHeapAllocWithIndex(CmpFun cmp, SetIndexFun setter)
{
   MinHeap_p h = SizeMalloc(sizeof(MinHeapCell));
   h->arr = PStackAlloc();
   h->cmp = cmp;
   h->setter = setter;
   return h;
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapSize()
//
//   Number of stored elements in the heap.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

long MinHeapSize(MinHeap_p h)
{
   return PStackGetSP(h->arr);
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapAddP()
//
//   Add a pointer to heap. O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void MinHeapAddP(MinHeap_p h, void* p)
{
   IntOrP key;
   key.p_val = p;
   add(h, key);
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapAddInt()
//
//   Add an integer to heap. O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void MinHeapAddInt(MinHeap_p h, long i)
{
   IntOrP key;
   key.i_val = i;
   add(h, key);
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapPopMin()
//
//   Pop the maximum element and restore heap property. O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

IntOrP MinHeapPopMin(MinHeap_p h)
{
   if(MinHeapSize(h))
   {
      IntOrP ret_val = PStackElement(h->arr, 0);
      IntOrP last = PStackPop(h->arr);
      if(PStackGetSP(h->arr))
      {
         *PStackElementRef(h->arr, 0) = last;
         CALL_SETTER(h, last.p_val, 0);
         drop_down(h, 0);
      }
      CALL_SETTER(h, ret_val.p_val, -1);
      return ret_val;
   }
   else
   {
      SysError("Trying to get an element from an empty heap", -1);
      return (IntOrP){.p_val = NULL};
   }
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapDecrKey()
//
//   Notify that the key assigned to the idx has (possibly) been decreased. O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void MinHeapDecrKey(MinHeap_p h, long idx)
{
   drop_down(h, idx);
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapIncrKey()
//
//   Notify that the key assigned to the idx has (possibly) been increased. O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void MinHeapIncrKey(MinHeap_p h, long idx)
{
   bubble_up(h, idx);
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapFree()
//
//   Deallocate the space allocated for junk.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void MinHeapFree(MinHeap_p junk)
{
   PStackFree(junk->arr);
   SizeFree(junk, sizeof(MinHeapCell));
}

/*-----------------------------------------------------------------------
//
// Function: DBGPrintHeap()
//
//   Print the contents of the heap. If as_ptr is true, then the heap
//   is interpreted as heap of pointers. O(n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DBGPrintHeap(FILE* out, MinHeap_p h, bool as_ptr)
{
   for(PStackPointer i=0; i<PStackGetSP(h->arr); i++)
   {
      if(as_ptr)
      {
         fprintf(out, "%p; ", PStackElementP(h->arr, i));
      }
      else
      {
         fprintf(out, "%ld; ", PStackElementInt(h->arr, i));
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapUpdateElement()
//
//   When the value of an element has been updated, then fix its position
//   inside the heap by reruning comparison function on corresponding nodes.
//   * If the element becomes smaller than parent -- than bubble the node up
//   * Else -- try sifting the node down (this will succeed only if the node
//    increased in value)
//   O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/
void MinHeapUpdateElement(MinHeap_p h, long idx)
{
   if(idx >= 0)
   {
      // if element is in the heap
      if(!IS_ROOT(idx)
         && h->cmp(PStackElementRef(h->arr, idx),
                   PStackElementRef(h->arr, PARENT(idx))) < 0)
      {
         bubble_up(h, idx);
      }
      else
      {
         drop_down(h, idx);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: MinHeapUpdateElement()
//
//   Assuming the index is within the heap, the function removes
//   an element with index idx from the heap. O(log n)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/
void MinHeapRemoveElement(MinHeap_p h, long idx)
{
   assert(idx >=0 && idx < PStackGetSP(h->arr));
   CALL_SETTER(h, PStackElementP(h->arr, idx), -1);
   IntOrP last = PStackPop(h->arr);
   if(idx != PStackGetSP(h->arr))
   {
      CALL_SETTER(h, last.p_val, idx);
      *PStackElementRef(h->arr, idx) = last;
      MinHeapUpdateElement(h, idx);
   }
}

// code meant for testing
// ProblemType problemType  = PROBLEM_NOT_INIT;

// int cmp(IntOrP* ia, IntOrP* ib)
// {
//    return CMP(ia->i_val, ib->i_val);
// }

// int main(int argc, char* argv[])
// {
//    int test_arr[] = {5, -1, 4, 10, 20, 12, 8, 99, 1};
//    int test_arr_size = sizeof(test_arr) / sizeof(int);
//    MinHeap_p h = MinHeapAlloc(cmp);

//    for(int i=0; i< test_arr_size; i++)
//    {
//       MinHeapAddInt(h, test_arr[i]);
//    }

//    DBGPrintHeap(stderr, h, false);
//    fprintf(stderr, ";; \n ");

//    while(MinHeapSize(h))
//    {
//       fprintf(stderr, "%ld; ", MinHeapPopMinInt(h));
//    }
//    fprintf(stderr, "\n");
//    MinHeapFree(h);
//    return 0;
// }


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
