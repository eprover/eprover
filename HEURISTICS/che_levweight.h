/*-----------------------------------------------------------------------

File  : che_levweight.h

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

