/*-----------------------------------------------------------------------

File  : ccl_evaluations.c

Author: Stephan Schulz

Contents
 
  Data type for representing evaluations of clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr  9 02:00:51 MET DST 1998
    New
<2> Thu Jan 28 00:58:19 MET 1999
    Replaced AVL trees with Splay-Trees

-----------------------------------------------------------------------*/

#include "ccl_evaluations.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long EvaluationCounter = 0;


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
//   Perform the splay operation on tree at node with key.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static Eval_p splay_tree(Eval_p tree, Eval_p splay) 
{
   Eval_p   left, right, tmp;
   EvalCell newnode;
   long     cmpres;
   
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
      cmpres = EvalCompare(splay, tree);
      if (cmpres < 0) 
      {
         if(!tree->lson)
         {
            break;
         }
         if(EvalCompare(splay, tree->lson) < 0)
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
      else if(cmpres > 0)
      {
         if (!tree->rson)
         {
            break;
         }
         if(EvalCompare(splay, tree->rson) > 0) 
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



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: Eval_p EvalAlloc()
//
//   Allocate an evaluation cell with proper eval_count;
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

Eval_p EvalAlloc(void)
{
   Eval_p eval = EvalCellAlloc();

   eval->eval_count = EvaluationCounter++;

   return eval;
}

/*-----------------------------------------------------------------------
//
// Function: EvalPrint()
//
//   Print an evaluation to the given channel.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EvalPrint(FILE* out, Eval_p eval)
{
   fprintf(out, "[%3ld:%.10f:%ld]", eval->priority,
	   eval->heuristic, eval->eval_count);
}


/*-----------------------------------------------------------------------
//
// Function: EvalPrintComment()
//
//   Print an evaluation (as a comment) to the given channel.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EvalPrintComment(FILE* out, Eval_p eval)
{
   fprintf(out, "/*");
   EvalPrint(out, eval);
   fprintf(out, "*/");

}


/*-----------------------------------------------------------------------
//
// Function: EvalListPrint()
//
//   Print an evaluation list.
//
// Global Variables: -
//
// Side Effects    : Ouput
//
/----------------------------------------------------------------------*/

void EvalListPrint(FILE* out, Eval_p list)
{
   while(list)
   {
      EvalPrint(out, list);
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EvalListPrintComment()
//
//   Print an evaluation list as a comment.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EvalListPrintComment(FILE* out, Eval_p list)
{   
   fprintf(out, "/*");
   EvalListPrint(out, list);
   fprintf(out, "*/");
}


/*-----------------------------------------------------------------------
//
// Function: EvalListSetPriority()
//
//   Set the priority in all elements of the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EvalListSetPriority(Eval_p list, EvalPriority priority)
{
   while(list)
   {
      list->priority = priority;
      list = list->next;
   }
}

/*-----------------------------------------------------------------------
//
// Function: EvalListChangePriority()
//
//   Change the priority in all elements of the list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EvalListChangePriority(Eval_p list, EvalPriority diff)
{
   while(list)
   {
      list->priority += diff;
      list = list->next;
   }
}


/*-----------------------------------------------------------------------
//
// Function: EvalGreater()
//
//   Compare two evaluations, return true if the first one is
//   greater. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


bool EvalGreater(Eval_p ev1, Eval_p ev2)
{
   if(ev1->priority > ev2->priority)
   {
      return true;
   }
   else if(ev1->priority == ev2->priority)
   {
      if(ev1->eval_count==ev2->eval_count)
      {
         return false;
      }
      if(ev1->heuristic > ev2->heuristic)
      {
	 return true;
      }
      else if(ev1->heuristic == ev2->heuristic)
      {
	 if(ev1->eval_count > ev2->eval_count)
	 {
	    return true;
	 }      
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: EvalCompare()
//
//   Compare two evaluations, return a value <0, =0 or >0 if the first
//   one is smaller than, equal two, or bigger than the second one.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long EvalCompare(Eval_p ev1, Eval_p ev2)
{
   long   res;
   
   res = ev1->priority - ev2->priority;
   if(res)
   {
      return res;
   }
   if(ev1->eval_count==ev2->eval_count)
   {
      return 0;
   }
   if(ev1->heuristic<ev2->heuristic)
   {
      assert(ev1->eval_count!=ev2->eval_count);
      return -1;
   }
   if(ev1->heuristic>ev2->heuristic)
   {
      assert(ev1->eval_count!=ev2->eval_count);
      return 1;
   }
   return ev1->eval_count - ev2->eval_count;
}


/*-----------------------------------------------------------------------
//
// Function: EvalListFree()
//
//   Free a list of evaluations. Does _not_ check for tree
//   dependencies. 
//
// Global Variables: -
//
// Side Effects    : Memory management.
//
/----------------------------------------------------------------------*/

void EvalListFree(Eval_p junk)
{
   Eval_p next;
   
   while(junk)
   {
      next = junk->next;
      EvalCellFree(junk);
      junk = next;
   }      
}


/*-----------------------------------------------------------------------
//
// Function: EvalTreeFree()
//
//   Free a evaluation tree (does not free the object).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void EvalTreeFree(Eval_p junk)
{
   if(junk)
   {
      PStack_p stack = PStackAlloc();
      
      PStackPushP(stack, junk);
      
      while(!PStackEmpty(stack))
      {
         junk = PStackPopP(stack);
         if(junk->lson)
         {
            PStackPushP(stack, junk->lson);
         }
         if(junk->rson)
         { 
            PStackPushP(stack, junk->rson);
         }
	 EvalCellFree(junk);            
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: EvalTreeInsert()
//
//   If an entry with newnode exists in the tree return a
//   pointer to it. Otherwise insert newnode in the tree and return
//   NULL. 
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/

Eval_p EvalTreeInsert(Eval_p *root, Eval_p newnode)
{
   long cmpres;
   if (!*root) 
   {
      newnode->lson = newnode->rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_tree(*root, newnode);

   cmpres = EvalCompare(newnode, *root);
   
   if (cmpres < 0) 
   {
      newnode->lson = (*root)->lson;
      newnode->rson = *root;
      (*root)->lson = NULL;
      *root = newnode;
      return NULL;
   } 
   else if(cmpres > 0) 
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
// Function: EvalTreeFind()
//
//   Find the entry with key key in the tree and return it. Return
//   NULL if no such key exists.
//
// Global Variables: -
//
// Side Effects    : Splays the tree
//
/----------------------------------------------------------------------*/

Eval_p EvalTreeFind(Eval_p *root, Eval_p key)
{
   if(*root)
   {
      *root = splay_tree(*root, key);  
      if(EvalCompare(*root, key)==0)
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: EvalTreeExtractEntry()
//
//   Find the entry with key key, remove it from the tree, rebalance
//   the tree, and return the pointer to the removed element. Return
//   NULL if no matching element exists.
//
//   Basic Algorithm (obsolete, replaced by recursive Version):
//
//   - Search for element el to remove, keeping potential update position
//   - Search for the one-child element mv to take its position, again
//     keeping potential update postition:
//        + if balance == -1, search for largest element in el->lson
//        + Otherwise search for the smallest element in el->rson
//   - for update to mv update balance
//   - replace el by mv and update mv's parent.
//   - Rebalance tree at *update and hope that it works ;-)
//   - Oh yes, return el...
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/


Eval_p EvalTreeExtractEntry(Eval_p *root, Eval_p key)
{
   Eval_p x, cell;
   
   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key);
   if(EvalCompare(key, (*root))==0)
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
// Function: EvalTreeDeleteEntry()
//
//   Delete the entry with key key from the tree. 
//
// Global Variables: -
//
// Side Effects    : By EvalExtract(), memory operations
//
/----------------------------------------------------------------------*/

bool EvalTreeDeleteEntry(Eval_p *root, Eval_p key)
{
   Eval_p cell;
   
   cell = EvalTreeExtractEntry(root, key);
   if(cell)
   {
      EvalCellFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: EvalTreeFindSmallest()
//
//   Find the smallest evaluation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Eval_p EvalTreeFindSmallest(Eval_p root)
{
   if(!root)
   {
      return NULL;
   }
   while(root->lson)
   {
      root = root->lson;
   }
   
   return root;
}


AVL_TRAVERSE_DEFINITION(EvalTree,Eval_p)


/*-----------------------------------------------------------------------
//
// Function: EvalTreePrintInOrder()
//
//   Print an evaluation tree in ascending order to stdout (mainly for
//   debugging and to test the traversal functions ;-)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void EvalTreePrintInOrder(FILE* out, Eval_p tree)
{
   PStack_p state;
   Eval_p   handle;

   state = EvalTreeTraverseInit(tree);
   
   while((handle = EvalTreeTraverseNext(state)))
   {
      EvalPrintComment(out, handle);
   }
   EvalTreeTraverseExit(state);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/




