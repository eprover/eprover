/*-----------------------------------------------------------------------

File  : cco_clausesplitting.c

Author: Stephan Schulz

Contents

  Functions for controling (and performing) clause splitting.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr 27 20:27:16 MET DST 2001
    New

-----------------------------------------------------------------------*/

#include "cco_clausesplitting.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ControlledClauseSplit()
//
//   Check if the clause meets one of the criteria for splitting, if
//   yes try to split it. Return number of new clauses if splitting
//   occurs, 0 otherwise.
//
// Global Variables: -
//
// Side Effects    : Creates new clauses, memory operations - all by
//                   calling ClauseSplit()
//
/----------------------------------------------------------------------*/

int  ControlledClauseSplit(DefStore_p store, Clause_p clause, ClauseSet_p set,
            SplitClassType which, SplitType how,
                           bool fresh_defs)
{
   /* Make the common case fast: */

   if(!which)
   {
      return 0;
   }
   if(
      (ClauseIsHorn(clause)&&QuerySplitClass(which,SplitHorn))||
      (!ClauseIsHorn(clause)&&QuerySplitClass(which,SplitNonHorn))||
      (ClauseIsNegative(clause)&&QuerySplitClass(which,SplitNegative))||
      (ClauseIsPositive(clause)&&QuerySplitClass(which,SplitPositive))||
      (ClauseIsMixed(clause)&&QuerySplitClass(which,SplitMixed))
      )
   {
      return ClauseSplit(store, clause, set, how, fresh_defs);
   }
   return 0;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


