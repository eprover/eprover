/*-----------------------------------------------------------------------

File  : ccl_context_sr.h

Author: Stephan Schulz

Contents

  Declarations for functions implementing contextual simplify-reflect
  (or subsumption resolution in Vampire's terminology).

  C v L     C' v -L v R
  ---------------------   if s(C v L) = C' v L for some subst. s
  C' v R

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jul 13 01:51:56 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CCL_CONTEXT_SR

#define CCL_CONTEXT_SR

#include <ccl_subsumption.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


int  ClauseContextualSimplifyReflect(ClauseSet_p set, Clause_p clause);
long ClauseSetFindContextSRClauses(ClauseSet_p set, Clause_p clause,
               PStack_p res);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





