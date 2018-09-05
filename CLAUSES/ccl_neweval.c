/*-----------------------------------------------------------------------

File  : ccl_neweval.c

Author: Stephan Schulz

Contents

  Data type for representing evaluations of clauses.

  Copyright 2006 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue May 16 23:08:03 CEST 2006
    New (adapted ccl_evaluations.c)

-----------------------------------------------------------------------*/

#include "ccl_neweval.h"

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
// Function: Eval_p evals_alloc_raw()
//
//   Allocate an evaluation cell with proper eval_no and eval_count 0.
//
// Global ariables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static Eval_p evals_alloc_raw(int eval_no)
{
   Eval_p eval = EvalCellAlloc(eval_no);

   eval->eval_no    = eval_no;
   eval->eval_count = 0;

   return eval;
}


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

static Eval_p splay_tree(Eval_p tree, Eval_p splay, int pos)
{
   Eval_p left, right, tmp, newnode;
   long   cmpres;

   if (!tree)
   {
      return tree;
   }

   newnode = evals_alloc_raw(splay->eval_no);
   newnode->evals[pos].lson = NULL;
   newnode->evals[pos].rson = NULL;
   left = newnode;
   right = newnode;

   for (;;)
   {
      cmpres = EvalCompare(splay, tree, pos);
      if (cmpres < 0)
      {
         if(!tree->evals[pos].lson)
         {
            break;
         }
         if(EvalCompare(splay, tree->evals[pos].lson, pos) < 0)
         {
            tmp = tree->evals[pos].lson;
            tree->evals[pos].lson = tmp->evals[pos].rson;
            tmp->evals[pos].rson = tree;
            tree = tmp;
            if (!tree->evals[pos].lson)
            {
               break;
            }
         }
         right->evals[pos].lson = tree;
         right = tree;
         tree = tree->evals[pos].lson;
      }
      else if(cmpres > 0)
      {
         if (!tree->evals[pos].rson)
         {
            break;
         }
         if(EvalCompare(splay, tree->evals[pos].rson, pos) > 0)
         {
            tmp = tree->evals[pos].rson;
            tree->evals[pos].rson = tmp->evals[pos].lson;
            tmp->evals[pos].lson = tree;
            tree = tmp;
            if (!tree->evals[pos].rson)
            {
               break;
            }
         }
         left->evals[pos].rson = tree;
         left = tree;
         tree = tree->evals[pos].rson;
      }
      else
      {
         break;
      }
   }
   left->evals[pos].rson = tree->evals[pos].lson;
   right->evals[pos].lson = tree->evals[pos].rson;
   tree->evals[pos].lson = newnode->evals[pos].rson;
   tree->evals[pos].rson = newnode->evals[pos].lson;

   EvalsFree(newnode);
   return tree;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: Eval_p EvalsAlloc()
//
//   Allocate an evaluation cell with proper eval_count;
//
// Global ariables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Eval_p EvalsAlloc(int eval_no)
{
   Eval_p eval = evals_alloc_raw(eval_no);

   eval->eval_count = EvaluationCounter++;

   return eval;
}


/*-----------------------------------------------------------------------
//
// Function: EvalsFree()
//
//   Free a list of evaluations. Does _not_ check for tree
//   dependencies.
//
// Global Variables: -
//
// Side Effects    : Memory management.
//
/----------------------------------------------------------------------*/

void EvalsFree(Eval_p junk)
{
   if(junk)
   {
      /* printf("Evalno: %d\n", junk->eval_no); */
      EvalCellFree(junk, junk->eval_no);
   }
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

void EvalPrint(FILE* out, Eval_p eval, int pos)
{
   fprintf(out, "[%3ld:%.10f:%ld]", eval->evals[pos].priority,
      eval->evals[pos].heuristic, eval->eval_count);
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

void EvalPrintComment(FILE* out, Eval_p eval, int pos)
{
   fprintf(out, "/*");
   EvalPrint(out, eval, pos);
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
   int i;

   for(i=0; i<list->eval_no; i++)
   {
      EvalPrint(out, list, i);
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
   int i;

   for(i=0; i<list->eval_no; i++)
   {
      list->evals[i].priority = priority;
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
   int i;

   for(i=0; i<list->eval_no; i++)
   {
      list->evals[i].priority += diff;
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


bool EvalGreater(Eval_p ev1, Eval_p ev2, int pos)
{
   if(ev1->evals[pos].priority > ev2->evals[pos].priority)
   {
      return true;
   }
   else if(ev1->evals[pos].priority == ev2->evals[pos].priority)
   {
      if(ev1->eval_count==ev2->eval_count)
      {
         return false;
      }
      if(ev1->evals[pos].heuristic > ev2->evals[pos].heuristic)
      {
    return true;
      }
      else if(ev1->evals[pos].heuristic == ev2->evals[pos].heuristic)
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

long EvalCompare(Eval_p ev1, Eval_p ev2, int pos)
{
   long res;

   res = ev1->evals[pos].priority - ev2->evals[pos].priority;
   if(res)
   {
      return res;
   }

   res = ev1->eval_count - ev2->eval_count;
   if(res == 0)
   {
      return res;
   }

   long res_heuristic = CMP(ev1->evals[pos].heuristic, ev2->evals[pos].heuristic);
   if(res_heuristic)
   {
      return res_heuristic;
   }

   return res;
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

Eval_p EvalTreeInsert(Eval_p *root, Eval_p newnode, int pos)
{
   long cmpres;
   if (!*root)
   {
      newnode->evals[pos].lson = newnode->evals[pos].rson = NULL;
      *root = newnode;
      return NULL;
   }
   *root = splay_tree(*root, newnode, pos);

   cmpres = EvalCompare(newnode, *root, pos);

   if (cmpres < 0)
   {
      newnode->evals[pos].lson = (*root)->evals[pos].lson;
      newnode->evals[pos].rson = *root;
      (*root)->evals[pos].lson = NULL;
      *root = newnode;
      return NULL;
   }
   else if(cmpres > 0)
   {
      newnode->evals[pos].rson = (*root)->evals[pos].rson;
      newnode->evals[pos].lson = *root;
      (*root)->evals[pos].rson = NULL;
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

Eval_p EvalTreeFind(Eval_p *root, Eval_p key, int pos)
{
   if(*root)
   {
      *root = splay_tree(*root, key, pos);
      if(EvalCompare(*root, key, pos)==0)
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
//   Find the entry with key key and remove it from the tree. Return
//   NULL if no matching element exists.
//
// Global Variables: -
//
// Side Effects    : Changes the tree
//
/----------------------------------------------------------------------*/


Eval_p EvalTreeExtractEntry(Eval_p *root, Eval_p key, int pos)
{
   Eval_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_tree(*root, key, pos);
   if(EvalCompare(key, (*root), pos)==0)
   {
      if (!(*root)->evals[pos].lson)
      {
         x = (*root)->evals[pos].rson;
      }
      else
      {
         x = splay_tree((*root)->evals[pos].lson, key, pos);
         x->evals[pos].rson = (*root)->evals[pos].rson;
      }
      cell = *root;
      cell->evals[pos].lson = cell->evals[pos].rson = NULL;
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

bool EvalTreeDeleteEntry(Eval_p *root, Eval_p key, int pos)
{
   Eval_p cell;

   cell = EvalTreeExtractEntry(root, key, pos);
   if(cell)
   {
      EvalsFree(cell);
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

Eval_p EvalTreeFindSmallest(Eval_p root, int pos)
{
   if(!root)
   {
      return NULL;
   }
   while(root->evals[pos].lson)
   {
      root = root->evals[pos].lson;
   }

   return root;
}

/*-----------------------------------------------------------------------
//
// Function: EvalTreeTraverseInit()
//
//   Return a stack containing the path to the smallest element in the
//   tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
//---------------------------------------------------------------------*/

PStack_p EvalTreeTraverseInit(Eval_p root, int pos)
{
   PStack_p stack = PStackAlloc();

   while(root)
   {
      PStackPushP(stack, root);
      root = root->evals[pos].lson;
   }
   return stack;
}


/*---------------------------------------------------------------------
//
// Function: EvalTreeTraverseNext()
//
//   Given a stack describing a traversal state, return the next node
//   and update the stack.
//
// Global Variables: -
//
// Side Effects    : Updates stack
//
/----------------------------------------------------------------------*/

Eval_p EvalTreeTraverseNext(PStack_p state, int pos)
{
   Eval_p handle, res;

   if(PStackEmpty(state))
   {
      return NULL;
   }
   res = PStackPopP(state);
   handle = res->evals[pos].rson;
   while(handle)
   {
      PStackPushP(state, handle);
      handle = handle->evals[pos].lson;
   }
   return res;
}


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

void EvalTreePrintInOrder(FILE* out, Eval_p tree, int pos)
{
   PStack_p state;
   Eval_p   handle;

   state = EvalTreeTraverseInit(tree, pos);

   while((handle = EvalTreeTraverseNext(state, pos)))
   {
      EvalListPrintComment(out, handle);
      fputc('\n', out);
   }
   EvalTreeTraverseExit(state);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/



