/*-----------------------------------------------------------------------

File  : che_termweights.h

Author: Jan Jakubuv

Contents
 
  Common functions for term-based clause evaluation heuristics.

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Mar 11 11:55:38 CET 2016
    New

-----------------------------------------------------------------------*/

#ifndef CHE_TERMWEIGHTS

#define CHE_TERMWEIGHTS

#include <ccl_relevance.h>
#include <che_refinedweight.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   RTSConjectureTerms = 0,
   RTSConjectureSubterms = 1,
   RTSConjectureSubtermsTopGens = 2,
   RTSConjectureSubtermsAllGens = 3
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

