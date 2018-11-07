/*-----------------------------------------------------------------------

  File  : cte_termweightext.h

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

#ifndef CTE_TERMWEIGHTEXT

#define CTE_TERMWEIGHTEXT

#include "cte_termtypes.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Term weight extension styles */
typedef enum
{
   TWESimple = 0,         /* apply term function to a term */
   TWESubtermsSum = 1,    /* apply to all substerms and sum */
   TWESubtermsMax = 2     /* take max weight of all substerms */
}TermWeightExtenstionStyle;

/* Generic parametric term weight function */
typedef double (*TermWeightFun)(Term_p term, void* data);

/* Term weight externsion description */
typedef struct termweightextensioncell 
{
   double max_term_multiplier;     /* max term multiplier */
   double max_literal_multiplier;  /* max literal multilier */
   double pos_eq_multiplier;       /* pos. equality multiplier */

   TermWeightExtenstionStyle ext_style; /* extension style */
   TermWeightFun term_weight_fun;       /* term weight function */
   void* data;                          /* optional data param */
} TermWeightExtensionCell, *TermWeightExtension_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TermWeightExtensionCellAlloc() (TermWeightExtensionCell*)SizeMalloc(sizeof(TermWeightExtensionCell))
#define TermWeightExtensionCellFree(junk)         SizeFree(junk, sizeof(TermWeightExtensionCell))

TermWeightExtension_p TermWeightExtensionAlloc(
   double max_term_multiplier,
   double max_literal_multiplier,
   double pos_eq_multiplier,
   TermWeightExtenstionStyle ext_style,
   TermWeightFun term_weight_fun,
   void* data);

void TermWeightExtensionFree(TermWeightExtension_p junk);

double TermExtWeight(Term_p term, TermWeightExtension_p twe);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
