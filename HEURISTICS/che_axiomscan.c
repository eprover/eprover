/*-----------------------------------------------------------------------

File  : che_axiomscan.c

Author: Stephan Schulz

Contents

  Functions for recognizing axioms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#include "che_axiomscan.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


#define FAIL_ON(x) if(x){return 0;}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: DetectCommutativity()
//
//   If clause is a comutativity axiom for some function symbol,
//   return this symbol. Otherwise return 0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode DetectCommutativity(Clause_p clause)
{
   Eqn_p lit;

    /*printf("DetectCommutativity: ");
      ClausePrint(stdout,clause,true);printf("\n");*/

   FAIL_ON(!ClauseIsUnit(clause));
   lit = clause->literals;

   assert(lit);
   FAIL_ON(!EqnIsPositive(lit));
   FAIL_ON(TermIsPhonyApp(lit->lterm) || TermIsPhonyApp(lit->rterm));
   FAIL_ON(lit->lterm->arity != 2 || lit->rterm->arity != 2);
   
   FAIL_ON(!TermIsFreeVar(lit->lterm->args[0]) || !TermIsFreeVar(lit->lterm->args[1]));
   FAIL_ON(!TermIsFreeVar(lit->rterm->args[0]) || !TermIsFreeVar(lit->rterm->args[1]));

   FAIL_ON((lit->lterm->arity!=2)||
      (lit->lterm->f_code!=lit->rterm->f_code));
   // due to weight adjustment either arguement can 
   // be a pattern applied variable.
   FAIL_ON(TermIsLambda(lit->lterm->args[0]));
   FAIL_ON(TermIsLambda(lit->lterm->args[1]));
   FAIL_ON(TermIsLambda(lit->rterm->args[0]));
   FAIL_ON(TermIsLambda(lit->rterm->args[1]));
   assert(lit->lterm->args);
   assert(TermIsFreeVar(lit->lterm->args[0])); /* Otherwise default weight */
   assert(TermIsFreeVar(lit->lterm->args[1])); /* is borked */
   assert(lit->rterm->args);
   assert(TermIsFreeVar(lit->rterm->args[0]));
   assert(TermIsFreeVar(lit->rterm->args[1]));

   FAIL_ON(lit->lterm->args[0] == lit->lterm->args[1]);
   FAIL_ON((lit->lterm->args[0] != lit->rterm->args[1])||(lit->lterm->args[1] != lit->rterm->args[0]));

   return lit->lterm->f_code;
}



/*-----------------------------------------------------------------------
//
// Function: DetectAssociativity()
//
//  If clause is a associativity for some function symbol,
//  return this symbol. Otherwise return 0.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode DetectAssociativity(Clause_p clause)
{
   Eqn_p lit;
   Term_p lterm, rterm;
   FunCode v1, v2, v3;
   FunCode f;

   FAIL_ON(!ClauseIsUnit(clause));
   lit = clause->literals;

   assert(lit);
   FAIL_ON(!EqnIsPositive(lit));
   FAIL_ON((TermStandardWeight(lit->lterm)!=
       (2*DEFAULT_FWEIGHT+(3*DEFAULT_VWEIGHT)))||
      (TermStandardWeight(lit->lterm)!=
       (2*DEFAULT_FWEIGHT+(3*DEFAULT_VWEIGHT))));

   FAIL_ON(TermIsAppliedFreeVar(lit->lterm));
   FAIL_ON(TermIsLambda(lit->lterm));
   FAIL_ON(TermIsAppliedFreeVar(lit->rterm));
   FAIL_ON(TermIsLambda(lit->rterm));

   FAIL_ON(lit->lterm->f_code!=lit->rterm->f_code);
   FAIL_ON((lit->lterm->arity!=2));
   assert(lit->rterm->arity == 2);

   if(TermIsFreeVar(lit->lterm->args[0]))
   {
      rterm=lit->lterm;
      lterm=lit->rterm;
   }
   else
   {
      rterm=lit->rterm;
      lterm=lit->lterm;
   }
   f = lterm->f_code;
   FAIL_ON(f!=lterm->args[0]->f_code);
   FAIL_ON(lterm->args[0]->arity != 2);
   FAIL_ON(!TermIsFreeVar(lterm->args[0]->args[0]));
   FAIL_ON(!TermIsFreeVar(lterm->args[0]->args[1]));
   FAIL_ON(!TermIsFreeVar(lterm->args[1]));
   v1 = lterm->args[0]->args[0]->f_code;
   v2 = lterm->args[0]->args[1]->f_code;
   v3 = lterm->args[1]->f_code;
   FAIL_ON((v1==v2) || (v1==v3) || (v2==v3));
   /* Now  we know that the left hand side matches. We also know that
      the top symbol of the right hand side is f */
   FAIL_ON(f!=rterm->args[1]->f_code);
   FAIL_ON(v1!=rterm->args[0]->f_code);
   FAIL_ON(rterm->args[1]->arity != 2);
   FAIL_ON(v2!=rterm->args[1]->args[0]->f_code);
   FAIL_ON(v3!=rterm->args[1]->args[1]->f_code);

   return f;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseScanAC()
//
//   Enter AC properties induced by clause into sig. Return true if at
//   least a C-axiom has beed detected.
//
// Global Variables: -
//
// Side Effects    : Changes properties in sig
//
/----------------------------------------------------------------------*/

bool ClauseScanAC(Sig_p sig, Clause_p clause)
{
   FunCode f;

   f = DetectCommutativity(clause);
   if(f)
   {
      if(!SigQueryFuncProp(sig, f, FPCommutative))
      {
    SigSetFuncProp(sig, f, FPCommutative);
    PStackPushP(sig->ac_axioms, clause);
      }
      return true;
   }
   f = DetectAssociativity(clause);
   if(f)
   {
      if(!SigQueryFuncProp(sig, f, FPAssociative))
      {
    SigSetFuncProp(sig, f, FPAssociative);
    PStackPushP(sig->ac_axioms, clause);
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetScanAC()
//
//   Enter AC properties induced by clause set into sig. Return true
//   if at least a C-axiom has beed detected.
//
// Global Variables: -
//
// Side Effects    : Changes properties in sig
//
/----------------------------------------------------------------------*/

bool ClauseSetScanAC(Sig_p sig, ClauseSet_p set)
{
   bool res = false;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
     handle->succ)
   {
      res |= ClauseScanAC(sig, handle);
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


