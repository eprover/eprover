/*-----------------------------------------------------------------------

File  : cte_fp_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Fingerprint based indexing of terms. A fingerprint is a extor of
  samples of symbols at different positions. The index is a try build
  over these vectors.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Feb 20 19:19:23 EET 2010
    New

-----------------------------------------------------------------------*/

#ifndef CTE_FP_INDEX

#define CTE_FP_INDEX

#include <clb_intmap.h>
#include <cte_idx_fp.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* This datatype is used for building fingerprint indexes for terms. A
 * fingerprint index is a trie over fingerprints. Fingerprints contain
 * the actual f_codes of certain sampled positions in a term, or the
 * values BELOW_VAR, ANY_VAR and NOT_IN_TERM to describe positions not
 * in the term and not in any possible instance of the term,
 * respectively. Function symbol
 * alternatives are handled in the obvious way. The value NOT_IN_TEM
 * is encoded  * ar f_alternatives[0]. */

typedef struct fp_index_cell
{
   IntMap_p             f_alternatives;   /* Function symbols */
   struct fp_index_cell *below_var;
   struct fp_index_cell *any_var;
   long                 count;
   void                 *payload;
}FPTreeCell, *FPTree_p;


typedef void (*FPTreeFreeFun)(void*);


/* Wrapper for the index */

typedef struct subterm_index_cell
{
   FPTree_p        index;
   FPIndexFunction fp_fun;
   FPTreeFreeFun   payload_free;
}FPIndexCell, *FPIndex_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FPINDEX_VAR_INIT_ALT 4
#define FPINDEX_VAR_GROW_ALT 4


#define FPTreeCellAlloc() (FPTreeCell*)SizeMalloc(sizeof(FPTreeCell))
#define FPTreeCellFree(junk)         SizeFree(junk, sizeof(FPTreeCell))


FPTree_p FPTreeAlloc();
void     FPTreeFree(FPTree_p index, FPTreeFreeFun payload_free);

#define  FPTreeChildNo(icell) ((icell)->f_count+(icell)->v_count)

FPTree_p FPTreeFind(FPTree_p root, IndexFP_p key);
FPTree_p FPTreeInsert(FPTree_p root, IndexFP_p key);
void     FPTreeDelete(FPTree_p root, IndexFP_p key);

long     FPTreeFindUnifiable(FPTree_p root, IndexFP_p key, PStack_p collect);
long     FPTreeFindMatchable(FPTree_p root, IndexFP_p key, PStack_p collect);




#define FPIndexCellAlloc() (FPIndexCell*)SizeMalloc(sizeof(FPIndexCell))
#define FPIndexCellFree(junk)         SizeFree(junk, sizeof(FPIndexCell))


FPIndex_p FPIndexAlloc(FPIndexFunction fp_fun, FPTreeFreeFun payload_free);
void      FPIndexFree(FPIndex_p index);

FPTree_p FPIndexFind(FPIndex_p index, Term_p term);
FPTree_p  FPIndexInsert(FPIndex_p index, Term_p term);
void      FPIndexDelete(FPIndex_p index, Term_p term);

long      FPIndexFindUnifiable(FPIndex_p index, Term_p term, PStack_p collect);
long      FPIndexFindMatchable(FPIndex_p index, Term_p term, PStack_p collect);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





