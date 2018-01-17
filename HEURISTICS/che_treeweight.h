/*-----------------------------------------------------------------------

File  : che_treeweight.h

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

#ifndef CHE_TREEWEIGHT

#define CHE_TREEWEIGHT

#include <che_termweights.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct treeweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   VarBank_p vars;
   PStack_p terms;
   int ins_cost;
   int del_cost; 
   int ch_cost;

   TermWeightExtension_p twe;
   void   (*init_fun)(struct treeweightparamcell*);
}TreeWeightParamCell, *TreeWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TreeWeightParamCellAlloc() (TreeWeightParamCell*) \
        SizeMalloc(sizeof(TreeWeightParamCell))
#define TreeWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(TreeWeightParamCell))

TreeWeightParam_p TreeWeightParamAlloc(void);
void              TreeWeightParamFree(TreeWeightParam_p junk);


WFCB_p ConjectureTreeDistanceWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p ConjectureTreeDistanceWeightInit(
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

double ConjectureTreeDistanceWeightCompute(void* data, Clause_p clause);

void ConjectureTreeDistanceWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

