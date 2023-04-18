/*-----------------------------------------------------------------------

File  : ccl_clausepos.c

Author: Stephan Schulz

Contents

  Funktions for dealing with positions in clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
// Function: ClausePosPrint()
//
//   Print a clause position.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePosPrint(FILE* out, ClausePos_p pos)
{
   long i = 0;
   Eqn_p handle;

   for(handle=pos->clause->literals;
       handle!=pos->literal;
       handle=handle->next)
   {
      i++;
   }
   fprintf(out, "%ld.%ld.%c.", pos->clause->ident, i,
           pos->side==RightSide?'R':'L');
   TermPosPrint(out, pos->pos);
}



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


/*-----------------------------------------------------------------------
//
// Function: TermComputeRWSequence()
//
//   Given two terms from and two, connected by a rewrite chain, push
//   a sequence of clause idents onto the stack such that they
//   represent a rewrite chain transforming to into from. Returns true
//   if the chain has length 0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermComputeRWSequence(PStack_p stack, Term_p from, Term_p to,
                           int inject_op)
{
   bool     res = false;
   Clause_p demod;
   Term_p   tmp;

   while(from != to)
   {
      assert(TermIsRewritten(from));
      demod = TermRWDemod(from);
      tmp = TermRWReplaceField(from);
      if(!demod)
      {
         int i;
         assert(from->f_code == tmp->f_code);
         assert(from->arity);

         for(i=0; i<from->arity; i++)
         {
            TermComputeRWSequence(stack,
                                  from->args[i],
                                  tmp->args[i],
                                  inject_op);
         }
      }
      else
      {
         if(inject_op)
         {
            PStackPushInt(stack, inject_op);
         }
         PStackPushP(stack, demod);
      }
      from = tmp;
      assert(from);
      res = true;
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
