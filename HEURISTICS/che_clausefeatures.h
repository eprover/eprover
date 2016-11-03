/*-----------------------------------------------------------------------

File  : che_clausefeatures.h

Author: Stephan Schulz

Contents

  Functions for determining various features of clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Sep 28 19:17:50 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_CLAUSEFEATURES

#define CHE_CLAUSEFEATURES

#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


int  ClauseCountExtSymbols(Clause_p clause, Sig_p sig, long
            min_arity);

FunCode TermAddVarDistribution(Term_p term, PDArray_p dist_array);
FunCode EqnAddVarDistribution(Eqn_p eqn, PDArray_p dist_array);
FunCode EqnListAddVarDistribution(Eqn_p list, PDArray_p dist_array);
#define ClauseAddVarDistribution(clause, dist_array)\
        EqnListAddVarDistribution((clause)->literals, (dist_array))

long ClauseCountVariableSet(Clause_p clause);
long ClauseCountSingletonSet(Clause_p clause);

long ClauseCountMaximalTerms(Clause_p clause);
long ClauseCountMaximalLiterals(Clause_p clause);
long ClauseCountUnorientableLiterals(Clause_p clause);
long ClauseTPTPDepthInfoAdd(Clause_p clause, long* depthmax, long*
             depthsum, long* count);

void ClauseInfoPrint(FILE* out, Clause_p clause);
void ClauseLinePrint(FILE* out, Clause_p clause, bool printinfo);

void ClausePropInfoPrint(FILE* out, Clause_p clause);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

















