/*-----------------------------------------------------------------------

File  : ccl_efficent_subsumption_index.h

Author: Constantin Ruhdorfer

Contents

  Functions for clause abstraction.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCC_CLAUSE_ABSTRACTION
#define CCC_CLAUSE_ABSTRACTION

#include <ccl_fcvindexing.h>
#include <ccl_unitclause_index.h>
#include <cte_signature.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum {
   CONSTANT,
   SKOLEM
} AbstractionMode;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void RewriteConstants(Clause_p clause, TB_p target, PDArray_p constant_sorts);
void RewriteSkolemSymbols(Clause_p clause, TB_p target,
                          PDArray_p skolem_sym_lookup, Sig_p sig);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

Term_p RewriteSymbolsOnTerm(Term_p source, VarBank_p vars, DerefType deref,
                            PDArray_p look_up, AbstractionMode mode, Sig_p sig);
Term_p RewriteConstantsOnTermCell(Term_p source, PDArray_p constant_sorts);
Term_p RewriteSkolemsOnTermCell(Term_p source, PDArray_p constant_sorts, 
                                Sig_p sig);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif