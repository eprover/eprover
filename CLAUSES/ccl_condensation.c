/*-----------------------------------------------------------------------

File  : ccl_condensation.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Implementation of the condensation rule.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1>     New

-----------------------------------------------------------------------*/

#include "ccl_condensation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long CondensationAttempts = 0;
long CondensationSuccesses = 0;

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
// Function: CondenseOnce()
//
//   Try to condense clause. If successful, simplify the clause, and
//   return true. If not, the clause is unchanged and false is
//   returned. 
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

bool CondenseOnce(Clause_p clause)
{
   CondensationAttempts++;


   return false;
}



/*-----------------------------------------------------------------------
//
// Function: Condense()
//
//   Condense a clause as much as possible. Return true if the clause
//   was changed, false otherwise.
//
// Global Variables: 
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool Condense(Clause_p clause)
{
   bool res = false;

   fprintf(GlobalOut, "Clause to condense: ");
   ClausePrint(GlobalOut, clause, true);
   fprintf(GlobalOut, "\n");

   while(CondenseOnce(clause))
   {
      res = true;
   }
   fprintf(GlobalOut, "%s: ", res?"Modified":"Unchanged");
   ClausePrint(GlobalOut, clause, true);
   fprintf(GlobalOut, "\n");

   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


