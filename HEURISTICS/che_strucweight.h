/*-----------------------------------------------------------------------

  File  : che_strucweight.c

  Author: Stephan Schulz, yan

  Contents
 
  Iplementation of conjecture structural distance weight (Struc) 
  from [CICM'16/Sec.3].
  
  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

-----------------------------------------------------------------------*/

#ifndef CHE_STRUCWEIGHT

#define CHE_STRUCWEIGHT

#include <che_termweights.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct strucweightparamcell
{
   OCB_p        ocb;
   ProofState_p proofstate;

   VarNormStyle var_norm;
   RelatedTermSet rel_terms;

   PStack_p  terms;
   VarBank_p vars;
   double    var_mismatch;
   double    sym_mismatch;
   double    inst_factor;
   double    gen_factor;

   TermWeightExtension_p twe;
   void   (*init_fun)(struct strucweightparamcell*);
}StrucWeightParamCell, *StrucWeightParam_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define StrucWeightParamCellAlloc() (StrucWeightParamCell*) \
        SizeMalloc(sizeof(StrucWeightParamCell))
#define StrucWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(StrucWeightParamCell))

StrucWeightParam_p StrucWeightParamAlloc(void);
void              StrucWeightParamFree(StrucWeightParam_p junk);


WFCB_p ConjectureStrucDistanceWeightParse(
   Scanner_p in, 
   OCB_p ocb, 
   ProofState_p state);

WFCB_p ConjectureStrucDistanceWeightInit(
   ClausePrioFun prio_fun, 
   OCB_p ocb,
   ProofState_p proofstate,
   VarNormStyle var_norm,
   RelatedTermSet rel_terms,
   double var_mismatch,
   double sym_mismatch,
   double inst_factor,
   double gen_factor,
   TermWeightExtenstionStyle ext_style,
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_multiplier);

double ConjectureStrucDistanceWeightCompute(void* data, Clause_p clause);

void ConjectureStrucDistanceWeightExit(void* data);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

