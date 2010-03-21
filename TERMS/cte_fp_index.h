/*-----------------------------------------------------------------------

File  : cte_fp_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Fingerprint based indexing of terms. A fingerprint is a extor of
  samples of symbols at different positions. The index is a try build
  over these vectors.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
 * values LONG_MIN and 0 to describe positions not in the term and
 * not in any possible instance of the term,
 * respectively. Variable alternatives and function symbol
 * alternatives are handled in the obvious way. The value 0 is encoded
 * ar f_alternatives[0], the value LONG_MIN in v_alternatives[0]. */

typedef struct fp_index_cell
{
   IntMap_p           f_alternatives;   /* Function symbols */
   long               f_count;
   PDArray_p          v_alternatives;   /* Variables */
   long               v_count;
   long               max_var;
   void               *payload;
}FPIndexCell, *FPIndex_p;

typedef void (*FPIndexFreeFun)(void*);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FPINDEX_VAR_INIT_ALT 4
#define FPINDEX_VAR_GROW_ALT 4


#define FPIndexCellAlloc() (FPIndexCell*)SizeMalloc(sizeof(FPIndexCell))
#define FPIndexCellFree(junk)         SizeFree(junk, sizeof(FPIndexCell))


FPIndex_p FPIndexAlloc();
void      FPIndexFree(FPIndex_p index, FPIndexFreeFun payload_free);

FPIndex_p FPIndexFind(FPIndex_p root, IndexFP_p key);
FPIndex_p FPIndexInsert(FPIndex_p root, IndexFP_p key);
void      FPIndexDelete(FPIndex_p root, IndexFP_p key);

long      FPIndexFindUnifiable(FPIndex_p root, IndexFP_p key, PStack_p res);
long      FPIndexFindMatchable(FPIndex_p root, IndexFP_p key, PStack_p res);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





