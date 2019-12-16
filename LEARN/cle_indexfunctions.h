/*-----------------------------------------------------------------------

  File  : cle_indexfunctions.h

  Author: Stephan Schulz

  Contents

  Functions and data types realizing simple index functions.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Aug  4 15:36:51 MET DST 1999

  -----------------------------------------------------------------------*/

#ifndef CLE_INDEXFUNCTIONS

#define CLE_INDEXFUNCTIONS

#include <clb_simple_stuff.h>
#include <clb_objtrees.h>
#include <cle_patterns.h>
#include <cle_termtops.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   IndexNoIndex  =   0,
   IndexArity    =   1,
   IndexSymbol   =   2,
   IndexTop      =   4,
   IndexAltTop   =   8,
   IndexCSTop    =  16,
   IndexESTop    =  32,
   IndexIdentity =  64,
   IndexEmpty    = 128
}IndexType;


typedef struct indextermcell
{
   Term_p   term;         /* Usually has reference if malloced() */
   PatternSubst_p subst;  /* Shared, necessary for object-tree
                             comparison */
   long           key;    /* The returned index number */
}IndexTermCell, *IndexTerm_p;


/* Operations on index:

   - insert(term, patternsubst) -> value >=0,
   - find(term, patternsubst) -> value or -1

   All values should populate 0...max{values} somewhat densely

*/


typedef struct tsmindexcell
{
   long           ident;
   IndexType      type;
   int            depth;
   long           count;
   TB_p           bank;       /* Shared, only here for convenience */
   PatternSubst_p subst;      /* Ditto */
   union
   {
      PTree_p     t_index;    /* Map IndexTerms onto index number */
      NumTree_p   n_index;    /* Map f_codes onto number */
   }              tree;
}TSMIndexCell, *TSMIndex_p;

#define IndexDynamicDepth 0


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* IndexFunNames[];

#define IndexTermCellAlloc()    (IndexTermCell*)SizeMalloc(sizeof(IndexTermCell))
#define IndexTermCellFree(junk) SizeFree(junk, sizeof(IndexTermCell))

int GetIndexType(char* name);

IndexTerm_p IndexTermAlloc(Term_p term, PatternSubst_p subst, long
                           key);
void         IndexTermFree(IndexTerm_p junk, TB_p bank);

int          IndexTermCompareFun(const void* term1, const void* term2);

#define TSMIndexCellAlloc()    (TSMIndexCell*)SizeMalloc(sizeof(TSMIndexCell))
#define TSMIndexCellFree(junk) SizeFree(junk, sizeof(TSMIndexCell))

TSMIndex_p TSMIndexAlloc(IndexType type, int depth, TB_p bank,
                         PatternSubst_p subst);
void       TSMIndexFree(TSMIndex_p junk);

long       TSMIndexFind(TSMIndex_p index, Term_p term, PatternSubst_p
                        subst);
long       TSMIndexInsert(TSMIndex_p index, Term_p term);

void       TSMIndexPrint(FILE* out, TSMIndex_p index, int depth);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
