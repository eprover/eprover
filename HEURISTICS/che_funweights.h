/*-----------------------------------------------------------------------

File  : che_funweights.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Heuristic weight functions dealing with individual weights for
  different symbols.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat May  7 20:57:21 CEST 2005
    New

-----------------------------------------------------------------------*/

#ifndef CHE_FUNWEIGHTS

#define CHE_FUNWEIGHTS

#include <che_refinedweight.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



typedef struct funweightparamcell
{
   /* Generic stuff, see Refinedweight() */
   OCB_p  ocb;
   double max_term_multiplier;
   double max_literal_multiplier;
   double pos_multiplier;
   long   vweight;

   /* Weights for non-conjecture symbols (by type) */
   long   fweight;
   long   pweight;

   /* Weights for conjecture-symbols (by type) */
   long   conj_fweight;
   long   conj_pweight;
   
   /* Actual encoding for the weights */
   long   flimit;
   long   *fweights;
}FunWeightParamCell, *FunWeightParam_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define FunWeightParamCellAlloc() (FunWeightParamCell*) \
        SizeMalloc(sizeof(FunWeightParamCell))
#define FunWeightParamCellFree(junk) \
        SizeFree(junk, sizeof(FunWeightParamCell))


WFCB_p ConjectureSymbolWeightInit(ClausePrioFun prio_fun, 
                                  OCB_p ocb,
                                  ClauseSet_p axioms,
                                  double max_term_multiplier,
                                  double max_literal_multiplier,
                                  double pos_multiplier,
                                  long   vweight,
                                  long   fweight,
                                  long   pweight,
                                  long   conj_fweight,
                                  long   conj_pweight);

WFCB_p ConjectureSymbolWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
                                   state);

double GenericFunWeightCompute(void* data, Clause_p clause);


void   GenericFunWeightExit(void* data);





#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





