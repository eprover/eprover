/*-----------------------------------------------------------------------

  File  : cle_indexfunctions.c

  Author: Stephan Schulz

  Contents

  Functions and data types realizing simple index function for
  learning heuristics.

  Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Aug  4 18:10:31 MET DST 1999

  -----------------------------------------------------------------------*/

#include "cle_indexfunctions.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* IndexFunNames[] =
{
   "IndexNoIndex",
   "IndexDynamic",
   "IndexArity",
   "IndexSymbol",
   "IndexTop",
   "IndexAltTop",
   "IndexCSTop",
   "IndexESTop",
   "IndexIdentity",
   "IndexEmpty",
   NULL
};


static long index_counter = 0;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: any_term_top()
//
//   Return a term top as specified by the parameters.
//
// Global Variables: -
//
// Side Effects    : By calling the TermTop-Functions.
//
/----------------------------------------------------------------------*/

static Term_p any_term_top(IndexType type, Term_p term, int depth,
                           VarBank_p freshvars)
{
   Term_p result;

   switch(type)
   {
   case IndexTop:
         result = TermTop(term, depth, freshvars);
         break;
   case IndexAltTop:
         result = AltTermTop(term, depth, freshvars);
         break;
   case IndexCSTop:
         result = CSTermTop(term, depth, freshvars);
         break;
   case IndexESTop:
         result = ESTermTop(term, depth, freshvars);
         break;
   default:
         assert(false && "Wrong index type!");
         result = NULL; /* Stiffle warning */
         break;
   }
   return result;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: GetIndexType()
//
//   Given a string, return the proper index type or -1.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int GetIndexType(char* name)
{
   int position = StringIndex(name, IndexFunNames);

   if(position < 1)
   {
      return position;
   }
   if(position == 1)
   {
      return IndexArity | IndexSymbol | IndexTop | IndexAltTop |
         IndexCSTop | IndexESTop | IndexIdentity;
   }
   return 1 << (position-2);
}


/*-----------------------------------------------------------------------
//
// Function: GetIndexName()
//
//   Return the name of an IndexType.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* GetIndexName(IndexType type)
{
   switch(type)
   {
   case IndexNoIndex:
         return IndexFunNames[0];
   case IndexArity:
         return IndexFunNames[2];
   case IndexSymbol:
         return IndexFunNames[3];
   case IndexTop:
         return IndexFunNames[4];
   case IndexAltTop:
         return IndexFunNames[5];
   case IndexCSTop:
         return IndexFunNames[6];
   case IndexESTop:
         return IndexFunNames[7];
   case IndexIdentity:
         return IndexFunNames[8];
   case IndexEmpty:
         return IndexFunNames[9];
   default:
         return IndexFunNames[1];
   }
}


/*-----------------------------------------------------------------------
//
// Function: IndexTermAlloc()
//
//   Return an initialized index term.
//
// Global Variables: -
//
// Side Effects    : Memory operations, gets reference on term
//
/----------------------------------------------------------------------*/

IndexTerm_p IndexTermAlloc(Term_p term, PatternSubst_p subst, long
                           key)
{
   IndexTerm_p handle = IndexTermCellAlloc();

   handle->term = term;
   handle->subst = subst;
   handle->key = key;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: IndexTermFree()
//
//   Free a IndexTerm.
//
// Global Variables: -
//
// Side Effects    : Changes bank
//
/----------------------------------------------------------------------*/

void IndexTermFree(IndexTerm_p junk, TB_p bank)
{
   assert(junk);
   assert(bank);

   IndexTermCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: IndexTermCompareFun()
//
//   Compare two index terms (as patterns).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int IndexTermCompareFun(const void* term1, const void* term2)
{
   const IndexTerm_p t1 = (const IndexTerm_p) term1;
   const IndexTerm_p t2 = (const IndexTerm_p) term2;
   int res = 0;

   switch(PatternTermCompare(t1->subst, t1->term, t2->subst,
                             t2->term))
   {
   case to_uncomparable:
         assert(false && "Only total substitutions expected here!");
         break;
   case to_equal:
         break;
   case to_greater:
         res = 1;
         break;
   case to_lesser:
         res = -1;
         break;
   default:
         assert(false && "Unknown comparison result (?!?)");
         break;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TSMIndexAlloc()
//
//   Return an initialized index cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

TSMIndex_p TSMIndexAlloc(IndexType type, int depth, TB_p bank,
                         PatternSubst_p subst)
{
   TSMIndex_p handle = TSMIndexCellAlloc();
   assert(type != IndexNoIndex);

   handle->type = type;
   handle->depth = depth;
   handle->bank = bank;
   handle->subst = subst;
   handle->count = 0;
   handle->ident = index_counter++;

   switch(type)
   {
   case IndexArity:
         break;
   case IndexSymbol:
         assert(subst);
         handle->tree.n_index = NULL;
         break;
   case IndexTop:
   case IndexAltTop:
   case IndexCSTop:
   case IndexESTop:
         assert(depth > 0);
         assert(bank);
         assert(subst);
         handle->tree.t_index = NULL;
         break;
   case IndexIdentity:
         assert(bank);
         assert(subst);
         handle->tree.t_index = NULL;
         break;
   case IndexEmpty:
         break;
   default:
         assert(false && "Unknown index type");
         break;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TSMIndexFree()
//
//   Free a TSMIndex.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may change term bank.
//
/----------------------------------------------------------------------*/

void TSMIndexFree(TSMIndex_p junk)
{
   PStack_p stack;
   PTree_p  cell;
   assert(junk);

   switch(junk->type)
   {
   case IndexArity:
         break;
   case IndexSymbol:
         NumTreeFree(junk->tree.n_index);
         break;
   case IndexTop:
   case IndexAltTop:
   case IndexCSTop:
   case IndexESTop:
   case IndexIdentity:
         stack = PTreeTraverseInit(junk->tree.t_index);
         while((cell = PTreeTraverseNext(stack)))
         {
            IndexTermFree(cell->key, junk->bank);
         }
         PTreeTraverseExit(stack);
         PTreeFree(junk->tree.t_index);
         break;
   case IndexEmpty:
         break;
   default:
         assert(false && "Unknown index type");
         break;
   }
   TSMIndexCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: TSMIndexFind()
//
//   Return an index for term (-1 if no index exists);
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long TSMIndexFind(TSMIndex_p index, Term_p term, PatternSubst_p subst)
{
   long          res = -1, key;
   IndexTermCell query;
   IndexTerm_p   found;
   NumTree_p     cell1;
   PTree_p       cell2;

   switch(index->type)
   {
   case IndexArity:
         res = term->arity;
         index->count = MAX(index->count, res+1);
         break;
   case IndexSymbol:
         /* printf("=Find in %ld =\n", index->ident);
            TermPrint(stdout, term, index->bank->sig, DEREF_NEVER);
            printf("\n");
            PatternTermPrint(stdout, subst, term,
            index->bank->sig); */
         if(TermIsFreeVar(term) && VarIsAltVar(term))
         {
            key = term->f_code;
         }
         else
         {
            key = PatSymbValue(subst, term->f_code);
            assert(key);
         }
         cell1 = NumTreeFind(&(index->tree.n_index), key);
         if(cell1)
         {
            res = cell1->val1.i_val;
         }
         /* printf("\n===Key: %ld Res: %ld\n", key, res);  */
         break;
   case IndexTop:
   case IndexAltTop:
   case IndexCSTop:
   case IndexESTop:
         query.term  = any_term_top(index->type, term, index->depth,
                                    index->bank->vars);
         query.subst = subst;
         cell2 = PTreeObjFind(&(index->tree.t_index), &query,
                              IndexTermCompareFun);
         if(cell2)
         {
            found = cell2->key;
            res = found->key;
         }
         TermFree(query.term);
         break;
   case IndexIdentity:
         query.term  = term;
         query.subst = subst;
         cell2 = PTreeObjFind(&(index->tree.t_index), &query,
                              IndexTermCompareFun);
         if(cell2)
         {
            found = cell2->key;
            res = found->key;
         }
         break;
   case IndexEmpty:
         break;
   default:
         assert(false && "Unknown index type");
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMIndexInsert()
//
//   Insert the term/index association into the index. This duplicates
//   some stuff from TSMIndexFind() to save on term copies. The
//   patterns substitution is taken to be consistent for all terms
//   inserted or to be inserted into index and is not passed
//   along. Returns index assigned.
//
// Global Variables: -
//
// Side Effects    : Changes index
//
/----------------------------------------------------------------------*/

long TSMIndexInsert(TSMIndex_p index, Term_p term)
{
   long          res=-1, key;
   IndexTermCell query;
   IndexTerm_p   entry;
   PTree_p       cell2;
   Term_p        shared;
   NumTree_p     cell, old;

   switch(index->type)
   {
   case IndexArity:
         res = term->arity;
         index->count = MAX(index->count, res+1);
         break;
   case IndexSymbol:
         /* printf("\n=Insert in %ld - term %ld=\n", index->ident, term->entry_no);
            TermPrint(stdout, term, index->bank->sig, DEREF_NEVER);
            printf("\n");
            PatternTermPrint(stdout, index->subst, term,
            index->bank->sig); */
         if(TermIsFreeVar(term) && VarIsAltVar(term))
         {
            key = term->f_code;
         }
         else
         {
            key = PatSymbValue(index->subst, term->f_code);
            assert(key);
         }
         cell = NumTreeCellAlloc();
         cell->key = key;
         cell->val1.i_val = index->count;
         cell->val2.i_val = index->count;
         old = NumTreeInsert(&(index->tree.n_index), cell);
         if(old)
         {
            NumTreeCellFree(cell);
            res = old->val1.i_val;
         }
         else /* key was unknown, association stays...*/
         {
            res = index->count;
            index->count++;
         }
         /* printf("\n===Key: %ld Res: %ld\n", key, res); */
         break;
   case IndexTop:
   case IndexAltTop:
   case IndexCSTop:
   case IndexESTop:
         query.term  = any_term_top(index->type, term, index->depth,
                                    index->bank->vars);
         query.subst = index->subst;
         cell2 = PTreeObjFind(&(index->tree.t_index), &query,
                              IndexTermCompareFun);
         if(cell2)
         {
            entry = cell2->key;
            res = entry->key;
         }
         else
         {
            shared = TBInsert(index->bank, query.term, DEREF_NEVER);
            entry = IndexTermAlloc(shared, index->subst,
                                   index->count);
            res = index->count++;
            entry = PTreeObjStore(&(index->tree.t_index), entry,
                                  IndexTermCompareFun);
            assert(!entry);
         }
         TermFree(query.term);
         break;
   case IndexIdentity:
         query.term  = term;
         query.subst = index->subst;
         cell2 = PTreeObjFind(&(index->tree.t_index), &query,
                              IndexTermCompareFun);
         if(cell2)
         {
            entry = cell2->key;
            res = entry->key;
         }
         else
         {
            shared = TBInsert(index->bank, query.term, DEREF_NEVER);
            entry = IndexTermAlloc(shared, index->subst,
                                   index->count);
            res = index->count++;
            entry = PTreeObjStore(&(index->tree.t_index), entry,
                                  IndexTermCompareFun);
            assert(!entry);
         }
         break;
   case IndexEmpty:
         assert(false && "Cannot insert term into IndexEmpty index!");
         break;
   default:
         assert(false && "Unknown index type");
         res = 0; /* Stiffle warnings */
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMIndexPrint()
//
//   Print a TSM-Index (as a comment) in the form "keyno:keyobj"*
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TSMIndexPrint(FILE* out, TSMIndex_p index, int depth)
{
   PStack_p stack;
   NumTree_p ncell;
   PTree_p   pcell;
   IndexTerm_p current;
   long i;
   long f_code;
   char pattern[10];
   char pattern2[70];

   sprintf(pattern, "%%%ds", 3*depth);
   sprintf(pattern2, pattern, "");

   switch(index->type)
   {
   case IndexArity:
         fprintf(out, COMCHAR" %sIndex %ld is arity index!\n", pattern2, index->ident);
         break;
   case IndexSymbol:
         fprintf(out, COMCHAR" %sIndex %ld is symbol index!\n",
                 pattern2,index->ident);
         fprintf(out, COMCHAR" %sPSymbol         Index  FCode     (Symbol)\n", pattern2);
         stack = NumTreeTraverseInit(index->tree.n_index);
         i=0;
         while((ncell = NumTreeTraverseNext(stack)))
         {
            f_code = PatternSubstGetOriginalSymbol(index->subst,
                                                   ncell->key);

            fprintf(out, COMCHAR" %s#%10ld :%7ld  %7ld     %s\n", pattern2,
                    ncell->key,
                    ncell->val1.i_val,
                    f_code,
                    ((f_code > 0))&&(f_code<=index->bank->sig->f_count)?
                    SigFindName(index->bank->sig, f_code):"variable");
            i++;
         }
         NumTreeTraverseExit(stack);
         fprintf(out, COMCHAR" %s%ld alternatives in the index\n",
                 pattern2, i);
         break;
   case IndexTop:
   case IndexAltTop:
   case IndexCSTop:
   case IndexESTop:
   case IndexIdentity:
         fprintf(out, COMCHAR" Index is %s index!\n",
                 GetIndexName(index->type));
         stack = PTreeTraverseInit(index->tree.t_index);
         i=0;
         while((pcell = PTreeTraverseNext(stack)))
         {
            current = pcell->key;
            fprintf(out, COMCHAR" %3ld : ", current->key);
            PatternTermPrint(out, index->subst, current->term, index->bank->sig);
            /* TermPrint(out, current->term, index->bank->sig,
               DEREF_NEVER); */
            fputc('\n', out);
            i++;
         }
         PTreeTraverseExit(stack);
         fprintf(out, COMCHAR" %ld alternatives in the index\n", i);
         break;
   case IndexEmpty:
         fprintf(out, COMCHAR" Index is empty index!\n");
         break;
   default:
         assert(false && "Unknown index type");
         break;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
