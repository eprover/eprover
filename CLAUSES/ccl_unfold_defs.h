/*-----------------------------------------------------------------------

  File  : ccl_unfold_defs.h

  Author: Stephan Schulz

  Contents

  Functions used for unfolding equational definitions (sometimes also
  called "demodulating", but that term seems to be seriously
  overloaded). This is basically a special case of rewriting. However,
  the application is sufficiently different to warrant separate
  implementation. It also is not shared (shame on me), but then it
  also is quite cheap and will be applied very rarely.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Wed Aug 14 19:54:01 CEST 2002

  -----------------------------------------------------------------------*/

#ifndef CCL_UNFOLD_DEFS

#define CCL_UNFOLD_DEFS

#include "ccl_clausefunc.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* How much is a definition allowed to increase term size to be still
 * applicable? */
#define DEFAULT_EQDEF_INCRLIMIT 20

/* When to not even try unfilding during preprocessing? */
#define DEFAULT_EQDEF_MAXCLAUSES 20000

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* We'll not export term and clause versions, because we need the
   clause information for proof output anyways */

bool ClauseUnfoldEqDef(Clause_p clause, ClausePos_p demod, Term_p lside, Term_p rside);
bool ClauseSetUnfoldEqDef(ClauseSet_p set, ClausePos_p demod);
long ClauseSetUnfoldAllEqDefs(ClauseSet_p set, ClauseSet_p passive,
                              ClauseSet_p archive,
                              int min_arity, long eqdef_incrlimit);

long ClauseSetPreprocess(ClauseSet_p set, ClauseSet_p passive,
                         ClauseSet_p archive, TB_p tmp_terms, TB_p terms,
                         bool replace_inj_defs,
                         int eqdef_incrlimit, long eqdef_maxclauses);

long ClauseSetUnfoldEqDefNormalize(ClauseSet_p set, ClauseSet_p passive,
                                   ClauseSet_p archive, TB_p tmp_terms,
                                   long eqdef_incrlimit,
                                   long eqdef_maxclauses);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
