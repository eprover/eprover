/*-----------------------------------------------------------------------

File  :  cco_clausesplitting.h

Author: Stephan Schulz

Contents
 
  The interface functions for controlled clause splitting.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr 27 20:13:53 MET DST 2001
    New

-----------------------------------------------------------------------*/

#ifndef CCO_CLAUSESPLITTING

#define CCO_CLAUSESPLITTING

#include <ccl_splitting.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* See ccl_splitting.h */

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

int  ControlledClauseSplit(Clause_p clause, ClauseSet_p set,
			   SplitClassType which, SplitType how);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





