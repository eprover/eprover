/*-----------------------------------------------------------------------

  File  : cte_termweightext.c

  Author: Stephan Schulz, yan

  Contents

  Generic extensions of term weight functions to clause weight functions

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

-----------------------------------------------------------------------*/


#include <float.h>
#include "cte_termweightext.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static double term_ext_weight_sum(Term_p term, TermWeightExtension_p twe)
{
   int i;
   double res;
   PStack_p stack;
   Term_p subterm;

   res = 0;
   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      res += twe->term_weight_fun(subterm, twe->data);

      if (!TermIsFreeVar(subterm))
      {
         for (i=0; i<subterm->arity; i++)
         {
            PStackPushP(stack, subterm->args[i]);
         }
      }
   }
   PStackFree(stack);
   
   return res;
}

static double term_ext_weight_max(Term_p term, TermWeightExtension_p twe)
{
   int i;
   double res;
   PStack_p stack;
   Term_p subterm;

   res = -DBL_MAX;
   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      res = MAX(res, twe->term_weight_fun(subterm, twe->data));

      if (!TermIsFreeVar(subterm))
      {
         for (i=0; i<subterm->arity; i++)
         {
            PStackPushP(stack, subterm->args[i]);
         }
      }
   }
   PStackFree(stack);
   
   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: TermWeightExtensionAlloc()
//
//   Allocate and initialize a new extension cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

TermWeightExtension_p TermWeightExtensionAlloc(
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_eq_multiplier,
   TermWeightExtenstionStyle ext_style,
   TermWeightFun term_weight_fun,
   void* data)
{
   TermWeightExtension_p handle = TermWeightExtensionCellAlloc();

   handle->max_term_multiplier = max_term_multiplier;
   handle->max_literal_multiplier = max_literal_multiplier;
   handle->pos_eq_multiplier = pos_eq_multiplier;
   handle->ext_style = ext_style;
   handle->term_weight_fun = term_weight_fun;
   handle->data = data;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermWeightExtensionFree()
//
//   Free an extension cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermWeightExtensionFree(TermWeightExtension_p junk)
{
   TermWeightExtensionCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: 
//
//   Compute the weight of a term using one of the extension styles:
//   1) simply apply the term weight function to the term
//   2) apply the term function to all subterms and sum results
//   3) apply to all the subterms and take the maximum
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double TermExtWeight(Term_p term, TermWeightExtension_p twe)
{
   switch (twe->ext_style) 
   {
      case TWESimple: return twe->term_weight_fun(term, twe->data);
      case TWESubtermsSum: return term_ext_weight_sum(term, twe);
      case TWESubtermsMax: return term_ext_weight_max(term, twe);
      default: Error("TermExtWeight: Unsupported evaluation extension style %d", 
         USAGE_ERROR, twe->ext_style);
   }

   return 0; // just to avoid a warning
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
