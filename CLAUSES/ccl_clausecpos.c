/*-----------------------------------------------------------------------

File  : ccl_clausecpos.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for handling full and compact clause positions.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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

   assert(pos);

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

   assert(pos->literal);
   while(handle != pos->literal)
   {
      res += EqnStandardWeight(handle);
      handle = handle->next;
      assert(handle);
   }

   if(pos->side == RightSide)
   {
      res += TermStandardWeight(handle->lterm);
   }
   res += PackTermPos(pos->pos);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: UnpackTermPos()
//
//   Given a compact term position in t, encode it into the given full
//   postion.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void UnpackTermPos(TermPos_p pos, Term_p t, CompactPos cpos)
{
   int i;

   PStackReset(pos);

   while(cpos > 0)
   {
      assert(!TermIsVar(t));
      cpos -= DEFAULT_FWEIGHT;
      assert(cpos>=0);

      PStackPushP(pos, t);
      for(i=0; i< t->arity; i++)
      {
         if(cpos < TermStandardWeight(t->args[i]))
         {
            PStackPushInt(pos, i);
            t = t->args[i];
            break;
         }
         cpos -= TermStandardWeight(t->args[i]);
         assert(cpos>=0);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: UnpackClausePosInto()
//
//   Unpack the compact position cpos in clause into the existing
//   ClausePos pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void UnpackClausePosInto(CompactPos cpos, Clause_p clause,
                         ClausePos_p pos)
{
   Eqn_p handle;
   Term_p t;

   assert(clause);

   pos->clause = clause;

   handle = clause->literals;
   assert(handle);

   while(EqnStandardWeight(handle) <= cpos)
   {
      cpos -= EqnStandardWeight(handle);
      handle = handle->next;
      assert(handle);
   }
   pos->literal = handle;
   if(cpos >= TermStandardWeight(handle->lterm))
   {
      cpos -= TermStandardWeight(handle->lterm);
      pos->side = RightSide;
      t = handle->rterm;
   }
   else
   {
      pos->side = LeftSide;
      t = handle->lterm;
   }
   UnpackTermPos(pos->pos, t, cpos);
}


/*-----------------------------------------------------------------------
//
// Function: UnpackClausePos()
//
//   Unpack a clause compact position, returning a newly allocated
//   clause position.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

ClausePos_p UnpackClausePos(CompactPos cpos, Clause_p clause)
{
   ClausePos_p handle = ClausePosAlloc();

   UnpackClausePosInto(cpos, clause, handle);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCPosGetSubterm()
//
//   Given a clause and a compact position, return the indicated
//   term. This is a very simple but obviously correct version.
//
// Global Variables: -
//
// Side Effects    : Memory allocations/deallocations
//
/----------------------------------------------------------------------*/

Term_p ClauseCPosGetSubterm(Clause_p clause, CompactPos cpos)
{
   Term_p      res;
   ClausePos_p pos = UnpackClausePos(cpos, clause);

   res = ClausePosGetSubterm(pos);
   ClausePosFree(pos);

   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


