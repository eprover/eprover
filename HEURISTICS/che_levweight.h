/*-----------------------------------------------------------------------

  File  : che_levweight.h

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture Levenstein distance weight (Lev) 
  from [CICM'16/Sec.3].
  
  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

-----------------------------------------------------------------------*/

#ifndef CHE_LEVWEIGHT

#define CHE_LEVWEIGHT

#include <che_termweights.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct levweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   VarBank_p vars;
   PStack_p codes;
   int ins_cost;
   int del_cost; 
   int ch_cost;

   TermWeightExtension_p twe;
   void   (*init_fun)(struct levweightparamcell*);
}LevWeightParamCell, *LevWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define LevWeightParamCellAlloc() (LevWeightParamCell*) \
        SizeMalloc(sizeof(LevWeightParamCell))
#define LevWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(LevWeightParamCell))

LevWeightParam_p LevWeightParamAlloc(void);
void              LevWeightParamFree(LevWeightParam_p junk);


WFCB_p ConjectureLevDistanceWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p ConjectureLevDistanceWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   int ins_cost,
   int del_cost, 
   int ch_cost,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier);

double ConjectureLevDistanceWeightCompute(void* data, Clause_p clause);

void ConjectureLevDistanceWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

