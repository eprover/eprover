/*-----------------------------------------------------------------------

File  : ccl_clausepos.c

Author: Stephan Schulz

Contents
 
  Funktions for dealing with positions in clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed May 20 03:42:41 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "ccl_clausepos.h"



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
// Function: ClausePosFindPosLiteral()
//
//    Find the first positive literal (if maximal = true, find the
//    first positive and maximal literal).
//
// Global Variables: -
//
// Side Effects    : Changes pos->literal
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosFindPosLiteral(ClausePos_p pos, bool maximal)
{
   Eqn_p handle;

   for(handle = pos->literal; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle) && (EqnIsMaximal(handle)|| !maximal))
      {
         break;
      }
   }
   pos->literal = handle;
   return handle;
}



/*-----------------------------------------------------------------------
//
// Function: ClausePosFindMaxLiteral()
//
//   Find the first maximal literal in the list at pos->literal. If
//   positive, find positive literals only.
//
// Global Variables: -
//
// Side Effects    : Changes pos->literal
//
/----------------------------------------------------------------------*/

Eqn_p ClausePosFindMaxLiteral(ClausePos_p pos, bool positive)
{
   Eqn_p handle;

   for(handle = pos->literal; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle) &&
	 (!positive || EqnIsPositive(handle)))
      {
         break;
      }
   }
   pos->literal = handle;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFindFirstMaximalSide()
//
//   Find the first maximal side in the list at pos->literal, if
//   positive is set, use positive equations only.
//
// Global Variables: -
//
// Side Effects    : Changes pos->literal, pos->side
//
/----------------------------------------------------------------------*/

Term_p ClausePosFindFirstMaximalSide(ClausePos_p pos, bool positive)
{
   Eqn_p handle;
   Term_p res = NULL;

   handle = ClausePosFindMaxLiteral(pos, positive);
   if(handle)
   {
      pos->side = LeftSide;
      PStackReset(pos->pos);
      res = ClausePosGetSide(pos);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFindNextMaximalSide()
//
//   Given a position, find the next maximal side in the eqnlist at
//   pos->literal. If positive is set, use positive equations only.
//
// Global Variables: -
//
// Side Effects    : Changes pos->literal, pos->side
//
/----------------------------------------------------------------------*/

Term_p ClausePosFindNextMaximalSide(ClausePos_p pos, bool positive)
{
   Eqn_p handle;
   Term_p res = NULL;

   if(pos->literal)
   {
      PStackReset(pos->pos);
      if(pos->side == LeftSide 
         && !EqnIsOriented(pos->literal))
      {
         pos->side = RightSide;
         res = ClausePosGetSide(pos);
      }
      else
      {
         pos->literal = pos->literal->next;
         handle = ClausePosFindMaxLiteral(pos, positive);
         if(handle)
         {
            pos->side = LeftSide;
            res = ClausePosGetSide(pos);
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFindFirstMaximalSubterm()
//
//   Given a clause, find the first subterm in a maximal side in a
//   maximal literal.
//
// Global Variables: -
//
// Side Effects    : Changes pos.
//
/----------------------------------------------------------------------*/

Term_p ClausePosFindFirstMaximalSubterm(ClausePos_p pos)
{
   Term_p res;
   
   res = ClausePosFindFirstMaximalSide(pos, false);
   
   if(res)
   {
      res = TermPosFirstLIPosition(res, pos->pos);
   }
   else
   {
      res = NULL;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFindNextMaximalSubterm()
//
//   Given a position in a clause, find the next maximal subterm in
//   it.  
//
// Global Variables: -
//
// Side Effects    : Changes pos.
//
/----------------------------------------------------------------------*/

Term_p ClausePosFindNextMaximalSubterm(ClausePos_p pos)
{
   Term_p res, side;
    
   side = ClausePosGetSide(pos);
   res = TermPosNextLIPosition(pos->pos);

   if(!res)
   {
      side = ClausePosFindNextMaximalSide(pos, false);
      if(side)
      {
         res = TermPosFirstLIPosition(side, pos->pos);
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


