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
#include <clb_ptrees.h>
#include <cte_lambda.h>
#include "ccl_derivation.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool PDTreeUseAgeConstraints  = true;
bool PDTreeUseSizeConstraints = true;

#ifdef PDT_COUNT_NODES
unsigned long PDTNodeCounter = 0;
#endif

#define TermPCompare PCmpFun

TraversalState trav_order[] = {TRAVERSING_SYMBOLS, TRAVERSING_VARIABLES, DONE};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static long pdt_compute_size_constraint(PDTNode_p node);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

void print_t_stack(Sig_p sig, PStack_p stack)
{
   if(PStackEmpty(stack))
   {
      fprintf(stderr, " - ");
   }
   for(long i=PStackGetTopSP(stack); i>=0; i--)
   {
      TermPrintDbg(stderr, PStackElementP(stack, i), sig, DEREF_NEVER);
      fprintf(stderr, ", ");
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
   assert(!junk->entries);

   IntMapFree(junk->f_alternatives);
   PStackFree(junk->var_traverse_stack);
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
   bool added_objmap_node = false;

   if(TermIsTopLevelFreeVar(term))
   {
      res = (PObjMapGetRef(&node->v_alternatives, term, TermPCompare, &added_objmap_node));
      assert(node->v_alternatives);
   }
   else if(TermIsDBVar(term) || TermIsAppliedDBVar(term) || TermIsLambda(term))
   {
      term = TermIsDBVar(term) ? term : term->args[0];
      res = (PObjMapGetRef(&node->db_alternatives, term, TermPCompare, &added_objmap_node));
      assert(node->db_alternatives);
   }
   else
   {
      tree->arr_storage_est -= IntMapStorage(node->f_alternatives);
      res = IntMapGetRef(node->f_alternatives, term->f_code);
      tree->arr_storage_est += IntMapStorage(node->f_alternatives);
   }

   tree->arr_storage_est += added_objmap_node ? SizeOfPObjNode() : 0;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: pdt_select_next()
//
//   Find an alternative node based on the term given. The difference
//   between this function and pdt_select_alt_ref is that no node in
//   the underlying data structures will be created.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void* pdt_select_next(PDTree_p tree, PDTNode_p node, Term_p term)
{
   void* res;

   if(TermIsTopLevelFreeVar(term))
   {
      res = PObjMapFind(&node->v_alternatives, term, TermPCompare);
   }
   else if(TermIsDBVar(term) || TermIsAppliedDBVar(term) || TermIsLambda(term))
   {
      term = TermIsDBVar(term) ? term : term->args[0];
      res = PObjMapFind(&node->db_alternatives, term, TermPCompare);
   }
   else
   {
      tree->arr_storage_est -= IntMapStorage(node->f_alternatives);
      res = IntMapGetVal(node->f_alternatives, term->f_code);
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
   PDTNode_p next;

   IntMapIter_p iter = IntMapIterAlloc(node->f_alternatives, 0, LONG_MAX);
   FunCode dummy;
   while((next=IntMapIterNext(iter, &dummy)))
   {
      assert(next);
      PStackPushP(result, next);
   }
   IntMapIterFree(iter);

   PStack_p objmap_iter = PStackAlloc();
   objmap_iter = PObjMapTraverseInit(node->v_alternatives, objmap_iter);
   while((next = PObjMapTraverseNext(objmap_iter, NULL)))
   {
      PStackPushP(result, next);
   }
   PObjMapTraverseExit(objmap_iter);

   objmap_iter = PObjMapTraverseInit(node->db_alternatives, objmap_iter);
   while((next = PObjMapTraverseNext(objmap_iter, NULL)))
   {
      PStackPushP(result, next);
   }
   PObjMapTraverseExit(objmap_iter);
   PStackFree(objmap_iter);

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

static long  delete_clause_entries(PTree_p *root, Clause_p clause, Deleter deleter)
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
      ClausePosCellFreeWDeleter(pos, deleter);
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
      tree->term_date != PDTREE_IGNORE_NF_DATE &&
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
//   indicate this by setting tree->tree_pos->var_traverse_stack to
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
   TraversalState curr_state = tree->tree_pos->trav_state;
   Term_p    term = PStackGetSP(tree->term_stack) ? PStackTopP(tree->term_stack) : NULL;

   if(!term)
   {
      curr_state = tree->tree_pos->trav_state = DONE;
      PStackReset(tree->tree_pos->var_traverse_stack);
   }

   while(curr_state<DONE)
   {
      PStackPointer prev_binding = PStackGetSP(subst);
      if(trav_order[curr_state] == TRAVERSING_SYMBOLS)
      {
         tree->tree_pos->trav_state++;
         curr_state++;
         if(trav_order[curr_state] == TRAVERSING_VARIABLES)
         {
            PObjMapTraverseInit(tree->tree_pos->v_alternatives,
                                tree->tree_pos->var_traverse_stack);
         }
         if(!TermIsTopLevelFreeVar(term))
         {
            Term_p query =
               TermIsPhonyApp(term) ? (assert(TermIsDBVar(term->args[0])), term->args[0]):
                                      term;
            // using nested DB symbol for indentifying Lambda expression
            if(TermIsDBVar(query) || TermIsLambda(query))
            {
               next = PObjMapFind(&handle->db_alternatives,
                                  TermIsDBVar(query) ? query : query->args[0], TermPCompare);
            }
            else
            {
               next = IntMapGetVal(handle->f_alternatives,term->f_code);
            }
            if(next)
            {
               PStackPushP(tree->term_proc, term);
               TermLRTraverseNext(tree->term_stack);
               next->trav_state = 0;
               if(trav_order[0] == TRAVERSING_VARIABLES)
               {
                  PObjMapTraverseInit(next->v_alternatives, next->var_traverse_stack);
               }
               else
               {
                  PStackReset(next->var_traverse_stack);
               }
               next->prev_subst      = prev_binding;
               tree->tree_pos = next;
#ifdef MEASURE_EXPENSIVE
               tree->visited_count++;
#endif
               break;
            }
         }
      }
      else
      {
         assert(trav_order[curr_state] == TRAVERSING_VARIABLES);
         next = PObjMapTraverseNext(tree->tree_pos->var_traverse_stack, NULL);
         if(next)
         {
            assert(next->variable);
            bool success=false;
            if(next->variable->type == term->type)
            {
               Term_p hd_var =
                  TermIsAppliedFreeVar(next->variable) ? next->variable->args[0]
                                                       : next->variable;
               if(hd_var->binding)
               {
                  if(problemType == PROBLEM_FO)
                  {
                     success = (hd_var->binding == term);
                  }
                  else
                  {
                     /* if(!next->variable->owner_bank) */
                     /* { */
                     /*    printf("Evil term 1: "); */
                     /*    TermPrint(stdout, next->variable, tree->bank->sig, DEREF_NEVER); */
                     /*    printf("\n"); */
                     /* } */
                     success = SubstMatchComplete(next->variable, term, subst);
                  }
               }
               else
               {
                  /* if(!next->variable->owner_bank) */
                  /* { */
                  /*    printf("Evil term 2: "); */
                  /*    TermPrint(stdout, next->variable, tree->bank->sig, DEREF_NEVER); */
                  /*    printf(" in "); */
                  /*    TermPrint(stdout, tree->term, tree->bank->sig, DEREF_NEVER); */
                  /*    printf("\n"); */
                  /* } */
                  success = SubstMatchComplete(next->variable, term, subst);
               }

               if(success)
               {
                  PStackDiscardTop(tree->term_stack);
                  PStackPushP(tree->term_proc, term);
                  next->trav_state = 0;
                  if(trav_order[0] == TRAVERSING_VARIABLES)
                  {
                     PObjMapTraverseInit(next->v_alternatives, next->var_traverse_stack);
                  }
                  else
                  {
                     PStackReset(next->var_traverse_stack);
                  }
                  next->prev_subst   = prev_binding;
                  tree->tree_pos     = next;
                  tree->term_weight  -= (TermStandardWeight(term) -
                                         TermStandardWeight(next->variable));
#ifdef MEASURE_EXPENSIVE
                  tree->visited_count++;
#endif
                  assert(hd_var->binding);
                  assert(problemType != PROBLEM_FO || next->variable->binding);
                  break;
               }
            }
         }
         else
         {
            tree->tree_pos->trav_state++;
            curr_state++;
         }
      }
   }
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
   if(handle->variable && TermIsTopLevelFreeVar(handle->variable))
   {
      assert(!TermIsFreeVar(handle->variable) || handle->variable->binding);
      assert(!TermIsAppliedFreeVar(handle->variable) ||
               handle->variable->args[0]->binding);
      tree->term_weight  += (TermStandardWeight((Term_p)PStackTopP(tree->term_proc)) -
                              TermStandardWeight(handle->variable));
      PStackPushP(tree->term_stack, PStackPopP(tree->term_proc));
      SubstBacktrackToPos(subst, handle->prev_subst);
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

void pdt_node_print(FILE* out, PDTNode_p node, Sig_p sig, int level)
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

      fprintf(out, "%sinternal size=%ld age=%lu f_alts=%p, v_alts=%p, db_alts=%p, type=%d\n",
              IndentStr(2*level),
              node->size_constr,
              node->age_constr,
              node->f_alternatives,
              node->v_alternatives,
              node->db_alternatives,
              node->f_alternatives?node->f_alternatives->type:-1);

      iter = IntMapIterAlloc(node->f_alternatives, 0, LONG_MAX);
      while((next=IntMapIterNext(iter, &i)))
      {
         fprintf(out, "%sBranch(fcode) %s, %ld\n", IndentStr(2*level), SigFindName(sig, i), i);
         pdt_node_print(out, next, sig, level+1);
      }
      IntMapIterFree(iter);
      PStack_p mapiter = PStackAlloc();
      mapiter = PObjMapTraverseInit(node->v_alternatives, mapiter);
      while((next=PObjMapTraverseNext(mapiter, NULL)))
      {
         assert(next);
         fprintf(out, "%sBranch(var) ", IndentStr(2*level));
         if(TermIsFreeVar(next->variable))
         {
            fprintf(out, "%ld\n", -(next->variable->f_code));
         }
         else
         {
            TermPrint(out, next->variable, sig, DEREF_NEVER);
            fputs("\n", out);
         }
         pdt_node_print(out, next, sig, level+1);
      }
      PObjMapTraverseExit(mapiter);

      mapiter = PObjMapTraverseInit(node->db_alternatives, mapiter);
      while((next=PObjMapTraverseNext(mapiter, NULL)))
      {
         assert(next);
         assert(TermIsDBVar(next->variable));
         fprintf(out, "%sBranch(db) %ld", IndentStr(2*level), next->variable->f_code);
         pdt_node_print(out, next, sig, level+1);
      }
      PStackFree(mapiter);
   }
   else
   {
      fprintf(out, "strange. it is leaf, but has no entries.\n");
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PDTreeAllocWDeleter()
//
//   Allocate an empty, initialized PDTreeCell (including the
//   initial PDTNodeCell().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PDTree_p PDTreeAllocWDeleter(TB_p bank, Deleter deleter)
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
   handle->clause_count    = 0;
   handle->node_count      = 0;
   handle->arr_storage_est = 0;
   handle->match_count     = 0;
   handle->visited_count   = 0;
   handle->bank            = bank;
   handle->deleter         = deleter;

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
   PDTNodeFree(tree->tree, tree->deleter);
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
   handle->v_alternatives = NULL;
   handle->db_alternatives = NULL;
   handle->size_constr    = LONG_MAX;
   handle->age_constr     = SysDateCreationTime();
   handle->parent         = NULL;
   handle->ref_count      = 0;
   handle->entries        = NULL;
   handle->var_traverse_stack = PStackAlloc();
   handle->trav_state     = 0;
   handle->variable       = NULL;
   handle->prev_subst     = 0;
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

void PDTNodeFree(PDTNode_p tree, Deleter deleter)
{
   FunCode      i;
   IntMapIter_p iter;
   ClausePos_p  tmp;
   PDTNode_p     subtree;

   iter = IntMapIterAlloc(tree->f_alternatives, 0, LONG_MAX);
   while((subtree = IntMapIterNext(iter, &i)))
   {
      assert(subtree);
      PDTNodeFree(subtree, deleter);
   }
   IntMapIterFree(iter);
   PStack_p objmap_iter = PStackAlloc();
   objmap_iter = PObjMapTraverseInit(tree->v_alternatives, objmap_iter);
   while((subtree=PObjMapTraverseNext(objmap_iter, NULL)))
   {
      assert(subtree);
      PDTNodeFree(subtree, deleter);
   }
   PObjMapTraverseExit(objmap_iter);
   PObjMapFree(tree->v_alternatives);
   objmap_iter = PObjMapTraverseInit(tree->db_alternatives, objmap_iter);
   while((subtree=PObjMapTraverseNext(objmap_iter, NULL)))
   {
      assert(subtree);
      PDTNodeFree(subtree, deleter);
   }
   PObjMapTraverseExit(objmap_iter);
   PObjMapFree(tree->db_alternatives);
   while(tree->entries)
   {
      tmp = PTreeExtractRootKey(&tree->entries);
      ClausePosCellFreeWDeleter(tmp, deleter);
   }
   PStackFree(objmap_iter);
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
   PStackPushP(stack, term);
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
   // applied variables are not getting destructed
   if(!TermIsTopLevelFreeVar(handle))
   {
      // phony DB variable gets skipped over for lambdas
      // and the head DB variable gets skipped for applied db vars
      for(i = handle->arity-1;
          i >= (TermIsLambda(handle) || TermIsAppliedDBVar(handle)) ? 1 : 0; i--)
      {
         PStackPushP(stack, handle->args[i]);
      }
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

   for(i = (TermIsLambda(term) || TermIsAppliedDBVar(term) ? 1 : 0);
       i < term->arity; i++)
   {
      tmp = PStackPopP(stack);
      UNUSED(tmp); assert(tmp == term->args[i]);
   }
   PStackPushP(stack, term);

   return term;
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

bool PDTreeInsert(PDTree_p tree, ClausePos_p demod_side)
{
   Term_p term;

   assert(demod_side);
   term = ClausePosGetSide(demod_side);
   bool ans = PDTreeInsertTerm(tree, term, demod_side, true);
   return ans;
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

bool PDTreeInsertTerm(PDTree_p tree, Term_p term, ClausePos_p demod_side,
                      bool store_data)
{
   Term_p    curr;
   PDTNode_p node, *next;
   bool      res;
   long      tmp;

   assert(tree);
   assert(tree->tree);
   assert(!TermIsBetaReducible(term));

   if(TermIsNonFOPattern(term))
   {
      term = LambdaEtaExpandDB(tree->bank, term);
   }
   else
   {
      term = LambdaEtaReduceDB(tree->bank, term);
   }

   TermLRTraverseInit(tree->term_stack, term);
   node              = tree->tree;
   tmp               = TermStandardWeight(term);
   if(demod_side&&demod_side->clause&&(!SysDateIsInvalid(node->age_constr)))
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
      /* printf("Inserting: "); */
      /* TermPrint(stdout, curr, tree->bank->sig, DEREF_NEVER); */
      /* printf("\n"); */

      next = pdt_select_alt_ref(tree, node, curr);

      if(!(*next))
      {
         *next = PDTNodeAlloc();
         node->leaf = false;
         assert(!TermIsTopLevelAnyVar(curr) || node->v_alternatives || node->db_alternatives);

         // initally PObjMaps for v and db alternatives are null
         tree->arr_storage_est += (IntMapStorage((*next)->f_alternatives));
         (*next)->parent = node;
         tree->node_count++;// applied variables are not getting destructed
         if(TermIsTopLevelAnyVar(curr))
         {
            (*next)->variable = TermIsAppliedDBVar(curr) ? curr->args[0] : curr;
         }
      }
      node = *next;
      //assert(!node->variable || (TermIsFreeVar(curr) && node->variable->type == curr->type));
      tmp = TermStandardWeight(term);
      node->size_constr = MIN(tmp, node->size_constr);
      if(demod_side&&demod_side->clause&&(!SysDateIsInvalid(node->age_constr)))
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
   return true;
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
   PDTNode_p node, last, next;

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
         next = pdt_select_next(tree, node, curr);
         if (!(next))
         {
            (*remains)++;
            node = NULL;
         }
         else {
            (*matched)++;
            node = next;
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
   long res=0;
   Term_p    curr;
   PDTNode_p node, prev, *next;

   assert(tree);
   assert(tree->tree);
   assert(term);
   assert(clause);
   assert(!TermIsBetaReducible(term));

   if(TermIsNonFOPattern(term))
   {
      term = LambdaEtaExpandDB(tree->bank, term);
   }
   else
   {
      term = LambdaEtaReduceDB(tree->bank, term);
   }

   PStack_p  del_stack = PStackAlloc();
   TermLRTraverseInit(tree->term_stack, term);
   node = tree->tree;
   curr = TermLRTraverseNext(tree->term_stack);

   while(node && curr)
   {
      next = pdt_select_alt_ref(tree, node, curr);
      assert(next);
      PStackPushP(del_stack, curr);

      node = *next;
      // assert(node);
      curr = TermLRTraverseNext(tree->term_stack);
   }

   if (node)
   {
      res = delete_clause_entries(&(node->entries), clause, tree->deleter);
      assert(res);

      if(term->weight == node->size_constr)
      {
         node->size_constr = -1;
      }
      if(SysDateEqual(node->age_constr, clause->date))
      {
         node->age_constr = SysDateInvalidTime();
      }

      Term_p del_term;
      while(node->parent)
      {
         prev = node->parent;
         del_term = PStackPopP(del_stack);
         node->ref_count -= res;
         if(!node->ref_count)
         {
            tree->arr_storage_est -= (IntMapStorage(node->f_alternatives));

            tree->node_count--;
            if(TermIsTopLevelAnyVar(del_term) || TermIsLambda(del_term))
            {
               PObjMap_p* to_del =
                  TermIsTopLevelFreeVar(del_term) ? &(prev->v_alternatives) :
                                                   &(prev->db_alternatives);
               void* deleted =
                  PObjMapExtract(to_del,
                                 (TermIsLambda(del_term) || TermIsAppliedDBVar(del_term))
                                    ? del_term->args[0] : del_term,
                                 TermPCompare);
               UNUSED(deleted); assert(deleted);
               tree->arr_storage_est -= SizeOfPObjNode();
            }
            else
            {
               tree->arr_storage_est -= IntMapStorage(node->f_alternatives);
               void* deleted = IntMapDelKey(prev->f_alternatives, del_term->f_code);
               UNUSED(deleted); assert(deleted);
               tree->arr_storage_est += IntMapStorage(node->f_alternatives);
            }
            pdtree_default_cell_free(node);
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
   }
   else
   {
      PStackReset(tree->term_stack);
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

   if(TermIsNonFOPattern(term))
   {
      term = LambdaEtaExpandDB(tree->bank, term);
   }
   else
   {
      term = LambdaEtaReduceDB(tree->bank, term);
   }

   TermLRTraverseInit(tree->term_stack, term);
   PStackReset(tree->term_proc);
   tree->tree_pos         = tree->tree;
   tree->tree->trav_state = 0;
   if(prefer_general)
   {
      trav_order[0] = TRAVERSING_SYMBOLS;
      trav_order[1] = TRAVERSING_VARIABLES;
   }
   else
   {
      trav_order[0] = TRAVERSING_VARIABLES;
      trav_order[1] = TRAVERSING_SYMBOLS;
      PObjMapTraverseInit(tree->tree_pos->v_alternatives,
                          tree->tree_pos->var_traverse_stack);
   }
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
   // not all terms can be targets
   assert(problemType == PROBLEM_HO || tree->term);

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
         (tree->tree_pos->trav_state==PDT_NODE_CLOSED(tree,tree->tree_pos)))
      {
         pdtree_backtrack(tree, subst);
      }
      else if(tree->tree_pos->leaf && PStackEmpty(tree->term_stack)) /* Leaf node */
      {
         assert(tree->tree_pos->entries);
         tree->tree_pos->trav_state = DONE;
         break;
      }
      else
      {
         pdtree_forward(tree, subst);
         if(tree->tree_pos && tree->tree_pos->entries && PStackEmpty(tree->term_stack))
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

ClausePos_p PDTreeFindNextDemodulator(PDTree_p tree, Subst_p subst)
{
   PTree_p res_cell = NULL;

   while(tree->tree_pos)
   {
      if(tree->store_stack)
      {
         res_cell = PTreeTraverseNext(tree->store_stack);
         if(res_cell)
         {
            return res_cell->key;
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
   pdt_node_print(out, tree->tree, tree->bank->sig, 0);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
