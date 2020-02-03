/*-----------------------------------------------------------------------

  File  : ccl_pdtrees.c

  Author: Stephan Schulz

  Contents

  Perfect discrimination trees for optimized rewriting.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Jun 24 00:55:29 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_pdtrees.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool PDTreeUseAgeConstraints  = true;
bool PDTreeUseSizeConstraints = true;

#ifdef PDT_COUNT_NODES
unsigned long PDTNodeCounter = 0;
#endif

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static long pdt_compute_size_constraint(PDTNode_p node);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: push_remaining_args()
//
//   Puts arguments that are trailing to the term stack.
//   Determines the number of trailing arguments based on eaten_args.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ void push_remaining_args(PStack_p term_stack, int eaten_args, Term_p to_match)
{
   // make up for app encoding
   int limit = eaten_args + (TermIsAppliedVar(to_match) ? 1 : 0);
   for(int i=to_match->arity-1; i >= limit; i--)
   {
      PStackPushP(term_stack, to_match->args[i]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: pdtree_default_cell_free()
//
//   Free a node cell (but not potential children et al.)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void pdtree_default_cell_free(PDTNode_p junk)
{
   assert(junk);
   assert(junk->f_alternatives);
   assert(junk->v_alternatives);
   assert(!junk->entries);

   IntMapFree(junk->f_alternatives);
   PDArrayFree(junk->v_alternatives);
   PDTNodeCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: pdt_select_alt_ref()
//
//   Return a pointer to the position where the alternative to term is
//   stored.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void* pdt_select_alt_ref(PDTree_p tree, PDTNode_p node, Term_p term)
{
   void* res;

   if(TermIsVar(term))
   {
      tree->arr_storage_est -= PDArrayStorage(node->v_alternatives);
      res = &(PDArrayElementP(node->v_alternatives,
                              -term->f_code));
      tree->arr_storage_est += PDArrayStorage(node->v_alternatives);
   }
   else
   {
      tree->arr_storage_est -= IntMapStorage(node->f_alternatives);
      res = IntMapGetRef(node->f_alternatives, term->f_code);
      tree->arr_storage_est += IntMapStorage(node->f_alternatives);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: pdt_node_succ_stack_create()
//
//   Create a stack of all children of node and return it (for
//   convenient traversal).
//
// Global Variables: -
//
// Side Effects    : May reset node->max_var (which has to be at least
//                   as big as the largest variable number).
//
/----------------------------------------------------------------------*/

static PStack_p pdt_node_succ_stack_create(PDTNode_p node)
{
   PStack_p result = PStackAlloc();
   FunCode i = 0; /* Stiffle warning */
   long tmpmaxvar = 0;
   PDTNode_p next;
   IntMapIter_p iter;

   iter = IntMapIterAlloc(node->f_alternatives, 0, LONG_MAX);
   while((next=IntMapIterNext(iter, &i)))
   {
      assert(next);
      PStackPushP(result, next);
   }
   IntMapIterFree(iter);
   for(i=1; i<=node->max_var; i++)
   {
      next = PDArrayElementP(node->v_alternatives, i);
      if(next)
      {
         PStackPushP(result, next);
         tmpmaxvar = i;
      }
   }
   node->max_var = tmpmaxvar;
   return result;
}



/*-----------------------------------------------------------------------
//
// Function: pos_tree_compute_size_constraint()
//
//   Find the size of the smallest term at a position in tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long pos_tree_compute_size_constraint(PTree_p tree)
{
   ClausePos_p entry;
   PTree_p  trav;
   PStack_p trav_stack;
   long res = LONG_MAX;

   trav_stack = PTreeTraverseInit(tree);
   while((trav = PTreeTraverseNext(trav_stack)))
   {
      entry = trav->key;
      res = MIN(res, TermStandardWeight(ClausePosGetSide(entry)));
   }
   PTreeTraverseExit(trav_stack);
   /* This is a leaf node, size is fixed! */

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: pdt_compute_size_constraint()
//
//   Compute and set the size constraint of the current node in the PDT
//   tree.
//
// Global Variables: -
//
// Side Effects    : Sets the updated size constraint, possibly in all
//                   children.
//
/----------------------------------------------------------------------*/

static long pdt_compute_size_constraint(PDTNode_p node)
{
   if(node->leaf)
   {
      node->size_constr = pos_tree_compute_size_constraint(node->entries);
   }
   else
   {
      PStack_p iter_stack = pdt_node_succ_stack_create(node);
      PStackPointer i;
      long
         newsize = node->entries ? pos_tree_compute_size_constraint(node->entries) : LONG_MAX,
         tmpsize;
      PDTNode_p next_node;

      for(i = 0; i< PStackGetSP(iter_stack); i++)
      {
         next_node = PStackElementP(iter_stack, i);
         assert(next_node);
         tmpsize = PDTNodeGetSizeConstraint(next_node);
         newsize = MIN(newsize, tmpsize);
      }
      PStackFree(iter_stack);
      node->size_constr = newsize;
   }
   return node->size_constr;
}

/*-----------------------------------------------------------------------
//
// Function: pdt_verify_size_constraint()
//
//   Verify the size constraint at node, and return the optimal
//   value (or -1 if the tree is inconsistent)
//
// Global Variables: -
//
// Side Effects    : Sets the updated size constraint, possibly in all
//                   children.
//
/----------------------------------------------------------------------*/

long pdt_verify_size_constraint(PDTNode_p node)
{
   long actual_constr;

   if(node->leaf)
   {
      actual_constr = pos_tree_compute_size_constraint(node->entries);
   }
   else
   {
      PStackPointer i;
      actual_constr = node->entries ? pos_tree_compute_size_constraint(node->entries) : LONG_MAX;
      PStack_p  iter_stack = pdt_node_succ_stack_create(node);
      long      tmpsize;
      PDTNode_p next_node;

      for(i = 0; i< PStackGetSP(iter_stack); i++)
      {
         next_node = PStackElementP(iter_stack, i);
         assert(next_node);
         tmpsize = pdt_verify_size_constraint(next_node);
         actual_constr = MIN(actual_constr, tmpsize);
      }
      PStackFree(iter_stack);
   }
   if(node->size_constr == -1 || (node->size_constr == actual_constr))
   {
      return actual_constr;
   }
   return -1;
}


/*-----------------------------------------------------------------------
//
// Function: pos_tree_compute_age_constraint()
//
//   Find the creation date of the youngst clauss  at a position in
//   tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static SysDate pos_tree_compute_age_constraint(PTree_p tree)
{
   ClausePos_p entry;
   PTree_p  trav;
   PStack_p trav_stack;
   SysDate res = SysDateCreationTime();

   trav_stack = PTreeTraverseInit(tree);
   while((trav = PTreeTraverseNext(trav_stack)))
   {
      entry = trav->key;
      res = SysDateMaximum(res, entry->clause->date);
   }
   PTreeTraverseExit(trav_stack);
   /* This is a leaf node, size is fixed! */

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: pdt_compute_age_constraint()
//
//   Compute and set the age constraint (i.e. date stamp of the youngest clause
//   in the subtree) of the current node in the PDT tree.
//
// Global Variables: -
//
// Side Effects    : Sets the updated size constraint, possibly in all
//                   children.
//
/----------------------------------------------------------------------*/

static SysDate pdt_compute_age_constraint(PDTNode_p node)
{
   if(node->leaf)
   {
      node->age_constr = pos_tree_compute_age_constraint(node->entries);
   }
   else
   {
      PStack_p iter_stack = pdt_node_succ_stack_create(node);
      PStackPointer i;
      SysDate
         newdate = node->entries ? pos_tree_compute_age_constraint(node->entries) : SysDateCreationTime(),
         tmpdate;
      PDTNode_p next_node;

      for(i = 0; i< PStackGetSP(iter_stack); i++)
      {
         next_node = PStackElementP(iter_stack, i);
         assert(next_node);
         tmpdate = PDTNodeGetAgeConstraint(next_node);
         newdate = SysDateMaximum(newdate, tmpdate);
      }
      PStackFree(iter_stack);
      node->age_constr = newdate;
   }
   return node->age_constr;
}



/*-----------------------------------------------------------------------
//
// Function: pdt_verify_age_constraint()
//
//   Verify the age constraint at node, and return the optimal
//   value (or -1 if the tree is inconsistent)
//
// Global Variables: -
//
// Side Effects    : Sets the updated size constraint, possibly in all
//                   children.
//
/----------------------------------------------------------------------*/

SysDate pdt_verify_age_constraint(PDTNode_p node)
{
   SysDate actual_constr;

   if(node->leaf)
   {
      actual_constr = pos_tree_compute_age_constraint(node->entries);
   }
   else
   {
      PStackPointer i;
      actual_constr = node->entries ? pos_tree_compute_age_constraint(node->entries) : SysDateCreationTime();
      PStack_p      iter_stack = pdt_node_succ_stack_create(node);
      PDTNode_p     next_node;
      SysDate       tmpdate;

      for(i = 0; i< PStackGetSP(iter_stack); i++)
      {
         next_node = PStackElementP(iter_stack, i);
         assert(next_node);
         tmpdate = pdt_verify_age_constraint(next_node);
         actual_constr = SysDateMaximum(actual_constr, tmpdate);
      }
      PStackFree(iter_stack);
   }
   if(SysDateIsInvalid(node->age_constr) || (node->age_constr == actual_constr))
   {
      return actual_constr;
   }
   return -1;
}


/*-----------------------------------------------------------------------
//
// Function: delete_clause_entries()
//
//   Consider *root as a PTree of ClausePos_p and delete all entries
//   from it that describe a position in clause. Return number of
//   clauses.
//
// Global Variables: -
//
// Side Effects    : Changes tree
//
/----------------------------------------------------------------------*/

static long  delete_clause_entries(PTree_p *root, Clause_p clause)
{
   long        res = 0;
   PStack_p    trav_stack;
   PStack_p    store = PStackAlloc();
   PTree_p     handle;
   ClausePos_p pos;

   trav_stack = PTreeTraverseInit(*root);
   while((handle = PTreeTraverseNext(trav_stack)))
   {
      pos = handle->key;
      if(pos->clause == clause)
      {
         PStackPushP(store, pos);
      }
   }
   PTreeTraverseExit(trav_stack);

   while(!PStackEmpty(store))
   {
      pos = PStackPopP(store);
      PTreeDeleteEntry(root, pos);
      ClausePosCellFree(pos);
      res++;
   }
   PStackFree(store);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: pdtree_verify_node_constr()
//
//   Check if the current tree state is consistent with the query
//   constraints stored in the tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool pdtree_verify_node_constr(PDTree_p tree)
{
   PDT_COUNT_INC(PDTNodeCounter);

   /* Is largest term at or beyond node greater than the query term? */

   if(PDTreeUseSizeConstraints &&
      (tree->term_weight < PDTNodeGetSizeConstraint(tree->tree_pos)))
   {
      return false;
   }

   /* Is the youngest clause stored at or beyond node younger than the
      query terms normal form date ? */

   if(PDTreeUseAgeConstraints &&
      !SysDateIsEarlier(tree->term_date,PDTNodeGetAgeConstraint(tree->tree_pos)))
   {
      return false;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: pdtree_forward()
//
//   Find the next open possibility and advance to it. If none exists,
//   indicate this by setting tree->tree_pos->trav_count to
//   PDT_NODE_CLOSED.
//
// Global Variables: -
//
// Side Effects    : Changes tree state!
//
/----------------------------------------------------------------------*/

static void pdtree_forward(PDTree_p tree, Subst_p subst)
{
   PDTNode_p handle = tree->tree_pos, next = NULL;
   FunCode   i = tree->tree_pos->trav_count, limit;
   Term_p    term = PStackGetSP(tree->term_stack) ? PStackTopP(tree->term_stack) : NULL;

   if(!term)
   {
      i = tree->tree_pos->trav_count = PDT_NODE_CLOSED(tree, handle);
   }

   limit = PDT_NODE_CLOSED(tree,handle);
   while(i<limit)
   {
      if(((i==0)||(i>handle->max_var))&&!TermIsVar(term))
      {
         next = IntMapGetVal(handle->f_alternatives,term->f_code);
         i++;
         if(next)
         {
            PStackPushP(tree->term_proc, term);
            TermLRTraverseNext(tree->term_stack);
            next->trav_count = PDT_NODE_INIT_VAL(tree);
            next->bound      = false;
            assert(!next->variable);
            tree->tree_pos = next;
            /*if (next)
              fprintf(stderr, "next->entries ? %p next->leaf %d? \n", next->entries,  next->leaf);
              else
              fprintf(stderr, "no next\n");*/
#ifdef MEASURE_EXPENSIVE
            tree->visited_count++;
#endif
            break;
         }
      }
      else
      {
         next = PDArrayElementP(handle->v_alternatives,i);
         i++;
         if(next)
         {
            assert(next->variable);
            bool bound = false;
            if((!next->variable->binding)&&
               (problemType == PROBLEM_HO || TermIsVar(term) ||
                SigSymbolUnifiesWithVar(tree->bank->sig, term->f_code)) &&
               (problemType == PROBLEM_HO || next->variable->type == term->type))
            {
               if(problemType == PROBLEM_FO)
               {
                  assert(next->variable->type == term->type);
                  PStackDiscardTop(tree->term_stack);
                  SubstAddBinding(subst, next->variable, term);
                  bound = true;
               }
               else
               {
                  int matched_up_to = PartiallyMatchVar(next->variable, term, tree->bank->sig, false);
                  if(matched_up_to != MATCH_FAILED && matched_up_to <= ARG_NUM(term))
                  {
                     SubstBindAppVar(subst, next->variable, term, matched_up_to, tree->bank);
                     PStackPushP(tree->term_proc, term);
                     PStackDiscardTop(tree->term_stack);

                     if(matched_up_to != ARG_NUM(term))
                     {
                        push_remaining_args(tree->term_stack, matched_up_to, term);
                     }

                     bound = true;
                  }
               }

               if(bound)
               {
                  assert(next->variable->binding);
                  next->trav_count   = PDT_NODE_INIT_VAL(tree);
                  next->bound        = true;
                  tree->tree_pos     = next;
                  tree->term_weight  -= (TermStandardWeight(next->variable->binding) -
                                         TermStandardWeight(next->variable));
#ifdef MEASURE_EXPENSIVE
                  tree->visited_count++;
#endif
                  break;
               }

            }
            else if(next->variable->binding == term ||
                    (problemType == PROBLEM_HO && TermIsPrefix(next->variable->binding, term)))
            {
               //fprintf(stderr, "Got into next->variable->binding prefix part.\n");
               PStackDiscardTop(tree->term_stack);
               if(problemType == PROBLEM_HO)
               {
                  PStackPushP(tree->term_proc, term);
                  int args_eaten = next->variable->binding->arity -
                     (TermIsAppliedVar(next->variable->binding) ? 1 : 0);
                  push_remaining_args(tree->term_stack, args_eaten, term);
               }
               next->trav_count   = PDT_NODE_INIT_VAL(tree);
               next->bound        = false;
               tree->tree_pos     = next;
               tree->term_weight  -= (TermStandardWeight(next->variable->binding) -
                                      TermStandardWeight(next->variable));
#ifdef MEASURE_EXPENSIVE
               tree->visited_count++;
#endif
               break;
            }
         }
      }
   }
   handle->trav_count = i;
}


/*-----------------------------------------------------------------------
//
// Function: pdtree_backtrack()
//
//   Backtrack to the predecessor node of the current state.
//
// Global Variables: -
//
// Side Effects    : Changes tree state
//
/----------------------------------------------------------------------*/

static void pdtree_backtrack(PDTree_p tree, Subst_p subst)
{
   PDTNode_p handle = tree->tree_pos;
   bool      succ;

   if(handle->variable)
   {
      tree->term_weight  += (TermStandardWeight(handle->variable->binding) -
                             TermStandardWeight(handle->variable));
      if(problemType == PROBLEM_FO)
      {
         PStackPushP(tree->term_stack, handle->variable->binding);
         if(handle->bound)
         {
            succ = SubstBacktrackSingle(subst);
            UNUSED(succ); assert(succ);
         }
      }
      else if(handle->variable->binding)
      {
         Term_p original_term = PStackPopP(tree->term_proc);

         TermLRTraversePrevAppVar(tree->term_stack, original_term, handle->variable);
         if(handle->bound)
         {
            SubstBacktrackSingle(subst);
         }
      }
   }
   else if(handle->parent)
   {
      Term_p t = PStackPopP(tree->term_proc);

      UNUSED(t); assert(t);
      TermLRTraversePrev(tree->term_stack,t);
   }
   tree->tree_pos = handle->parent;
}


/*-----------------------------------------------------------------------
//
// Function: pdt_node_print()
//
//   Print a PDT node (and subtrees) for debugging.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void pdt_node_print(FILE* out, PDTNode_p node, int level)
{
   if(node->entries)
   {
      PStack_p trav_stack;
      PTree_p trav;
      ClausePos_p entry;

      fprintf(out, "%sleaf size=%ld age=%lu leaf?=%d\n", IndentStr(2*level),
              node->size_constr, node->age_constr, node->leaf);
      trav_stack = PTreeTraverseInit(node->entries);

      while((trav = PTreeTraverseNext(trav_stack)))
      {
         fprintf(out, "%s: ",IndentStr(2*level));
         entry = trav->key;
         ClausePrint(out, entry->clause, true);
         fprintf(out, "\n");
      }
      PTreeTraverseExit(trav_stack);
   }
   else if(!node->leaf)
   {
      FunCode i = 0; /* Stiffle warning */
      PDTNode_p next;
      IntMapIter_p iter;

      fprintf(out, "%sinternal size=%ld age=%lu f_alts=%p, type=%d\n",
              IndentStr(2*level),
              node->size_constr,
              node->age_constr,
              node->f_alternatives,
              node->f_alternatives?node->f_alternatives->type:-1);

      iter = IntMapIterAlloc(node->f_alternatives, 0, LONG_MAX);
      while((next=IntMapIterNext(iter, &i)))
      {
         fprintf(out, "%sBranch %ld\n", IndentStr(2*level), i);
         pdt_node_print(out, next, level+1);
      }
      IntMapIterFree(iter);
      for(i=1; i<=node->max_var; i++)
      {
         next = PDArrayElementP(node->v_alternatives, i);
         if(next)
         {
            fprintf(out, "%sBranch %ld\n", IndentStr(2*level), -i);
            pdt_node_print(out, next, level+1);
         }
      }
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PDTreeAlloc()
//
//   Allocate an empty, in initialized PDTreeCell (including the
//   initial PDTNodeCell().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PDTree_p PDTreeAlloc(TB_p bank)
{
   PDTree_p handle;

   handle = PDTreeCellAlloc();

   handle->tree            = PDTNodeAlloc();
   handle->term_stack      = PStackAlloc();
   handle->term_proc       = PStackAlloc();
   handle->tree_pos        = NULL;
   handle->store_stack     = NULL;
   handle->term            = NULL;
   handle->term_date       = SysDateCreationTime();
   handle->term_weight     = LONG_MAX;
   handle->prefer_general  = 0; /* Not really necessary, it's
                                   reinitialized in
                                   PDTreeSearchInit() anyways.*/
   handle->clause_count    = 0;
   handle->node_count      = 0;
   handle->arr_storage_est = 0;
   handle->match_count     = 0;
   handle->visited_count   = 0;
   handle->bank            = bank;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PDTreeFree()
//
//   Completely free a PDTree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PDTreeFree(PDTree_p tree)
{
   assert(tree);
   PDTNodeFree(tree->tree);
   PStackFree(tree->term_stack);
   PStackFree(tree->term_proc);
   assert(!tree->store_stack);
   PDTreeCellFree(tree);
}


/*-----------------------------------------------------------------------
//
// Function: PDTNodeAlloc()
//
//   Return an initialized node in a PDTree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PDTNode_p PDTNodeAlloc(void)
{
   PDTNode_p handle;

   handle = PDTNodeCellAlloc();

   handle->f_alternatives = IntMapAlloc();
   handle->v_alternatives = PDArrayAlloc(PDNODE_VAR_INIT_ALT,
                                         PDNODE_VAR_GROW_ALT);
   handle->max_var        = 0;
   handle->size_constr    = LONG_MAX;
   handle->age_constr     = SysDateCreationTime();
   handle->parent         = NULL;
   handle->ref_count      = 0;
   handle->entries        = NULL;
   handle->trav_count     = 0;
   handle->variable       = NULL;
   handle->bound          = false;
   handle->leaf           = true;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PDTNodeFree()
//
//   Free a PDTreeNode (including all referenced term positions.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PDTNodeFree(PDTNode_p tree)
{
   FunCode      i;
   IntMapIter_p iter;
   ClausePos_p  tmp;
   PDTNode_p     subtree;

   iter = IntMapIterAlloc(tree->f_alternatives, 0, LONG_MAX);
   while((subtree = IntMapIterNext(iter, &i)))
   {
      assert(subtree);
      PDTNodeFree(subtree);
   }
   IntMapIterFree(iter);
   for(i=1; i<=tree->max_var; i++)
   {
      subtree = PDArrayElementP(tree->v_alternatives, i);
      if(subtree)
      {
         PDTNodeFree(subtree);
      }
   }
   while(tree->entries)
   {
      tmp = PTreeExtractRootKey(&tree->entries);
      ClausePosCellFree(tmp);
   }
   pdtree_default_cell_free(tree);

}


/*-----------------------------------------------------------------------
//
// Function: TermLRTraverseInit()
//
//   Initialize a stack for term traversal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermLRTraverseInit(PStack_p stack, Term_p term)
{
   PStackReset(stack);
   PStackPushP(stack,term);
}


/*-----------------------------------------------------------------------
//
// Function: TermLRTraverseNext()
//
//   Return the next term node in LR-ordering and update the
//   stack. Return NULL if term traveral is complete.
//
// Global Variables: -
//
// Side Effects    : Stack changes
//
/----------------------------------------------------------------------*/

Term_p TermLRTraverseNext(PStack_p stack)
{
   int    i;
   Term_p handle;

   if(PStackEmpty(stack))
   {
      return NULL;
   }
   handle = PStackPopP(stack);
   for(i=handle->arity-1; i>=0; i--)
   {
      PStackPushP(stack, handle->args[i]);
   }

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermLRTraversePrev()
//
//   Undo a TermLRTraverseNext() operation by replacing terms args on
//   the stack with term.
//
// Global Variables: -
//
// Side Effects    : Stack changes
//
/----------------------------------------------------------------------*/

Term_p TermLRTraversePrev(PStack_p stack, Term_p term)
{
   Term_p tmp;
   int    i;

   for(i=0; i<term->arity; i++)
   {
      tmp = PStackPopP(stack);
      UNUSED(tmp); assert(tmp == term->args[i]);
   }
   PStackPushP(stack, term);

   return term;
}

/*-----------------------------------------------------------------------
//
// Function: TermLRTraversePrevAppVar()
//
//   Undoing binding applied variable. Differs from undoing binding
//   normal variable since the term bound is not always the whole term
//   on top of the stack -- applied variables can possibly match
//   prefixes.
//
// Global Variables: -
//
// Side Effects    : Stack changes
//
/----------------------------------------------------------------------*/

Term_p TermLRTraversePrevAppVar(PStack_p stack, Term_p original_term, Term_p var)
{
   Term_p tmp;
   int    i;

   assert(var->binding);
   assert(original_term->arity >= var->binding->arity);

   int to_backtrack_nr = original_term->arity - var->binding->arity;
   if(TermIsAppliedVar(original_term) && TermIsVar(var->binding))
   {
      to_backtrack_nr--;
   }

   for(i=0; i<to_backtrack_nr; i++)
   {
      tmp = PStackPopP(stack);
      UNUSED(tmp);
      assert(tmp == original_term->args[var->binding->arity + i +
                                        (TermIsAppliedVar(original_term) && TermIsVar(var->binding))]);
   }
   PStackPushP(stack, original_term);

   return original_term;
}



/*-----------------------------------------------------------------------
//
// Function: PDTreeInsert()
//
//   Insert a new demodulator into the tree.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

void PDTreeInsert(PDTree_p tree, ClausePos_p demod_side)
{
   Term_p term;
   
   assert(demod_side);
   term = ClausePosGetSide(demod_side);
   PDTreeInsertTerm(tree, term, demod_side, true);
}

/*-----------------------------------------------------------------------
//
// Function: PDTreeInsertTerm()
//
//   Insert a new term into the tree, possibly storing data in the leaf.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

void PDTreeInsertTerm(PDTree_p tree, Term_p term, ClausePos_p demod_side, 
   bool store_data)
{
   Term_p    curr;
   PDTNode_p node, *next;
   bool      res;
   long      tmp;

   assert(tree);
   assert(tree->tree);

   TermLRTraverseInit(tree->term_stack, term);
   node              = tree->tree;
   tmp = TermStandardWeight(term);
   if(demod_side&&(!SysDateIsInvalid(node->age_constr)))
   {
      node->age_constr  = SysDateMaximum(demod_side->clause->date,
                                         node->age_constr);
   }
   /* We need no guard here, since invalid = -1 will win out in either
      case. */
   node->size_constr = MIN(tmp, node->size_constr);
   node->ref_count++;

   curr = TermLRTraverseNext(tree->term_stack);

   while(curr)
   {
      if(TermIsAppliedVar(curr))
      {
         curr = TermLRTraverseNext(tree->term_stack);
         continue; // skipping the symbol for applied var.
      }

      next = pdt_select_alt_ref(tree, node, curr);

      if(!(*next))
      {
         *next = PDTNodeAlloc();
         node->leaf = false;

         tree->arr_storage_est+= (IntMapStorage((*next)->f_alternatives)+
                                  PDArrayStorage((*next)->v_alternatives));
         (*next)->parent = node;
         tree->node_count++;
         if(TermIsVar(curr))
         {
            (*next)->variable = curr;
            node->max_var = MAX(node->max_var, -curr->f_code);
         }
      }
      node = *next;
      //assert(!node->variable || (TermIsVar(curr) && node->variable->type == curr->type));
      tmp = TermStandardWeight(term);
      node->size_constr = MIN(tmp, node->size_constr);
      if(demod_side&&(!SysDateIsInvalid(node->age_constr)))
      {
         node->age_constr  = SysDateMaximum(demod_side->clause->date,
                                            node->age_constr);
      }
      node->ref_count++;
      curr = TermLRTraverseNext(tree->term_stack);
   }
   assert(node);
   if (store_data) 
   {
      res = PTreeStore(&(node->entries), demod_side);
      UNUSED(res); assert(res);
   }
   tree->clause_count++;
   //printf("ISizeConstr %p: %ld\n", tree, pdt_verify_size_constraint(tree->tree));
   //printf("IDateConstr %p: %ld\n", tree, pdt_verify_age_constraint(tree->tree));
}

/*-----------------------------------------------------------------------
//
// Function: PDTreeMatchPrefix()
//
//   Match the term against the tree and count matches/mismatches. Return
//   the last matched node. The term is in the tree iff remains == 0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PDTNode_p PDTreeMatchPrefix(PDTree_p tree, Term_p term, 
   long* matched, long* remains)
{
   Term_p    curr;
   PDTNode_p node, last, *next;

   assert(tree);
   assert(tree->tree);

   *matched = 0;
   *remains = 0;
   TermLRTraverseInit(tree->term_stack, term);
   node = tree->tree;
   last = node;
   curr = TermLRTraverseNext(tree->term_stack);
   while (curr)
   {
      if (!node) 
      {
         (*remains)++;
      }
      else 
      {
         next = pdt_select_alt_ref(tree, node, curr);
         if (!(*next))
         {
            (*remains)++;
            node = NULL;
         }
         else {
            (*matched)++;
            node = *next;
            last = node;
         }
      }
      curr = TermLRTraverseNext(tree->term_stack);
   }

   return last;
}

/*-----------------------------------------------------------------------
//
// Function: PDTreeDelete()
//
//   Delete all index entries of clause indexed by term from
//   tree. Return number of entries deleted.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

long PDTreeDelete(PDTree_p tree, Term_p term, Clause_p clause)
{
   long res;
   PStack_p  del_stack = PStackAlloc();
   Term_p    curr;
   PDTNode_p node, prev, *next, *del;

   assert(tree);
   assert(tree->tree);
   assert(term);
   assert(clause);

   TermLRTraverseInit(tree->term_stack, term);
   node = tree->tree;
   curr = TermLRTraverseNext(tree->term_stack);

   while(curr)
   {
      // ignore applied var funcode again
      if(TermIsAppliedVar(curr))
      {
         curr = TermLRTraverseNext(tree->term_stack);
         continue;
      }
      next = pdt_select_alt_ref(tree, node, curr);
      assert(next);
      PStackPushP(del_stack, next);

      node = *next;
      curr = TermLRTraverseNext(tree->term_stack);
   }
   assert(node);

   res = delete_clause_entries(&(node->entries), clause);

   if(term->weight == node->size_constr)
   {
      node->size_constr = -1;
   }
   if(SysDateEqual(node->age_constr, clause->date))
   {
      node->age_constr = SysDateInvalidTime();
   }

   while(node->parent)
   {
      prev = node->parent;
      del  = PStackPopP(del_stack);
      node->ref_count -= res;
      if(!node->ref_count)
      {
         tree->arr_storage_est -= (IntMapStorage(node->f_alternatives)+
                                   PDArrayStorage(node->v_alternatives));

         pdtree_default_cell_free(node);
         tree->node_count--;
         *del = NULL;
      }
      else if(node->ref_count == PTreeNodes(node->entries))
      {
         node->leaf = true;
      }
      node = prev;

      if(term->weight == node->size_constr)
      {
         node->size_constr = -1;
      }
      if(SysDateEqual(node->age_constr, clause->date))
      {
         node->age_constr = SysDateInvalidTime();
      }
   }
   PStackFree(del_stack);
   tree->clause_count-=res;
   /* printf("...removed\n"); */

   // printf("DSizeConstr %p: %ld\n", tree, pdt_verify_size_constraint(tree->tree));
   // printf("DDateConstr %p: %ld\n", tree, pdt_verify_age_constraint(tree->tree));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PDTreeSearchInit()
//
//   Initialize a PDTree for searching for matching terms.
//
// Global Variables: -
//
// Side Effects    : Changes state of the tree.
//
/----------------------------------------------------------------------*/

void PDTreeSearchInit(PDTree_p tree, Term_p term, SysDate age_constr,
                      bool prefer_general)
{
   assert(!tree->term);

   TermLRTraverseInit(tree->term_stack, term);
   PStackReset(tree->term_proc);
   tree->tree_pos         = tree->tree;
   tree->prefer_general   = prefer_general?1:0;
   tree->tree->trav_count = PDT_NODE_INIT_VAL(tree);
   tree->term             = term;
   tree->term_date        = age_constr;
   assert(TermStandardWeight(term) == TermWeight(term,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   tree->term_weight      = TermStandardWeight(term);
   tree->match_count++;
}

/*-----------------------------------------------------------------------
//
// Function: PDTreeSearchExit()
//
//   Mark a PDTree as not currently used in a search.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDTreeSearchExit(PDTree_p tree)
{
   assert(tree->term);

   if(tree->store_stack)
   {
      PTreeTraverseExit(tree->store_stack);
      tree->store_stack = NULL;
   }
   tree->term = NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PDTreeFindNextIndexedLeaf()
//
//   Given a search state encoded in the tree and a (partial)
//   substitution, find the next leaf node and return it. Extend subst
//   to a suitable substitution.
//
// Global Variables: PDTPreferGeneral
//
// Side Effects    : Changes tree state
//
/----------------------------------------------------------------------*/

PDTNode_p PDTreeFindNextIndexedLeaf(PDTree_p tree, Subst_p subst)
{
   while(tree->tree_pos)
   {
      // if it is FOL problem, then if it has entries it is a leaf
      assert(problemType == PROBLEM_HO || !tree->tree_pos->entries || tree->tree_pos->leaf);
      if(!pdtree_verify_node_constr(tree)||
         (tree->tree_pos->trav_count==PDT_NODE_CLOSED(tree,tree->tree_pos)))
      {
         pdtree_backtrack(tree, subst);
      }
      else if(tree->tree_pos->leaf) /* Leaf node */
      {
         assert(tree->tree_pos->entries);
         tree->tree_pos->trav_count = PDT_NODE_CLOSED(tree,tree->tree_pos);
         break;
      }
      else
      {
         pdtree_forward(tree, subst);
         if(tree->tree_pos && tree->tree_pos->entries)
         {
            // take clauses from this node
            break;
         }
      }
   }
   return tree->tree_pos;
}

/*-----------------------------------------------------------------------
//
// Function: PDTreeFindNextDemodulator()
//
//   Return the next matching clause position in the tree search
//   represented by tree.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

MatchRes_p PDTreeFindNextDemodulator(PDTree_p tree, Subst_p subst)
{
   PTree_p res_cell = NULL;
   MatchRes_p mi = MatchResAlloc();

   assert(tree->tree_pos);
   while(tree->tree_pos)
   {
      if(tree->store_stack)
      {
         res_cell = PTreeTraverseNext(tree->store_stack);
         if(res_cell)
         {
            mi->remaining_args = PStackGetSP(tree->term_stack);
            mi->pos = res_cell->key;
            return mi;
         }
         else
         {
            PTreeTraverseExit(tree->store_stack);
            tree->store_stack = NULL;
         }
      }
      PDTreeFindNextIndexedLeaf(tree, subst);
      if(tree->tree_pos)
      {
         tree->store_stack =
            PTreeTraverseInit(tree->tree_pos->entries);
      }
   }

   MatchResFree(mi);
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PDTreePrint()
//
//   Print a PD tree in human-readable form (for debugging).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PDTreePrint(FILE* out, PDTree_p tree)
{
   pdt_node_print(out, tree->tree, 0);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
