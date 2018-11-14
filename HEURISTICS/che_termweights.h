/*-----------------------------------------------------------------------

  File  : che_termweights.h

  Author: Stephan Schulz, yan

  Contents
 
  Common functions for term-based clause evaluation heuristics.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

-----------------------------------------------------------------------*/

#ifndef CHE_TERMWEIGHTS

#define CHE_TERMWEIGHTS

#include <ccl_relevance.h>
#include <che_refinedweight.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Related terms set */
typedef enum
{
   RTSConjectureTerms = 0,           /* conjecture terms only */
   RTSConjectureSubterms = 1,        /* conjecture terms and subterms */
   RTSConjectureSubtermsTopGens = 2, /* subterms and top-level gens */
   RTSConjectureSubtermsAllGens = 3  /* all substerms generalizations */
}RelatedTermSet;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#define TERM_MAX_GENS 1000

PStack_p ComputeSubtermsGeneralizations(Term_p term, VarBank_p vars);
PStack_p ComputeTopGeneralizations(
   Term_p term, 
   VarBank_p vars, 
   Sig_p sig);
void FreeGeneralizations(PStack_p gens);

int TupleInit(FixedDArray_p cur);
int TupleNext(FixedDArray_p cur, FixedDArray_p max);
void TuplePrint(FixedDArray_p t);

void TBIncSubtermsFreqs(Term_p term, NumTree_p* freqs);
NumTree_p TBCountTermFreqs(TB_p bank);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

