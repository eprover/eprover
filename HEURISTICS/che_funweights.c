/*-----------------------------------------------------------------------

File  : che_funweights.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Weight functions dealing with individual weights for individual
  function- and predicate symbols.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat May  7 21:22:32 CEST 2005
    New

-----------------------------------------------------------------------*/

#include "che_funweights.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: ConjectureSymbolWeightInit()
//
//   Return an initialized WFCB for FunWeight evaluation. This
//   gives different weights to conjecture predicates/function
//   symbols, and non-conjecture predicate/function symbols.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

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
                                  long   conj_pweight)
{
   FunWeightParam_p data = FunWeightParamCellAlloc();
   Clause_p handle;
   FunCode i;
   
   data->ocb                    = ocb;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   
   data->vweight                = vweight;
   data->fweight                = fweight;
   data->pweight                = pweight;
   data->conj_fweight           = conj_fweight;
   data->conj_fweight           = conj_pweight;

   data->flimit                 = ocb->sig->f_count+1;
   data->fweights               = SizeMalloc(data->flimit*sizeof(long));

   for(i=0;i<data->flimit; i++)
   {
      data->fweights[i] = 0;
   }
   for(handle=axioms->anchor->succ;
       handle!=axioms->anchor;
       handle = handle->succ)
   {
      if(ClauseQueryTPTPType(handle)==CPTypeNegConjecture)
      {
         printf("Conjecture clause found.\n");
         ClauseAddSymbolDistribution(handle, data->fweights);
      }
   }
   for(i=1;i<data->flimit; i++)
   {
      if(data->fweights[i] == 0)
      {
         data->fweights[i] = SigIsPredicate(ocb->sig, i)?pweight:fweight;
      }
      else
      {
         data->fweights[i] = SigIsPredicate(ocb->sig, i)?conj_pweight:conj_fweight;
      }   
   }   

   return WFCBAlloc(GenericFunWeightCompute, prio_fun,
                    GenericFunWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: ConjectureSymbolWeightParse()
//
//   Parse a funweight-weight function giving different weights to
//   conjecture symbols and other symbols.
//
// Global Variables: -
//
// Side Effects    : Via ConjectureFunWeightInit, I/O.
//
/----------------------------------------------------------------------*/

WFCB_p ConjectureSymbolWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
                                state)
{   
   ClausePrioFun prio_fun;
   int vweight, fweight, pweight, conj_fweight, conj_pweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   conj_fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   conj_pweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);
   
   return ConjectureSymbolWeightInit(prio_fun, 
                                     ocb,
                                     state->axioms,
                                     max_term_multiplier,
                                     max_literal_multiplier,
                                     pos_multiplier,
                                     vweight,
                                     fweight,
                                     pweight,
                                     conj_fweight,
                                     conj_pweight);
}


/*-----------------------------------------------------------------------
//
// Function: GenericFunWeightCompute()
//
//   Compute a clause weight as Refinedweight(), but use the function
//   symbol weights in data->fweights for individual values.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double GenericFunWeightCompute(void* data, Clause_p clause)
{
   FunWeightParam_p local = data;
   
   return ClauseFunWeight(clause, 
                          local->max_term_multiplier,
                          local->max_literal_multiplier,
                          local->pos_multiplier, 
                          local->vweight, 
                          local->flimit,
                          local->fweights,
                          local->fweight);
}



/*-----------------------------------------------------------------------
//
// Function: GenericFunWeightExit()
//
//   Free an FunWeightParamCell, including the optional weight array.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GenericFunWeightExit(void* data)
{
   FunWeightParam_p junk = data;
   
   if(junk->fweights)
   {
      assert(junk->flimit > 0);
      SizeFree(junk->fweights, sizeof(long)*junk->flimit);
   }   
   FunWeightParamCellFree(junk);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


