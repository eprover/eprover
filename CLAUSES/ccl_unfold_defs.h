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

  Copyright 1998-2002 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Aug 14 19:54:01 CEST 2002
    New

-----------------------------------------------------------------------*/

#ifndef CCL_UNFOLD_DEFS

#define CCL_UNFOLD_DEFS

#include "ccl_clausefunc.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* We'll not export term and clause versions, because we need the
   clause information for proog output anyways */

bool ClauseUnfoldEqDef(Clause_p clause, ClausePos_p demod);
bool ClauseSetUnfoldEqDef(ClauseSet_p set, ClausePos_p demod);
long ClauseSetUnfoldAllEqDefs(ClauseSet_p set, ClauseSet_p passive,
			      int min_arity);
long ClauseSetPreprocess(ClauseSet_p set, ClauseSet_p passive, TB_p
			 tmp_terms, bool no_eq_unfold);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





