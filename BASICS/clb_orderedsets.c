/*-----------------------------------------------------------------------

  File  : clb_orderedsets.c

  Author: Stephan Schulz

  Contents

  Ordered pointer sets: A splay tree (for access by key) combined with
  a cyclic doubly linked list (for iteration in insertion order) using
  the same cells. See clb_orderedsets.h for the representation.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Oct 29 2026

  -----------------------------------------------------------------------*/

#include "clb_orderedsets.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define OSLesser(p1,p2)  (((uintptr_t)(p1)) < ((uintptr_t)(p2)))
#define OSGreater(p1,p2) (((uintptr_t)(p1)) > ((uintptr_t)(p2)))
#define OSEqual(p1,p2)   (((uintptr_t)(p1)) == ((uintptr_t)(p2)))


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: splay_tree()
//
//   Perform the splay operation on tree at node with key (as in
//   clb_ptrees.c). Only the tree links are changed, never the list.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static OrdSet_p splay_tree(OrdSet_p tree, void* key)
{
   OrdSet_p   left, right, tmp;
   OrdSetCell newnode;

   if (!tree)
   {
      return tree;
   }

   newnode.lson = NULL;
   newnode.rson = NULL;
   left = &newnode;
   right = &newnode;

   for (;;)
   {
      if(OSLesser(key, tree->key))
      {
         if(!tree->lson)
         {
            break;
         }
         if(OSLesser(key, tree->lson->key))
         {
            tmp = tree->lson;
            tree->lson = tmp->rson;
            tmp->rson = tree;
            tree = tmp;
            if (!tree->lson)
            {
               break;
            }
         }
         right->lson = tree;
         right = tree;
         tree = tree->lson;
      }
      else if(OSGreater(key, tree->key))
      {
         if (!tree->rson)
         {
            break;
         }
         if(OSGreater(key, tree->rson->key))
         {
            tmp = tree->rson;
            tree->rson = tmp->lson;
            tmp->lson = tree;
            tree = tmp;
            if (!tree->rson)
            {
               break;
            }
         }
         left->rson = tree;
         left = tree;
         tree = tree->rson;
      }
      else
      {
         break;
      }
   }
   left->rson = tree->lson;
   right->lson = tree->rson;
   tree->lson = newnode.rson;
   tree->rson = newnode.lson;

   return tree;
}


/*-----------------------------------------------------------------------
//
// Function: tree_insert()
//
//   If a cell with key newnode->key exists in the tree at *root,
//   return it. Otherwise insert newnode into the tree and return
//   NULL. Splays the tree.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

static OrdSet_p tree_insert(OrdSet_p *root, OrdSet_p newnode)
{
   if(!*root)
   {
      newnode->lson = newnode->rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_tree(*root, newnode->key);

   if(OSLesser(newnode->key, (*root)->key))
   {
      newnode->lson = (*root)->lson;
      newnode->rson = *root;
      (*root)->lson = NULL;
      *root = newnode;
      return NULL;
   }
   else if(OSGreater(newnode->key, (*root)->key))
   {
      newnode->rson = (*root)->rson;
      newnode->lson = *root;
      (*root)->rson = NULL;
      *root = newnode;
      return NULL;
   }
   return *root;
}


/*-----------------------------------------------------------------------
//
// Function: tree_extract()
//
//   Find the cell with key key, remove it from the tree at *root and
//   return it. Return NULL if no such cell exists. The list links of
//   the cell are not changed.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

static OrdSet_p tree_extract(OrdSet_p *root, void* key)
{
   OrdSet_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key);
   if(OSEqual(key, (*root)->key))
   {
      if (!(*root)->lson)
      {
         x = (*root)->rson;
      }
      else
      {
         x = splay_tree((*root)->lson, key);
         x->rson = (*root)->rson;
      }
      cell = *root;
      cell->lson = cell->rson = NULL;
      *root = x;
      return cell;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: anchor_alloc()
//
//   Allocate the anchor of a new, empty set (an empty tree and an
//   empty list).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static OrdSet_p anchor_alloc(void)
{
   OrdSet_p anchor = OrdSetCellAlloc();

   anchor->lson = NULL;
   anchor->rson = NULL;
   anchor->key  = NULL;
   anchor->pred = anchor;
   anchor->succ = anchor;

   return anchor;
}


/*-----------------------------------------------------------------------
//
// Function: insert_cell()
//
//   Insert cell (with its key set) into *set. If the key is new,
//   append the cell at the end of the list and return true. If the
//   key is already in the set, return false and leave the cell to the
//   caller. Allocates the anchor if the set is empty.
//
// Global Variables: -
//
// Side Effects    : Changes the set, memory operations
//
/----------------------------------------------------------------------*/

static bool insert_cell(OrdSet_p *set, OrdSet_p cell)
{
   OrdSet_p anchor;

   if(!*set)
   {
      *set = anchor_alloc();
   }
   anchor = *set;
   if(tree_insert(&(anchor->lson), cell))
   {
      return false;
   }
   cell->pred = anchor->pred;
   cell->succ = anchor;
   anchor->pred->succ = cell;
   anchor->pred = cell;

   return true;
}


/*-----------------------------------------------------------------------
//
// Function: extract_cell()
//
//   Remove the cell with key key from *set (tree and list) and return
//   it. Return NULL if no such cell exists. If the set becomes empty,
//   the anchor is freed and *set is set to NULL.
//
// Global Variables: -
//
// Side Effects    : Changes the set, memory operations
//
/----------------------------------------------------------------------*/

static OrdSet_p extract_cell(OrdSet_p *set, void* key)
{
   OrdSet_p anchor = *set, cell;

   if(!anchor)
   {
      return NULL;
   }
   cell = tree_extract(&(anchor->lson), key);
   if(cell)
   {
      cell->pred->succ = cell->succ;
      cell->succ->pred = cell->pred;
      cell->pred = cell->succ = NULL;
      if(anchor->succ == anchor)
      {
         assert(!anchor->lson);
         OrdSetCellFree(anchor);
         *set = NULL;
      }
   }
   return cell;
}


/*-----------------------------------------------------------------------
//
// Function: tree_check()
//
//   Check that the tree at root is a search tree (keys strictly
//   increasing in in-order traversal) and that none of its cells is
//   the anchor and all have consistent list links. Return the number
//   of cells, or -1 if a check fails.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long tree_check(OrdSet_p root, OrdSet_p anchor)
{
   PStack_p stack = PStackAlloc();
   OrdSet_p handle, prev = NULL;
   long     res = 0;

   /* In-order traversal: keys must be strictly increasing */
   handle = root;
   while(handle || !PStackEmpty(stack))
   {
      while(handle)
      {
         PStackPushP(stack, handle);
         handle = handle->lson;
      }
      handle = PStackPopP(stack);
      if((handle == anchor) ||
         !handle->pred || !handle->succ ||
         (handle->pred->succ != handle) ||
         (handle->succ->pred != handle) ||
         (prev && !OSLesser(prev->key, handle->key)))
      {
         res = -1;
         break;
      }
      prev = handle;
      res++;
      handle = handle->rson;
   }
   PStackFree(stack);
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: OrdSetFree()
//
//   Free a set (all cells and the anchor, but not the objects pointed
//   to by the keys).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OrdSetFree(OrdSet_p junk)
{
   OrdSet_p handle, next;

   if(junk)
   {
      handle = junk->succ;
      while(handle != junk)
      {
         next = handle->succ;
         OrdSetCellFree(handle);
         handle = next;
      }
      OrdSetCellFree(junk);
   }
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetStore()
//
//   Insert key into the set. If it is new, it becomes the last element
//   and true is returned. If it is already in the set, the set is not
//   changed (in particular, the key keeps its position) and false is
//   returned.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

bool OrdSetStore(OrdSet_p *set, void* key)
{
   OrdSet_p handle = OrdSetCellAlloc();

   handle->key = key;
   if(!insert_cell(set, handle))
   {
      OrdSetCellFree(handle);
      return false;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetFind()
//
//   Return the cell with key key, or NULL if key is not in the
//   set. Splays the tree.
//
// Global Variables: -
//
// Side Effects    : Reorganizes the tree (not the order)
//
/----------------------------------------------------------------------*/

OrdSet_p OrdSetFind(OrdSet_p *set, void* key)
{
   OrdSet_p anchor = *set;

   if(anchor && anchor->lson)
   {
      anchor->lson = splay_tree(anchor->lson, key);
      if(OSEqual(anchor->lson->key, key))
      {
         return anchor->lson;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetFindBinary()
//
//   Find the cell with key key by simple binary search, without
//   reorganizing the tree. Return NULL if key is not in the set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OrdSet_p OrdSetFindBinary(OrdSet_p set, void* key)
{
   OrdSet_p handle = set?set->lson:NULL;

   while(handle)
   {
      if(OSLesser(key, handle->key))
      {
         handle = handle->lson;
      }
      else if(OSGreater(key, handle->key))
      {
         handle = handle->rson;
      }
      else
      {
         break;
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetExtractKey()
//
//   Remove key from the set and return it. Return NULL if key is not
//   in the set.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

void* OrdSetExtractKey(OrdSet_p *set, void* key)
{
   OrdSet_p handle = extract_cell(set, key);
   void*    res = NULL;

   if(handle)
   {
      res = handle->key;
      OrdSetCellFree(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetExtractFirst()
//
//   Remove the first (oldest) element from the set and return
//   it. Return NULL if the set is empty.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

void* OrdSetExtractFirst(OrdSet_p *set)
{
   if(*set)
   {
      return OrdSetExtractKey(set, (*set)->succ->key);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetExtractLast()
//
//   Remove the last (most recently inserted) element from the set and
//   return it. Return NULL if the set is empty.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

void* OrdSetExtractLast(OrdSet_p *set)
{
   if(*set)
   {
      return OrdSetExtractKey(set, (*set)->pred->key);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetDeleteEntry()
//
//   Delete key from the set. Return true if it was in the set, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

bool OrdSetDeleteEntry(OrdSet_p *set, void* key)
{
   OrdSet_p handle = extract_cell(set, key);

   if(handle)
   {
      OrdSetCellFree(handle);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetMerge()
//
//   Merge the two sets, i.e. destroy add and add its elements to
//   *set (new ones at the end, in the order of add). Return true if
//   *set gains a new element.
//
// Global Variables: -
//
// Side Effects    : Changes *set, frees add
//
/----------------------------------------------------------------------*/

bool OrdSetMerge(OrdSet_p *set, OrdSet_p add)
{
   OrdSet_p handle, next;
   bool     res = false;

   if(!add)
   {
      return false;
   }
   assert(*set != add);

   handle = add->succ;
   while(handle != add)
   {
      next = handle->succ;
      if(insert_cell(set, handle))
      {
         res = true;
      }
      else
      {
         OrdSetCellFree(handle);
      }
      handle = next;
   }
   OrdSetCellFree(add);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetInsertSet()
//
//   Insert the elements of add into *set (new ones at the end, in the
//   order of add). add is not changed.
//
// Global Variables: -
//
// Side Effects    : Changes *set, memory operations
//
/----------------------------------------------------------------------*/

void OrdSetInsertSet(OrdSet_p *set, OrdSet_p add)
{
   OrdSet_p handle;

   for(handle = OrdSetFirst(add); handle; handle = OrdSetNext(add, handle))
   {
      OrdSetStore(set, handle->key);
   }
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetCardinality()
//
//   Return the number of elements in the set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long OrdSetCardinality(OrdSet_p set)
{
   OrdSet_p handle;
   long     res = 0;

   for(handle = OrdSetFirst(set); handle; handle = OrdSetNext(set, handle))
   {
      res++;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PStackToOrdSet()
//
//   Insert the pointers on the stack (from the bottom up) into
//   *set. Return the number of new elements.
//
// Global Variables: -
//
// Side Effects    : Changes *set, memory operations
//
/----------------------------------------------------------------------*/

long PStackToOrdSet(OrdSet_p *set, PStack_p stack)
{
   PStackPointer i;
   long          res = 0;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      if(OrdSetStore(set, PStackElementP(stack,i)))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetToPStack()
//
//   Push all elements of the set onto the stack, in the order of the
//   set. Return the number of values pushed.
//
// Global Variables: -
//
// Side Effects    : Changes target_stack
//
/----------------------------------------------------------------------*/

long OrdSetToPStack(PStack_p target_stack, OrdSet_p set)
{
   OrdSet_p handle;
   long     res = 0;

   for(handle = OrdSetFirst(set); handle; handle = OrdSetNext(set, handle))
   {
      PStackPushP(target_stack, handle->key);
      res++;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetSharedElement()
//
//   Return the first element of set2 (in the order of set2) that is
//   also in *set1, or NULL if there is none. This iterates over set2
//   and searches in set1, so make set2 the smaller one if you have a
//   choice.
//
// Global Variables: -
//
// Side Effects    : Reorganizes the tree of set1 (not its order)
//
/----------------------------------------------------------------------*/

void* OrdSetSharedElement(OrdSet_p *set1, OrdSet_p set2)
{
   OrdSet_p handle;

   for(handle = OrdSetFirst(set2); handle; handle = OrdSetNext(set2, handle))
   {
      if(OrdSetFind(set1, handle->key))
      {
         return handle->key;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetIntersection()
//
//   Return a new set containing the elements of set1 that are also in
//   set2, in the order of set1.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

OrdSet_p OrdSetIntersection(OrdSet_p set1, OrdSet_p set2)
{
   OrdSet_p handle, res = NULL;

   for(handle = OrdSetFirst(set1); handle; handle = OrdSetNext(set1, handle))
   {
      if(OrdSetFindBinary(set2, handle->key))
      {
         OrdSetStore(&res, handle->key);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetDestrIntersection()
//
//   Make *set1 the intersection of *set1 and set2 (keeping the order
//   of *set1). set2 is not changed. Return the number of elements
//   removed.
//
// Global Variables: -
//
// Side Effects    : Changes *set1, memory operations
//
/----------------------------------------------------------------------*/

long OrdSetDestrIntersection(OrdSet_p *set1, OrdSet_p set2)
{
   OrdSet_p anchor = *set1, handle, next;
   long     res = 0;

   if(!anchor)
   {
      return 0;
   }
   /* The anchor is freed when the last element is removed. It is only
      compared against afterwards, never dereferenced. */
   handle = anchor->succ;
   while(handle != anchor)
   {
      next = handle->succ;
      if(!OrdSetFindBinary(set2, handle->key))
      {
         OrdSetDeleteEntry(set1, handle->key);
         res++;
      }
      handle = next;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetCopy()
//
//   Return a new set with the same elements in the same order.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

OrdSet_p OrdSetCopy(OrdSet_p set)
{
   OrdSet_p handle, res = NULL;

   for(handle = OrdSetFirst(set); handle; handle = OrdSetNext(set, handle))
   {
      OrdSetStore(&res, handle->key);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetEquiv()
//
//   Return true if both sets contain the same pointers (independent
//   of their order).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OrdSetEquiv(OrdSet_p set1, OrdSet_p set2)
{
   OrdSet_p handle;

   if(OrdSetCardinality(set1) != OrdSetCardinality(set2))
   {
      return false;
   }
   for(handle = OrdSetFirst(set1); handle; handle = OrdSetNext(set1, handle))
   {
      if(!OrdSetFindBinary(set2, handle->key))
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetIsSubset()
//
//   Return true if all elements of sub are in *super.
//
// Global Variables: -
//
// Side Effects    : Reorganizes the tree of *super (not its order)
//
/----------------------------------------------------------------------*/

bool OrdSetIsSubset(OrdSet_p sub, OrdSet_p *super)
{
   OrdSet_p handle;

   for(handle = OrdSetFirst(sub); handle; handle = OrdSetNext(sub, handle))
   {
      if(!OrdSetFind(super, handle->key))
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetVisit()
//
//   Apply visitor to every element of the set, in the order of the
//   set. arg is passed as the (first) additional argument.
//
// Global Variables: -
//
// Side Effects    : As visitor
//
/----------------------------------------------------------------------*/

void OrdSetVisit(OrdSet_p set, void (*visitor)(void*, void*), void* arg)
{
   OrdSet_p handle;

   for(handle = OrdSetFirst(set); handle; handle = OrdSetNext(set, handle))
   {
      visitor(arg, handle->key);
   }
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetCheck()
//
//   Check the consistency of a set: the list is cyclic and correctly
//   linked, the tree is a search tree over exactly the list cells, and
//   only non-empty sets have an anchor. Return true if all is well.
//   For testing and debugging.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OrdSetCheck(OrdSet_p set)
{
   OrdSet_p handle;
   long     list_cells = 0, tree_cells;

   if(!set)
   {
      return true;
   }
   if(set->succ == set || set->pred == set || !set->lson)
   {
      return false; /* Anchors only exist for non-empty sets */
   }
   handle = set;
   do
   {
      if(handle->succ->pred != handle)
      {
         return false;
      }
      handle = handle->succ;
      if(handle != set)
      {
         if(OrdSetFindBinary(set, handle->key) != handle)
         {
            return false;
         }
         list_cells++;
      }
   }while(handle != set);

   tree_cells = tree_check(set->lson, set);

   return tree_cells == list_cells;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetTraverseInit()
//
//   Return an iterator over the set, in the order of the set. While
//   iterating, the element just returned may be deleted from the set
//   (even if it is the last one), but no other changes are allowed.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

OrdSetIter_p OrdSetTraverseInit(OrdSet_p set)
{
   OrdSetIter_p iter = OrdSetIterCellAlloc();

   iter->anchor = set;
   iter->next   = set?set->succ:NULL;

   return iter;
}


/*-----------------------------------------------------------------------
//
// Function: OrdSetTraverseNext()
//
//   Return the next cell of the iteration, or NULL at the end.
//
// Global Variables: -
//
// Side Effects    : Updates iter
//
/----------------------------------------------------------------------*/

OrdSet_p OrdSetTraverseNext(OrdSetIter_p iter)
{
   OrdSet_p res;

   if(!iter->anchor || iter->next == iter->anchor)
   {
      return NULL;
   }
   res = iter->next;
   iter->next = res->succ;

   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
