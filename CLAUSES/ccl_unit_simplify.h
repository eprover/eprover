/*-----------------------------------------------------------------------

File  : ccl_unit_simplify.h

Author: Stephan Schulz

Contents

  Functions and datatypes for performing unit-cuts and
  unit-simplifications with a mixed clause set where units are
  indexed.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 23 01:34:49 CEST 2002
    New

-----------------------------------------------------------------------*/

#ifndef CCL_UNIT_SIMPLIFY

#define CCL_UNIT_SIMPLIFY

#include <ccl_clausefunc.h>
#include <ccl_clausepos.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   NoUnitSimplify,       /* No unit simplification */
   TopLevelUnitSimplify, /* Only on top level */
   FullUnitSimplify      /* Go down (with positive units only) */
}UnitSimplifyType;

#define SimplifyFailed(res) ((res) == NULL)

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* UnitSimplifyNames[];

#define TransUnitSimplifyString(str) StringIndex((str), UnitSimplifyNames);

ClausePos_p FindTopSimplifyingUnit(ClauseSet_p units, Term_p t1,
               Term_p t2);
ClausePos_p FindSignedTopSimplifyingUnit(ClauseSet_p units, Term_p t1,
                Term_p t2, bool sign);
ClausePos_p FindSimplifyingUnit(ClauseSet_p set, Term_p t1,
            Term_p t2, bool positive_only);

bool        ClauseSimplifyWithUnitSet(Clause_p clause, ClauseSet_p
                                      unit_set, UnitSimplifyType how);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/












