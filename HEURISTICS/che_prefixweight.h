/*-----------------------------------------------------------------------

File  : che_prefixweight.h

Author: could be anyone

Contents
 
  Auto generated. Your comment goes here ;-).

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar  8 22:40:31 CET 2016
    New

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

