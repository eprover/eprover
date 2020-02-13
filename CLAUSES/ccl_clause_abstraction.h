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

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef long REWRITE_CONSTANT;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void RewriteConstants(Clause_p clause);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

Term_p RewriteConstantsOnTerm(Term_p source, VarBank_p vars, 
                              DerefType deref);
Term_p RewriteConstantsOnTermCell(Term_p source);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif