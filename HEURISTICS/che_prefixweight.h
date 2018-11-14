/*-----------------------------------------------------------------------

  File  : che_prefixweight.h

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture term prefix weight (Pref) from 
  [CICM'16/Sec.3].

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

-----------------------------------------------------------------------*/

#ifndef CHE_PREFIXWEIGHT

#define CHE_PREFIXWEIGHT

#include <che_termweights.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct prefixweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   PDTree_p  terms;
   VarBank_p vars;
   double    match_weight;
   double    miss_weight;

   TermWeightExtension_p twe;
   void   (*init_fun)(struct prefixweightparamcell*);
}PrefixWeightParamCell, *PrefixWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PrefixWeightParamCellAlloc() (PrefixWeightParamCell*) \
        SizeMalloc(sizeof(PrefixWeightParamCell))
#define PrefixWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(PrefixWeightParamCell))

PrefixWeightParam_p PrefixWeightParamAlloc(void);
void              PrefixWeightParamFree(PrefixWeightParam_p junk);


WFCB_p ConjectureTermPrefixWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p ConjectureTermPrefixWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   double match_weight,
   double miss_weight,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier);

double ConjectureTermPrefixWeightCompute(void* data, Clause_p clause);

void ConjectureTermPrefixWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

