/*-----------------------------------------------------------------------

  File  : che_treeweight.h

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture tree distance weight (Ted) from 
  [CICM'16/Sec.3].
  
  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

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

