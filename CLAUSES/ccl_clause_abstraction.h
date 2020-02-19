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

void RewriteConstants(Clause_p clause, TB_p target, PDArray_p constant_sorts);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

Term_p RewriteConstantsOnTerm(Term_p source, VarBank_p vars, DerefType deref,
                              PDArray_p constant_sorts);
Term_p RewriteConstantsOnTermCell(Term_p source, PDArray_p constant_sorts);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif