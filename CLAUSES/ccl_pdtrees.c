/*-----------------------------------------------------------------------

File  : ccl_pdtrees.c

Author: Stephan Schulz

Contents
 
  Perfect discrimination trees for optimized rewriting.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 24 00:55:29 MET DST 1998
    New
<2> Sun Mar  4 21:39:27 CET 2001
    Completely rewritten

-----------------------------------------------------------------------*/

#include "ccl_pdtrees.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool PDTreePreferGeneral = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


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

   PDArrayFree(junk->f_alternatives);
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
      tree->arr_storage_est -= PDArrayStorage(node->f_alternatives);
      res = &(PDArrayElementP(node->f_alternatives,
			       term->f_code));
      tree->arr_storage_est += PDArrayStorage(node->f_alternatives);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: recompute_node_constraints()
//
//   Given a PDNode_p, recompute the contraints from the successor
//   constraints or stored clauses. Returns true, if the constraint
//   changed, false otherwise. Also recomputes max_var and max_fun.
//
// Global Variables: -
//
// Side Effects    : May change node constraints.
//
/----------------------------------------------------------------------*/

static bool recompute_node_constraints(PDTNode_p node)
{
   SysDate new_age  = SysDateCreationTime();
   long    new_size = LONG_MAX;
   bool    res = false;
   
   if(node->entries)
   {
      ClausePos_p entry;
      PTree_p  trav;
      PStack_p trav_stack;

      trav_stack = PTreeTraverseInit(node->entries);
      while((trav = PTreeTraverseNext(trav_stack)))
      {
	 entry = trav->key;
	 new_age = SysDateMaximum(entry->clause->date, new_age);
      }
      PTreeTraverseExit(trav_stack);
      /* This is a leaf node, size is fixed! */      
      new_size = node->size_constr;
   }
   else
   {
      FunCode i;
      PDTNode_p next;
      int tmpmax = 0;

      for(i=1; i<=node->max_fun; i++)
      {
	 next = PDArrayElementP(node->f_alternatives, i);
	 if(next)
	 {
	    new_age  = SysDateMaximum(new_age, next->age_constr);
	    new_size = MIN(new_size, next->size_constr);
	    tmpmax = i;
	 }
      }
      node->max_fun = tmpmax;
      tmpmax = 0;
      for(i=1; i<=node->max_var; i++)
      {
	 next = PDArrayElementP(node->v_alternatives, i);
	 if(next)
	 {
	    new_age  = SysDateMaximum(new_age, next->age_constr);
	    new_size = MIN(new_size, next->size_constr);
	    tmpmax = i;
	 }
      }
      node->max_var = tmpmax;
   }   
   if(SysDateCompare(new_age, node->age_constr)!=DateEqual)
   {
      res = true;
      node->age_constr = new_age;
   }
   if(new_size!=node->size_constr)
   {
      res = true;
      node->size_constr = new_size;
   }
   return res;
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
   /* Is largest term at or beyond node greater than the query term? */ 

   /* printf("Term: %6ld  Node: %6ld\n", tree->term_weight,
      tree->tree_pos->size_constr);*/
   
   if(tree->term_weight < tree->tree_pos->size_constr)
   {
      return false;
   } 

   /* Is the youngest clause stored at or beyond node younger than the
      query terms normal form date ? */

   if(SysDateCompare(tree->term_date,
		     tree->tree_pos->age_constr) != DateEarlier)
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
   FunCode   i = tree->tree_pos->trav_count;
   Term_p    term = PStackTopP(tree->term_stack);
   
   while(i<PDT_NODE_CLOSED(tree,handle))
   {
      if(((i==0)||(i>handle->max_var))&&!TermIsVar(term))
      {
	 next = PDArrayElementP(handle->f_alternatives,term->f_code);
	 i++;
	 if(next)
	 {
	    PStackPushP(tree->term_proc, term);
	    TermLRTraverseNext(tree->term_stack);
	    next->trav_count = PDT_NODE_INIT_VAL(tree);
	    next->bound      = false;
	    assert(!next->variable);
	    tree->tree_pos = next;
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
	    if((!next->variable->binding)&&(!TermCellQueryProp(term,TPPredPos)))
	    {
	       PStackDiscardTop(tree->term_stack);
	       SubstAddBinding(subst, next->variable, term);
	       next->trav_count   = PDT_NODE_INIT_VAL(tree);
	       next->bound        = true;
	       tree->tree_pos     = next;
	       tree->term_weight  -= (TermStandardWeight(term) -1);
#ifdef MEASURE_EXPENSIVE
	       tree->visited_count++;
#endif 
	       break;
	    }
	    else if(TBTermEqual(next->variable->binding,term))
	    {
	       PStackDiscardTop(tree->term_stack);
	       next->trav_count   = PDT_NODE_INIT_VAL(tree);
	       next->bound        = false;
	       tree->tree_pos     = next;
	       tree->term_weight  -= (TermStandardWeight(term) -1);
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
      tree->term_weight  += (TermStandardWeight(handle->variable->binding) -1);
      PStackPushP(tree->term_stack, handle->variable->binding);
      if(handle->bound)
      {
	 succ = SubstBacktrackSingle(subst);
	 assert(succ);
      }
   }
   else if(handle->parent)
   {
      Term_p t = PStackPopP(tree->term_proc);
      
      assert(t);
      TermLRTraversePrev(tree->term_stack,t);
			 
   }
   tree->tree_pos = handle->parent;
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

PDTree_p PDTreeAlloc(void)
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
   handle->prefer_general  = false; /* Not really necessary, it's
				      reinitialized in
                                      PDTreeSearchInit() anyways.*/ 
   handle->clause_count    = 0;
   handle->node_count      = 0; 
   handle->arr_storage_est = 0;
   handle->match_count     = 0;
   handle->visited_count   = 0;

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
   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTreeFree()...\n");

   assert(tree);
   PDTNodeFree(tree->tree);
   PStackFree(tree->term_stack);
   PStackFree(tree->term_proc);
   assert(!tree->store_stack);
   PDTreeCellFree(tree);
   DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTreeFree()\n");
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

   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTNodeAlloc() called\n");

   handle = PDTNodeCellAlloc();
   
   handle->f_alternatives = PDArrayAlloc(PDNODE_FUN_INIT_ALT,
					 PDNODE_FUN_GROW_ALT);
   handle->v_alternatives = PDArrayAlloc(PDNODE_VAR_INIT_ALT,
					 PDNODE_VAR_GROW_ALT);
   handle->max_var        = 0;
   handle->max_fun        = 0;
   handle->size_constr    = LONG_MAX;
   handle->age_constr     = SysDateCreationTime();
   handle->parent         = NULL;
   handle->ref_count      = 0;
   handle->entries        = NULL;
   handle->trav_count     = 0;
   handle->variable       = NULL;
   handle->bound          = false;
   
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
   ClausePos_p  tmp;
   PDTNode_p     subtree;
 
   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTNodeFree()...\n");
  
   for(i=1; i<=tree->max_fun; i++)
   {
      subtree = PDArrayElementP(tree->f_alternatives, i);
      if(subtree)
      {
         PDTNodeFree(subtree);
      }
   }
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

   DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTNodeFree()\n");
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
      assert(tmp == term->args[i]);
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

void PDTreeInsert(PDTree_p tree, ClausePos_p demod_side)
{
   Term_p    term, curr;
   PDTNode_p node, *next;
   bool      res;
   long      tmp;

   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTreeInsert()...\n");
   assert(tree);
   assert(tree->tree);
   assert(demod_side);
   
   term = ClausePosGetSide(demod_side);
   TermLRTraverseInit(tree->term_stack, term);
   node              = tree->tree;
   tmp = TermStandardWeight(term);
   node->size_constr = MIN(tmp, node->size_constr);
   node->age_constr  = SysDateMaximum(demod_side->clause->date,
				      node->age_constr);
   node->ref_count++;

   curr = TermLRTraverseNext(tree->term_stack);
   
   while(curr)
   {      
      next = pdt_select_alt_ref(tree, node, curr);

      if(!(*next))
      {
	 *next = PDTNodeAlloc();
         tree->arr_storage_est+= (PDArrayStorage((*next)->f_alternatives)+
                                  PDArrayStorage((*next)->v_alternatives));
	 (*next)->parent = node;
	 tree->node_count++;
	 if(TermIsVar(curr))
	 {
	    (*next)->variable = curr;
	    node->max_var = MAX(node->max_var, -curr->f_code);
	 }
	 else
	 {
	    node->max_fun = MAX(node->max_fun, curr->f_code);	    
	 }
      }
      node = *next;
      tmp = TermStandardWeight(term);
      node->size_constr = MIN(tmp, node->size_constr);
      node->age_constr  = SysDateMaximum(demod_side->clause->date,
					 node->age_constr);   
      node->ref_count++;
      curr = TermLRTraverseNext(tree->term_stack);
   }   
   assert(node);
   res = PTreeStore(&(node->entries), demod_side);
   tree->clause_count++;
   assert(res);   
   DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTreeInsert()\n");
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
   bool      constr_change = true;

   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTreeDelete()...\n");

   assert(tree);
   assert(tree->tree);
   assert(term);
   assert(clause); 
   
   /* printf("\nRemoving: ");
   ClausePrint(stdout, clause, true);
   if(clause->literals)
   {
      printf("-- term: ");
      TBPrintTerm(stdout, clause->literals->bank, term, true);
   }	 
   printf("\n");
   */

   TermLRTraverseInit(tree->term_stack, term);
   node = tree->tree;
   curr = TermLRTraverseNext(tree->term_stack);
   
   while(curr)
   {
      next = pdt_select_alt_ref(tree, node, curr);
      assert(next);
      PStackPushP(del_stack, next);

      node = *next;
      curr = TermLRTraverseNext(tree->term_stack);
   }   
   assert(node);
   
   res = delete_clause_entries(&(node->entries), clause);
   
   while(node->parent)
   {
      prev = node->parent;
      del  = PStackPopP(del_stack);
      node->ref_count -= res;
      if(!node->ref_count)
      {
         tree->arr_storage_est -= (PDArrayStorage(node->f_alternatives)+
                                   PDArrayStorage(node->v_alternatives));
	 pdtree_default_cell_free(node);
	 tree->node_count--;
	 *del = NULL;
      }
      node = prev;
      if(constr_change&&res!=(node->ref_count))
      {
	 constr_change = recompute_node_constraints(node);
      }
   }
   PStackFree(del_stack);
   DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTreeDelete()\n");
   tree->clause_count-=res;
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
   tree->prefer_general   = prefer_general;
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
   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTreeFindNextIndexedLeaf()...\n");
   while(tree->tree_pos)
   {
      if(!pdtree_verify_node_constr(tree)||
	 (tree->tree_pos->trav_count==PDT_NODE_CLOSED(tree,tree->tree_pos)))
      {
	 pdtree_backtrack(tree, subst);
      }
      else if(tree->tree_pos->entries) /* Leaf node */
      {
	 tree->tree_pos->trav_count = PDT_NODE_CLOSED(tree,tree->tree_pos);
	 break;
      }
      else
      {
	 pdtree_forward(tree, subst);
      }
   }
   DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTreeFindNextIndexedLeaf()\n");
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
   
   DEBUGMARK(PDT_INTERFACE_WATCH, "PDTreeFindNextDemodulator()...\n");
   assert(tree->tree_pos);
   while(tree->tree_pos)
   {
      if(tree->store_stack)
      {
	 res_cell = PTreeTraverseNext(tree->store_stack);
	 if(res_cell)
	 {
	    DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTreeFindNextDemodulator()+\n");
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
   DEBUGMARK(PDT_INTERFACE_WATCH, "...PDTreeFindNextDemodulator()-\n");
   return NULL;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/








