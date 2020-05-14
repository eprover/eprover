/*-----------------------------------------------------------------------

File  : ccl_relevance.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code implementing some limited relevance analysis for function
  symbols and clauses/formulas.

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun May 31 11:20:27 CEST 2009
    New

-----------------------------------------------------------------------*/

#ifndef CCL_RELEVANCE

#define CCL_RELEVANCE

#include <clb_plist.h>
#include <ccl_findex.h>
#include <ccl_proofstate.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Data structure for computing the relevance of function symbols with
 * respect to a set of conjectures/goals */

typedef struct relevance_cell
{
   Sig_p     sig;

   PList_p   clauses_core;
   PList_p   formulas_core;

   PList_p   clauses_rest;
   PList_p   formulas_rest;

   FIndex_p  clauses_index;
   FIndex_p  formulas_index;

   long      max_level;
   PDArray_p fcode_relevance;
   PStack_p  new_codes;
   PStack_p  relevance_levels;
}RelevanceCell, *Relevance_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define RelevanceCellAlloc()    (RelevanceCell*)SizeMalloc(sizeof(RelevanceCell))
#define RelevanceCellFree(junk) SizeFree(junk, sizeof(RelevanceCell))


Relevance_p RelevanceAlloc(void);
void        RelevanceFree(Relevance_p junk);

void ClausePListPrint(FILE* out, PList_p list);
void FormulaPListPrint(FILE* out, PList_p list);

long        RelevanceDataInit(ProofState_p state, Relevance_p data);
Relevance_p RelevanceDataCompute(ProofState_p state);
long        ProofStateRelevancyProcess(ProofState_p state, long level);




#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
