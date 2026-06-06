/*-----------------------------------------------------------------------

  File  : ccl_gd_transformation.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Definitions for function implementing a TWEE-style direct goal
  transformation (by adding equational definitions that reduce goal
  ground terms to (usually new) constants.

  This goes from clause level to signature level - I put it together
  here to keep things under control...

  Copyright 2026 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Sun May 31 17:42:01 CEST 2026

-----------------------------------------------------------------------*/

#ifndef CCL_GD_TRANSFORMATION

#define CCL_GD_TRANSFORMATION

#include <ccl_formulafunc.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/



long ClauseSetGDTransform(TB_p terms,
                          ClauseSet_p clauses,
                          bool add_goal_defs_pos,
                          bool add_goal_defs_neg,
                          bool add_goal_defs_subterms);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
