/*-----------------------------------------------------------------------

File  : ccl_cnf.c

Author: Stephan Schulz

Contents
 
  Functions for CNF conversion of a FOF formula.

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 26 00:21:17 CET 2004
    New

-----------------------------------------------------------------------*/

#include "ccl_cnf.h"



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
// Function: FormulaSimplify()
//
//   Maximally (and destructively) simplify a formula using (primarily)
//   the simplification rules (from [NW:SmallCNF-2001]).
//
//   P | P => P    P | T => T     P | F => P
//   P & P => F    P & T => P     P & F -> F
//   ~T = F        ~F = Ta
//   P <-> P => T  P <-> F => F   P <-> T => T
//   P <~> P => F  P <~> F => P   P <~> T => ~P
//   P -> P => T   P -> T => T    P -> F => 
//   ...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Formula_p FormulaSimplify(Formula_p form, TB_p terms)
{
   if(FormulaHasSubForm1(form))
   {
      FormulaRefSimplify(form->arg1, terms);
   }
   if(FormulaHasSubForm2(form))
   {
      FormulaRefSimplify(form->arg2, terms);
   }
   switch(form->op)
   {
   case OpUNot:
         /* if(FormulaIsPropTrue(form->arg1))
         {
            form = FormulaPropConstantAlloc(terms, false);
         }
         else if(FormulaIsPropFalse(form->arg1))
         {
            form = FormulaPropConstantAlloc(terms, true);
            }*/
         /* When we have a literal formula, we always push the sign
            down into the (inherently equational) literal */
         if(FormulaIsLiteral(form->arg1))
         {
            Eqn_p tmp;
            tmp = EqnAlloc(form->arg1->special.literal->lterm, 
                           form->arg1->special.literal->rterm, 
                           terms,
                           !EqnIsPositive(form->arg1->special.literal));
            form = FormulaLitAlloc(tmp);
         }
         break;
   }
   return form;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


