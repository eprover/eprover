/*-----------------------------------------------------------------------

File  : che_to_autoselect.h

Author: Stephan Schulz

Contents

  Functions dealing with the automatic selection of a (suitable?) term
  ordering.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Dec 31 17:39:46 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_TO_AUTOSELECT

#define CHE_TO_AUTOSELECT

#include <che_proofcontrol.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* This is kind of ugly, as I have most of the information already in
   the HeuristicParmsCell. I just don't like to lug all the other
   stuff around and confuse issues. There are functions for converting
   the relevant parts of HeuristicParmsCell and OrderParmsCell. */

typedef struct order_parms_cell
{
   TermOrdering      ordertype;
   TOWeightGenMethod to_weight_gen;
   TOPrecGenMethod   to_prec_gen;
   bool              no_lit_cmp;
   long              to_const_weight;
}OrderParmsCell, *OrderParms_p;

typedef double (*OrderEvaluationFun)(OCB_p ocb, ProofState_p state,
                 HeuristicParms_p parms);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define OrderParmsCellAlloc() \
   (OrderParmsCell*)SizeMalloc(sizeof(OrderParmsCell))
#define OrderParmsCellFree(junk) \
   SizeFree(junk, sizeof(OrderParmsCell))

void OrderParmsInitialize(HeuristicParms_p master, OrderParms_p slave);
void HeuristicParmsUpdate(OrderParms_p master, HeuristicParms_p slave);

/* For now, we will fix the evaluation parameters by #defines's in the
   .c file....if somebody wants to tinker with it, he or she is
   welcome to do it. Not me, and not now!  */

double OrderEvaluate(OCB_p ocb, ProofState_p state, HeuristicParms_p
           params);

bool   OrderNextType(OrderParms_p ordering);
bool   OrderNextWeightGen(OrderParms_p ordering);
bool   OrderNextPrecGen(OrderParms_p ordering);
bool   OrderNextConstWeight(OrderParms_p ordering);

bool   OrderNextOrdering(OrderParms_p ordering, OrderParms_p mask);

OCB_p  OrderFindOptimal(OrderParms_p mask, OrderEvaluationFun
         eval_fun, ProofState_p state, HeuristicParms_p
         params);

OCB_p  TOSelectOrdering(ProofState_p state, HeuristicParms_p params,
         SpecFeature_p specs);
OCB_p  TOCreateOrdering(ProofState_p state, OrderParms_p params, char*
         pre_precedence, char* pre_weights);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





