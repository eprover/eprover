/*-----------------------------------------------------------------------

  File  : che_termweight.h

  Author: Stephan Schulz, yan

  Contents

  Iplementation of conjecture subterm weight (Term) from [CICM'16/Sec.3].

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018


-----------------------------------------------------------------------*/

#ifndef CHE_TERMWEIGHT

#define CHE_TERMWEIGHT

#include <che_termweights.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct termweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   TB_p   eval_bank;
   NumTree_p eval_freqs;
   long   vweight;
   long   fweight;
   long   cweight;
   long   pweight;
   long   conj_fweight;
   long   conj_cweight;
   long   conj_pweight;

   TermWeightExtension_p twe;
   void   (*init_fun)(struct termweightparamcell*);
}TermWeightParamCell, *TermWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TermWeightParamCellAlloc() (TermWeightParamCell*) \
        SizeMalloc(sizeof(TermWeightParamCell))
#define TermWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(TermWeightParamCell))

TermWeightParam_p TermWeightParamAlloc(void);
void              TermWeightParamFree(TermWeightParam_p junk);


WFCB_p ConjectureRelativeTermWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p ConjectureRelativeTermWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   long   vweight,
   long   fweight,
   long   cweight,
   long   pweight,
   long   conj_fweight,
   long   conj_cweight,
   long   conj_pweight,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier);

double ConjectureRelativeTermWeightCompute(void* data, Clause_p clause);

void ConjectureRelativeTermWeightExit(void* data);

void TBInsertClauseTermsNormalized(
   TB_p bank, 
   Clause_p clause, 
   VarNormStyle var_norm, 
   RelatedTermSet rel_terms);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

