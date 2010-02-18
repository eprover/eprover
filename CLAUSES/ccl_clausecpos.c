/*-----------------------------------------------------------------------

File  : ccl_clausecpos.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for handling full and compact clause positions.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Feb 19 01:24:59 EET 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_clausecpos.h"



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
// Function: PackTermPos()
//
//   Pack a term position.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompactPos PackTermPos(TermPos_p pos)
{
   CompactPos res = 0;
   PStackPointer sp;
   Term_p t;
   int    p,i;
   
   for(sp = 0; sp< PStackGetSP(pos); sp+=2)
   {
      t = PStackElementP(pos, sp);
      p = PStackElementInt(pos, sp+1);

      assert(!TermIsVar(t));
      assert(p < t->arity);

      res += DEFAULT_FWEIGHT;
      
      for(i=0; i<p; i++)
      {
         res += TermStandardWeight(t->args[i]);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PackPos()
//
//   Convert a full position into an integer-encoded position.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompactPos PackClausePos(ClausePos_p pos)
{
   CompactPos res = 0;
   Eqn_p handle;
   
   handle = pos->clause->literals;

   while(handle != pos->literal)
   {
      res += EqnStandardWeight(handle);
      handle = handle->next;
   }
   if(pos->side == LeftSide)
   {
   }
   else
   {
      res += TermStandardWeight(handle->lterm);
   }
   res += PackTermPos(pos->pos);

   return res;
}


void        UnpackClausePosInto(CompactPos cpos, Clause_p clause, 
                          ClausePos_p pos);
ClausePos_p UnpackClausePos(CompactPos cpos, Clause_p clause);



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


