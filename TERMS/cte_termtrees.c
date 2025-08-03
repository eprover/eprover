/*-----------------------------------------------------------------------

File  : cte_termtrees.c

Author: Stephan Schulz

Contents

  Implementation of term-top indexed trees (I found that I can
  cleanly separate this from the termbank stuff).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Nov 27 19:12:51 MET 1997
    New
<2> Thu Jan 28 00:59:59 MET 1999
    Replaced AVL trees with Splay-Trees

-----------------------------------------------------------------------*/

#include "cte_termtrees.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

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

static Term_p splay_term_tree(Term_p tree, Term_p splay)
{
   Term_p   left, right, tmp;
   TermCell new;

   if (!tree)
   {
      return tree;
   }

   new.lson = NULL;
   new.rson = NULL;
   left = &new;
   right = &new;

   for (;;)
   {
      long cmpres = TermTopCompare(splay, tree);
      if (cmpres < 0)
      {
         if(!tree->lson)
         {
            break;
         }
         if(TermTopCompare(splay, tree->lson) < 0)
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
         if(TermTopCompare(splay, tree->rson) > 0)
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
   tree->lson = new.rson;
   tree->rson = new.lson;

   return tree;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TermTreeFree(Term_p junk)
//
//   Release the memory taken by a term top AVL tree. Do not free
//   variables, as they belong to a variable bank as well. Yes, this
//   is an ugly hack! *sigh*
//
// Global Variables: -
//
// Side Effects    : Memory operations, destroys tree
//
/----------------------------------------------------------------------*/

void TermTreeFree(Term_p junk)
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
         if(!TermIsAnyVar(junk))
         {
            TermTopFree(junk);
         }
      }
      PStackFree(stack);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermTopCompare()
//
//   Compare two top level term cells as
//   f_code.masked_properties.args_as_pointers, return a value >0 if
//   t1 is greater, 0 if the terms are identical, <0 if t2 is
//   greater.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TermTopCompare(Term_p t1, Term_p t2)
{
   int i;

   long res = t1->f_code - t2->f_code;
   if(res)
   {
      return res;
   }

   assert(t1->type);
   assert(t2->type);
   //if(t1->sort!=t2->sort)
   //{
   //      printf(COMCHAR" Sort clash (%ld): %d vs. %d\n", t1->f_code, t1->sort, t2->sort);
   //}
   assert(problemType == PROBLEM_HO || t1->type == t2->type);
   //assert(TermIsPhonyApp(t1) || problemType == PROBLEM_HO  || t1->arity == t2->arity);
   //Not true with polymorphism!

   if(problemType == PROBLEM_HO)
   {
      res = PCmp(t1->type, t2->type);
      if(res)
      {
         return res;
      }
   }

   if(t1->arity != t2->arity)
   {
      return t1->arity - t2->arity;
   }

   for(i=0; i<t1->arity; i++)
   {
      /* res = (t1->args[i]->entry_no) - (t2->args[i]->entry_no); */
      res = PCmp(t1->args[i], t2->args[i]);
      if(res)
      {
         return res;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermTreeFind()
//
//   Find a entry in the term tree, given a cell with correct
//   (i.e. term-bank) argument pointers.
//   pointers
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TermTreeFind(Term_p *root, Term_p key)
{
   if(*root)
   {
      *root = splay_term_tree(*root, key);
      if(TermTopCompare(*root, key)==0)
      {
         return *root;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: TermTreeInsert()
//
//   Insert a term with valid subterm pointers into the termtree. If
//   the entry already exists, return pointer to existing entry as
//   usual, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

Term_p TermTreeInsert(Term_p *root, Term_p new)
{
   if (!*root)
   {
      new->lson = new->rson = NULL;
      *root = new;
      return NULL;
   }
   *root = splay_term_tree(*root, new);

   long cmpres = TermTopCompare(new, *root);

   if (cmpres < 0)
   {
      new->lson = (*root)->lson;
      new->rson = *root;
      (*root)->lson = NULL;
      *root = new;
      return NULL;
   }
   else if(cmpres > 0)
   {
      new->rson = (*root)->rson;
      new->lson = *root;
      (*root)->rson = NULL;
      *root = new;
      return NULL;
   }
   return *root;
}


/*-----------------------------------------------------------------------
//
// Function: TermTreeExtract()
//
//   Remove a top term cell from the term tree and return a pointer to
//   it.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

Term_p TermTreeExtract(Term_p *root, Term_p key)
{
   Term_p x, cell;

   if (!(*root))
   {
      return NULL;
   }
   *root = splay_term_tree(*root, key);
   if(TermTopCompare(key, (*root))==0)
   {
      if (!(*root)->lson)
      {
         x = (*root)->rson;
      }
      else
      {
         x = splay_term_tree((*root)->lson, key);
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
// Function: TermTreeDelete()
//
//   Delete a top term from the term tree.
//
// Global Variables: -
//
// Side Effects    : Changes tree, memory operations
//
/----------------------------------------------------------------------*/

bool TermTreeDelete(Term_p *root, Term_p term)
{
   Term_p cell;

   cell = TermTreeExtract(root, term);
   if(cell)
   {
      TermTopFree(cell);
      return true;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: TermTreeSetProp()
//
//   Set the given properties for all term cells in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermTreeSetProp(Term_p root, TermProperties props)
{
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, root);

   while(!PStackEmpty(stack))
   {
      root = PStackPopP(stack);
      if(root)
      {
    TermCellSetProp(root, props);
    PStackPushP(stack, root->lson);
    PStackPushP(stack, root->rson);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermTreeDelProp()
//
//   Delete the given properties for all term cells in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermTreeDelProp(Term_p root, TermProperties props)
{
   PStack_p stack = PStackAlloc();

   PStackPushP(stack, root);

   while(!PStackEmpty(stack))
   {
      root = PStackPopP(stack);
      if(root)
      {
    TermCellDelProp(root, props);
    PStackPushP(stack, root->lson);
    PStackPushP(stack, root->rson);
      }
   }
   PStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: TermTreeNodes()
//
//   Return the number of nodes in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TermTreeNodes(Term_p root)
{
   PStack_p stack = PStackAlloc();
   long     res   = 0;

   PStackPushP(stack, root);

   while(!PStackEmpty(stack))
   {
      root = PStackPopP(stack);
      if(root)
      {
         PStackPushP(stack, root->lson);
         PStackPushP(stack, root->rson);
         res++;
      }
   }
   PStackFree(stack);

   return res;
}


AVL_TRAVERSE_DEFINITION(TermTree, Term_p)


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
