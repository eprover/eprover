/*-----------------------------------------------------------------------

File  : cte_fp_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code implementing fingerprint indexing for terms.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Feb 28 22:49:34 CET 2010
    New

-----------------------------------------------------------------------*/

#include "cte_fp_index.h"
#include "cte_termfunc.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(IndexUnifTimer);
PERF_CTR_DEFINE(IndexMatchTimer);

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: fpindex_alternative()
//
//   Return the child indexed by key f_code in index if it exists,
//   NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FPTree_p fpindex_alternative(FPTree_p index, FunCode f_code)
{
   assert(index);

   if(!index->f_alternatives)
   {
      return NULL;
   }
   else
   {
      return IntMapGetVal(index->f_alternatives, f_code);
   }
}


/*-----------------------------------------------------------------------
//
// Function: fpindex_alternative_ref()
//
//   Return the address of the child pointer indexed by key f_code in
//   index (create it if its not already there).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FPTree_p* fpindex_alternative_ref(FPTree_p index, FunCode f_code)
{
   void *res;

   assert(index);

   if(!index->f_alternatives)
   {
      index->f_alternatives = IntMapAlloc();
   }
   res = IntMapGetRef(index->f_alternatives, f_code);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fpindex_extract_alt()
//
//   Return the the child pointer indexed by key f_code in
//   index and remove it from the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static FPTree_p fpindex_extract_alt(FPTree_p index, FunCode f_code)
{
   FPTree_p res = NULL;

   assert(index);

   if(index->f_alternatives)
   {
      res = IntMapDelKey(index->f_alternatives, f_code);
   }
   if(res)
   {
      index->count--;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: fpindex_rek_delete()
//
//   Delete the branches leading (only) to the leaf identified to the
//   given key, _if_ that node has no payload.
//
//   Return true if the current node should be deleted.
//
// Global Variables: -
//
// Side Effects    : Memory managment.
//
/----------------------------------------------------------------------*/

static bool fpindex_rek_delete(FPTree_p index, IndexFP_p key, int current)
{
   bool delete;

   if(!index)
   {
      return false;
   }
   if(current == key[0])
   {
      return index->payload==NULL;
   }
   delete = fpindex_rek_delete(fpindex_alternative(index, key[current]),
                               key,
                               current+1);
   if(delete)
   {
      FPTree_p junk = fpindex_extract_alt(index, key[current]);
      if(junk->f_alternatives)
      {
         IntMapFree(junk->f_alternatives);
      }
      FPTreeCellFree(junk);
   }
   return index->count==0;
}


/*-----------------------------------------------------------------------
//
// Function: fpindex_rek_find_unif()
//
//   Find (and push) all payloads from terms unification-compatible
//   with key.
//
// Global Variables: -
//
// Side Effects    : Memory managment.
//
/----------------------------------------------------------------------*/

static long fp_index_rek_find_unif(FPTree_p index, IndexFP_p key,
                                   Sig_p sig,
                                   int current, PStack_p collect)
{
   FunCode i = 0;
   long    res = 0;
   IntMapIter_p iter;
   long iter_start;
   FPTree_p child;

   if(!index)
   {
      return 0;
   }
   if(current == key[0])
   {
      PStackPushP(collect, index->payload);
      return 1;
   }
   if(key[current] > 0)
   {
      /* t|p is a function symbol, compatible with:
         - the same symbol or
         - any variable
         - or a below_var position */
      res += fp_index_rek_find_unif(fpindex_alternative(index, key[current]),
                                    key,
                                    sig,
                                    current+1,
                                    collect);
      if(SigSymbolUnifiesWithVar(sig, key[current]))
      {  /* Predicates can never unify with variables
               -- not true in full HO case
               -- temproraily enabled  */
         res += fp_index_rek_find_unif(fpindex_alternative(index, ANY_VAR),
                                       key,
                                       sig,
                                       current+1,
                                       collect);
         res += fp_index_rek_find_unif(fpindex_alternative(index, BELOW_VAR),
                                       key,
                                       sig,
                                       current+1,
                                       collect);
      }
   }
   else if(key[current] == NOT_IN_TERM)
   {
      /* Position does not exist in t or any instance -> it cannot
       * unify with an existing position. */
      res += fp_index_rek_find_unif(fpindex_alternative(index, NOT_IN_TERM),
                                    key,
                                    sig,
                                    current+1,
                                    collect);
      res += fp_index_rek_find_unif(fpindex_alternative(index, BELOW_VAR),
                                    key,
                                    sig,
                                    current+1,
                                    collect);
   }
   else if(key[current] == BELOW_VAR || key[current] == ANY_VAR)
   {
      /* t|p is a variable or below a variable -> all but NOT_IN_TERM
         can certainly be unified, NOT_IN_TERM can be unified if
         BELOW_VAR, but not if ANY_VAR (if t|p = X in the query term,
         the position must exist in the search term. But if t|p =
         BELOW_VAR in the query term, the instantiation of X may not
         have the p. */

      res += fp_index_rek_find_unif(fpindex_alternative(index, ANY_VAR),
                                    key,
                                    sig,
                                    current+1,
                                    collect);
      res += fp_index_rek_find_unif(fpindex_alternative(index, BELOW_VAR),
                                    key,
                                    sig,
                                    current+1,
                                    collect);

      iter_start = key[current] == BELOW_VAR? 0:1;
      iter = IntMapIterAlloc(index->f_alternatives, iter_start, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         assert(child);

         if(i<=0 || SigSymbolUnifiesWithVar(sig, i))
         {
            res += fp_index_rek_find_unif(child,
                                          key,
                                          sig,
                                          current+1,
                                          collect);
         }
      }
      IntMapIterFree(iter);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fpindex_rek_find_matchable()
//
//   Find (and push) all payloads from terms match-compatible
//   with key.
//
// Global Variables: -
//
// Side Effects    : Memory managment.
//
/----------------------------------------------------------------------*/


static long fp_index_rek_find_matchable(FPTree_p index, IndexFP_p key,
                                        Sig_p sig,
                                        int current, PStack_p collect)
{
   FunCode i = 0;
   long    res = 0;
   IntMapIter_p iter;
   long iter_start;
   FPTree_p child;

   if(!index)
   {
      return 0;
   }
   if(current == key[0])
   {
      PStackPushP(collect, index->payload);
      return 1;
   }
   if(key[current] > 0)
   {
      /* t|p is a function symbol, compatible with:
         - the same symbol  */
      res += fp_index_rek_find_matchable(fpindex_alternative(index, key[current]),
                                         key,
                                         sig,
                                         current+1,
                                         collect);
   }
   else if(key[current] == NOT_IN_TERM)
   {
      /* Position does not exist in t or any instance:
         - it cannot match an existing position
         - It also cannot  match below-var (?), though, as instantiation can
         introduce new excluded positions */
      res += fp_index_rek_find_matchable(fpindex_alternative(index, NOT_IN_TERM),
                                         key,
                                         sig,
                                         current+1,
                                         collect);
      //res += fp_index_rek_find_matchable(fpindex_alternative(index, BELOW_VAR),
      //key,
      //sig,
      //current+1,
      //collect);
   }
   else if(key[current] == BELOW_VAR || key[current] == ANY_VAR)
   {
      /* t|p is a variable or below a variable -> all but NOT_IN_TERM
         can certainly be matched, NOT_IN_TERM can be matched if
         BELOW_VAR, but not if ANY_VAR (if t|p = X in the query term,
         the position must exist in the search term. But if t|p =
         BELOW_VAR in the query term, the instantiation of X may not
         have the p. */

      res += fp_index_rek_find_matchable(fpindex_alternative(index, ANY_VAR),
                                         key,
                                         sig,
                                         current+1,
                                         collect);
      if(key[current] == BELOW_VAR)
      {
         res += fp_index_rek_find_matchable(fpindex_alternative(index, BELOW_VAR),
                                            key,
                                            sig,
                                            current+1,
                                            collect);
      }
      iter_start = key[current] == BELOW_VAR? 0:1;
      iter = IntMapIterAlloc(index->f_alternatives, iter_start, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         assert(child);

         if(i<=0 || SigSymbolUnifiesWithVar(sig, i))
         {
            res += fp_index_rek_find_matchable(child,
                                               key,
                                               sig,
                                               current+1,
                                               collect);
         }
      }
      IntMapIterFree(iter);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: fp_index_leaf_prt_size()
//
//   Print a leaf as the path leading to it and the number of direct
//   entries in the subterm.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void fp_index_leaf_prt_size(FILE* out, PStack_p stack, FPTree_p leaf)
{
   long tmp;
   fprintf(out, COMCHAR" ");
   PStackPrintInt(out, "%4ld.", stack);
   tmp =  PObjTreeNodes(leaf->payload);
   fprintf(out, ":%ld terms\n", tmp);
}


/*-----------------------------------------------------------------------
//
// Function: fp_index_tree_print()
//
//   Print an FP index tree. Return the number of leaves and (via
//   *entries), the number of entries.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static long fp_index_tree_print(FILE* out,
                                FPTree_p index,
                                PStack_p stack,
                                FPLeafPrintFun prtfun,
                                long *entries)
{
   long res = 0, tmp;
   IntMapIter_p iter;
   long         i=0;
   FPTree_p    child;

   if(index->payload)
   {
      res++;
      tmp =  PObjTreeNodes(index->payload);
      *entries += tmp;
      prtfun(out, stack, index);
   }
   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         PStackPushInt(stack, i);
         res+= fp_index_tree_print(out,
                                   child,
                                   stack,
                                   prtfun,
                                   entries);
         (void)PStackPopInt(stack);
      }
      IntMapIterFree(iter);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: fp_index_tree_collect_distrib()
//
//   Collect distribution information for an fp-tree. Return number of
//   nodes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long fp_index_tree_collect_distrib(FPTree_p index, PStack_p stack)
{
   IntMapIter_p iter;
   long         i = 0;
   FPTree_p     child;
   long         res = 1;

   if(index->payload)
   {
      PStackPushInt(stack, PObjTreeNodes(index->payload));
   }
   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         res += fp_index_tree_collect_distrib(child,
                                              stack);
      }
      IntMapIterFree(iter);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: fp_symbol()
//
//   Return the symbol of a given fingerprint sample.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static char* fp_symbol(Sig_p sig, FunCode symbol)
{
   switch(symbol)
   {
   case BELOW_VAR:
         return "B";
   case ANY_VAR:
         return "A";
   case NOT_IN_TERM:
         return "N";
   default:
         return SigFindName(sig, symbol);
   }
}

/*-----------------------------------------------------------------------
//
// Function: fp_index_tree_print_node()
//
//   Print a tree node in DOT notation. See below.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void fp_index_tree_print_node(FILE* out, FPTree_p index,
                               PStack_p stack, Sig_p sig)
{
   DStr_p label = DStrAlloc();
   PStackPointer i;
   char* del = "";
   FunCode symbol;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      DStrAppendStr(label, del);
      del = ", ";
      symbol = PStackElementInt(stack,i);
      DStrAppendStr(label, fp_symbol(sig, symbol));
   }
   fprintf(out, "   l%p [label=\"%s\"]\n", index, DStrView(label));

   DStrFree(label);
}

/*-----------------------------------------------------------------------
//
// Function: fp_index_tree_print_nodes()
//
//   Print all the nodes in the FP-Tree in DOT notation, using the
//   symbols (and symbol-codings) on the stack for the label.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void fp_index_tree_print_nodes(FILE* out, FPTree_p index,
                               PStack_p stack, Sig_p sig)
{
   IntMapIter_p iter;
   long         i = 0;
   FPTree_p    child;

   fp_index_tree_print_node(out, index, stack, sig);

   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         PStackPushInt(stack, i);
         fp_index_tree_print_nodes(out, child, stack, sig);
         (void)PStackPopInt(stack);
      }
      IntMapIterFree(iter);
   }
}


/*-----------------------------------------------------------------------
//
// Function: fp_index_tree_print_edges()
//
//   Print all the edges in the fp-tree in DOT notation.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void fp_index_tree_print_edges(FILE* out, FPTree_p index,
                               PStack_p stack, Sig_p sig)
{
   IntMapIter_p iter;
   long         i = 0;
   FPTree_p    child;

   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         fprintf(out, "   l%p -- l%p [label=%s]\n",
                 index, child, fp_symbol(sig, i));
         fp_index_tree_print_edges(out, child, stack, sig);
      }
      IntMapIterFree(iter);
   }
}

/*-----------------------------------------------------------------------
//
// Function: fp_index_collect_leaves()
//
//   Push all the leaves in index onto result.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void fp_index_collect_leaves(FPTree_p index, PStack_p result)
{
   IntMapIter_p iter;
   long         i = 0;
   FPTree_p    child;

   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         fp_index_collect_leaves(child, result);
      }
      IntMapIterFree(iter);
   }
   else
   {
      PStackPushP(result, index);
   }
}

/*-----------------------------------------------------------------------
//
// Function: fp_index_find_all()
//
//   Push all payloads in index onto stack. Return number of payloads
//   found.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

#ifdef NEVER_DEFINED
long fp_index_find_all(FPTree_p index, PStack_p collect)
{
   PStack_p stack = PStackAlloc();
   PStackPointer i;
   long res = 0;

   fp_index_collect_leaves(index, stack);
   for(i=0; i<PStackGetSP(stack); i++)
   {
      FPTree_p leaf;

      leaf = PStackElementP(stack, i);
      if(leaf->payload)
      {
         PStackPushP(collect, leaf->payload);
         res++;
      }
   }
   PStackFree(stack);
   return res;
}
#endif


/*-----------------------------------------------------------------------
//
// Function: GET_SYMBOL_ARITY()
//
//   Local macro for getting the effective arity of any top-level
//   symbol in a term (including variables).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define GET_SYMBOL_ARITY(sig, f_code) \
   (((f_code) > 0)?SigFindArity((sig), (f_code)):0)

/*-----------------------------------------------------------------------
//
// Function: dt_index_rek_find_matchable()
//
//   Find all leaves in index that are potentially matchable with the
//   term that is represented by key (key is the flat term version of
//   the query term). Push all payloads of leaves onto collect.
//
//   skip_term indicates how many complete (sub-)terms need to be
//   skipped to complete a term that corresponds to a variable in the
//   query.
//
//   If skip_term = 0: consume next symbol
//   Else: Go down all alternatives, for each skip_term is modified by
//   alternative->arity - 1
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long dt_index_rek_find_matchable(FPTree_p index,
                                 IndexFP_p key,
                                 Sig_p sig,
                                 int current,
                                 int skip_term,
                                 PStack_p collect)
{
   long res = 0;
   IntMapIter_p iter;
   long         i = 0;
   FPTree_p    child;

   if(!index)
   {
      return 0;
   }
   if(skip_term)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         //printf("Branch (%d) %s\n", skip_term,i>0?SigFindName(sig, i):"X");
         res += dt_index_rek_find_matchable(child,
                                            key,
                                            sig,
                                            current,
                                            skip_term-1+GET_SYMBOL_ARITY(sig,i),
                                            collect);
      }
      IntMapIterFree(iter);
   }
   else if(current == key[0])
   {
      PStackPushP(collect, index->payload);
      return 1;
   }
   else if(key[current] == ANY_VAR)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         if(i<=0 || SigSymbolUnifiesWithVar(sig, i))
         {
            //printf("Branch (%d) %s\n", skip_term,i>0?SigFindName(sig, i):"X");
            res += dt_index_rek_find_matchable(child,
                                               key,
                                               sig,
                                               current+1,
                                               GET_SYMBOL_ARITY(sig,i),
                                               collect);
         }
      }
      IntMapIterFree(iter);
   }
   else
   {
      child = fpindex_alternative(index, key[current]);
      //printf("Branch (%d) %s\n", skip_term, key[current]>0?SigFindName(sig,  key[current]):"X");
      res = dt_index_rek_find_matchable(child,
                                        key,
                                        sig,
                                        current+1,
                                        0,
                                        collect);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: dt_index_rek_find_unifiable()
//
//   Find all leaves in index that are potentially unifiable with the
//   term that is represented by key (key is the flat term version of
//   the query term). Push all payloads of leaves onto collect.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static long dt_index_rek_find_unifiable(FPTree_p index,
                                        IndexFP_p key,
                                        Sig_p sig,
                                        int current,
                                        int skip_term,
                                        int skip_key,
                                        PStack_p collect)
{
   long res = 0;
   IntMapIter_p iter;
   long         i = 0;
   FPTree_p    child;

   if(!index)
   {
      return 0;
   }
   if(skip_term)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         res += dt_index_rek_find_unifiable(child,
                                            key,
                                            sig,
                                            current,
                                            skip_term-1+GET_SYMBOL_ARITY(sig,i),
                                            0,
                                            collect);
      }
      IntMapIterFree(iter);
   }
   else if(skip_key)
   {
      res += dt_index_rek_find_unifiable(index,
                                         key,
                                         sig,
                                         current+1,
                                         0,
                                         skip_key-1+GET_SYMBOL_ARITY(sig,key[current]),
                                         collect);
   }
   else if(current == key[0])
   {
      PStackPushP(collect, index->payload);
      return 1;
   }
   else if(key[current] == ANY_VAR)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         if(i<=0 || SigSymbolUnifiesWithVar(sig, i))
         {
            res += dt_index_rek_find_unifiable(child,
                                               key,
                                               sig,
                                               current+1,
                                               GET_SYMBOL_ARITY(sig,i),
                                               0,
                                               collect);
         }
      }
      IntMapIterFree(iter);
   }
   else
   {
      child = fpindex_alternative(index, key[current]);
      res += dt_index_rek_find_unifiable(child,
                                         key,
                                         sig,
                                         current+1,
                                         0,
                                         0,
                                         collect);
      if(key[current] <= 0 || SigSymbolUnifiesWithVar(sig, key[current]))
      {
         child = fpindex_alternative(index, ANY_VAR);
         res += dt_index_rek_find_unifiable(child,
                                            key,
                                            sig,
                                            current+1,
                                            0,
                                            GET_SYMBOL_ARITY(sig, key[current]),
                                            collect);
      }

   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FPTreeAlloc()
//
//   Allocate an initialized FPTreeCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPTree_p FPTreeAlloc(void)
{
   FPTree_p handle = FPTreeCellAlloc();

   handle->f_alternatives = NULL;
   handle->count          = 0;
   handle->payload        = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeFree()
//
//   Free an FPTree tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPTreeFree(FPTree_p index, FPTreeFreeFun payload_free)
{
   IntMapIter_p iter;
   long         i;
   FPTree_p    child;

   if(index->payload)
   {
      payload_free(index->payload);
   }
   if(index->f_alternatives)
   {
      iter = IntMapIterAlloc(index->f_alternatives, BELOW_VAR, LONG_MAX);
      while((child=IntMapIterNext(iter, &i)))
      {
         assert(child);
         FPTreeFree(child, payload_free);
      }
      IntMapIterFree(iter);
      IntMapFree(index->f_alternatives);
   }
   FPTreeCellFree(index);
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeFind()
//
//   Find the leaf node corresponding to key in the index at
//   root. Return NULL if no such node exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FPTree_p FPTreeFind(FPTree_p root, IndexFP_p key)
{
   long i;
   FPTree_p res = root;

   for(i=1; (i<key[0]) && res; i++)
   {
      res = fpindex_alternative(res, key[i]);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeInsert()
//
//   Insert a node corrsponding to key into the index (if necessary)
//   and return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes the index
//
/----------------------------------------------------------------------*/

FPTree_p FPTreeInsert(FPTree_p root, IndexFP_p key)
{
   long i, current;
   FPTree_p res = root, *pos;

   for(i=1; (i<key[0]) && res; i++)
   {
      current = key[i];

      pos = fpindex_alternative_ref(root, current);
      root = *pos;
      if(!root)
      {
         root = FPTreeAlloc();
         *pos = root;
         res->count++;
      }
      res = root;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeDelete()
//
//   Delete a node corresponding to a key if it does not carry any
//   payload.
//
// Global Variables: -
//
// Side Effects    : Changes index, memory operations
//
/----------------------------------------------------------------------*/

void FPTreeDelete(FPTree_p root, IndexFP_p key)
{
   fpindex_rek_delete(root, key, 1);
}


/*-----------------------------------------------------------------------
//
// Function: FPTreeFindUnifiable()
//
//   Push all the payloads of nodes unification-compatible with the
//   given key onto the stack. Return number of payloads pushed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FPTreeFindUnifiable(FPTree_p root,
                         IndexFP_p key,
                         Sig_p sig,
                         PStack_p collect)
{
   long count = 0;

   count = fp_index_rek_find_unif(root, key, sig, 1, collect);

   return count;
}

/*-----------------------------------------------------------------------
//
// Function: FPTreeFindMatchable()
//
//   Push all the payloads of nodes match-compatible with the
//   given key onto the stack. Return number of payloads pushed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FPTreeFindMatchable(FPTree_p root,
                         IndexFP_p key,
                         Sig_p sig,
                         PStack_p collect)
{
   long count = 0;

   count = fp_index_rek_find_matchable(root, key, sig, 1, collect);

   return count;
}

/*-----------------------------------------------------------------------
//
// Function: FPIndexAlloc()
//
//   Alloc an FPIndex.
//
// Global Variables:
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPIndex_p FPIndexAlloc(FPIndexFunction fp_fun,
                       Sig_p sig,
                       FPTreeFreeFun payload_free)
{
   FPIndex_p handle = FPIndexCellAlloc();

   handle->fp_fun       = fp_fun;
   handle->sig          = sig;
   handle->payload_free = payload_free;
   handle->index        = FPTreeAlloc();

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: FPIndexFree()
//
//   Free an FPIndex.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPIndexFree(FPIndex_p index)
{
   FPTreeFree(index->index, index->payload_free);
   FPIndexCellFree(index);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFind()
//
//   Find the index tree node representing term (if any) and return it
//   (or NULL).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FPTree_p FPIndexFind(FPIndex_p index, Term_p term)
{
   IndexFP_p key = index->fp_fun(term);
   FPTree_p res = FPTreeFind(index->index, key);

   IndexFPFree(key);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FPIndexInsert()
//
//   Return a node representing term, creating it if necessary.
//
// Global Variables: -
//
// Side Effects    :  Memory operations
//
/----------------------------------------------------------------------*/

FPTree_p  FPIndexInsert(FPIndex_p index, Term_p term)
{
   IndexFP_p key = index->fp_fun(term);
   FPTree_p res = FPTreeInsert(index->index, key);

   IndexFPFree(key);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexDelete()
//
//   Delete the node representing term, unless it's still in use (per
//   the payload field).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPIndexDelete(FPIndex_p index, Term_p term)
{
   IndexFP_p key = index->fp_fun(term);

   FPTreeDelete(index->index, key);
   IndexFPFree(key);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFindUnifiable()
//
//   Return (via collect) all payloads of nodes representing
//   potentially unifiable terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long FPIndexFindUnifiable(FPIndex_p index, Term_p term, PStack_p collect)
{
   long res;
   IndexFP_p key;

   PERF_CTR_ENTRY(IndexUnifTimer);
   key = index->fp_fun(term);

   if(index->fp_fun == IndexDTCreate)
   {
      res = dt_index_rek_find_unifiable(index->index,
                                        key,
                                        index->sig,
                                        1,
                                        0,
                                        0,
                                        collect);
   }
   else
   {
      res = FPTreeFindUnifiable(index->index, key, index->sig, collect);
   }
   IndexFPFree(key);
   PERF_CTR_EXIT(IndexUnifTimer);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexFindMatchable()
//
//   Return (via collect) all payloads of nodes representing
//   potentially matchable terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long FPIndexFindMatchable(FPIndex_p index, Term_p term, PStack_p collect)
{
   long res;
   IndexFP_p key;

   PERF_CTR_ENTRY(IndexMatchTimer);
   key = index->fp_fun(term);

   if(index->fp_fun == IndexDTCreate)
   {
      //printf("Query: ");
      //TermPrint(stdout, term, index->sig, DEREF_NEVER);
      //printf("\n");
      res = dt_index_rek_find_matchable(index->index,
                                        key,
                                        index->sig,
                                        1,
                                        0,
                                        collect);
   }
   else
   {

      res = FPTreeFindMatchable(index->index, key, index->sig, collect);

   }
   IndexFPFree(key);
   PERF_CTR_EXIT(IndexMatchTimer);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexDistribPrint()
//
//   Print the pathes in the index and the number of stored terms at
//   each leaf of the FPTree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FPIndexDistribPrint(FILE* out, FPIndex_p index)
{
   long leaves, entries=0;
   PStack_p path = PStackAlloc();

   leaves = fp_index_tree_print(out, index->index, path, fp_index_leaf_prt_size,
                                &entries);
   fprintf(out, COMCHAR" %ld entries, %ld leaves, %f entries/leaf\n",
           entries, leaves, (double)entries/leaves);

   PStackFree(path);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexCollectDistrib()
//
//   Collect statistics for the node number and leaf term
//   distribution. Returns number of nodes directly, leaves and
//   average and standard deviation of terms/leaf via OUT
//   parameters.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long FPIndexCollectDistrib(FPIndex_p index, long* leaves, double* avg, double* stddev)
{
   long res;
   PStack_p dist_stack = PStackAlloc();

   res = fp_index_tree_collect_distrib(index->index, dist_stack);

   *avg    = PStackComputeAverage(dist_stack, stddev);
   *leaves = PStackGetSP(dist_stack);
   PStackFree(dist_stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexDistribDataPrint()
//
//   Collect and print statistics about the FP-Index.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FPIndexDistribDataPrint(FILE* out, FPIndex_p index)
{
   long   leaves = 0;
   long   nodes  = 0;
   double avg    = 0 ;
   double stddev = 0;

   if(index)
   {
      nodes = FPIndexCollectDistrib(index, &leaves, &avg, &stddev);
   }
   fprintf(out, "%5ld nodes, %5ld leaves, %6.2f+/-%4.3f terms/leaf",
           nodes, leaves, avg, stddev);
}

/*-----------------------------------------------------------------------
//
// Function: FPIndexPrint()
//
//   Print an FP-Index.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FPIndexPrint(FILE* out, FPIndex_p index, FPLeafPrintFun prtfun)
{
   PStack_p stack = PStackAlloc();
   long dummy = 0;

   fp_index_tree_print(out, index->index, stack, prtfun, &dummy);

   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexCollectLeaves()
//
//   Push all leaves of an FPIndex onto the result stack. Return
//   number of values pushed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FPIndexCollectLeaves(FPIndex_p index, PStack_p result)
{
   PStackPointer sp = PStackGetSP(result);

   fp_index_collect_leaves(index->index, result);
   return PStackGetSP(result)-sp;
}


/*-----------------------------------------------------------------------
//
// Function: FPIndexPrintDot()
//
//   Print an FP-Index as a dot graph.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FPIndexPrintDot(FILE* out, char* name, FPIndex_p index,
                     FPLeafPayloadPrint prt_leaf, Sig_p sig)
{
   PStack_p stack = PStackAlloc();
   PStackPointer i;

   fprintf(out, "graph %s{\n   rankdir=LR\n   nodesep=0.05\n", name);
   fp_index_tree_print_nodes(out, index->index, stack, sig);
   fp_index_tree_print_edges(out, index->index, stack, sig);

   FPIndexCollectLeaves(index, stack);
   for(i=0; i<PStackGetSP(stack); i++)
   {
      FPTree_p leaf;

      leaf = PStackElementP(stack, i);
      if(leaf->payload)
      {
         prt_leaf(out, leaf->payload, sig);
         fprintf(out, "   l%p -- t%p [ranksep=0.1]\n", leaf, leaf->payload);
      }
   }
   PStackReset(stack);

   fprintf(out, "}\n");

   PStackFree(stack);
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
