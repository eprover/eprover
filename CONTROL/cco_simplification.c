/*-----------------------------------------------------------------------

File  : cco_simplification.c

Author: Stephan Schulz

Contents
 
  Control of simplified clauses

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 14:49:49 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "cco_simplification.h"



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
// Function: term_unrestrict()
//
//   Create a copy of term cell that does not carry any non-intrinsic
//   properties (in particularly not TPIsRestricted). If such a cell
//   already exists, delete it's rewrite information and return it.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static Term_p term_unrestrict(TB_p bank, Term_p term)
{
   if(!TermCellIsAnyPropSet(term, TPRestricted)||TermIsVar(term))
   {
      return term;
   }
   term = TermTopCopy(term); /* This deletes the property! */
   term = TBTermtopInsert(bank, term);
   TermCellDelProp(term, TPIsRewritten|TPIsSOSRewritten);
   term->rw_data.nf_date[0] = SysDateCreationTime();
   term->rw_data.nf_date[1] = SysDateCreationTime();

   return term;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseMoveSimplified()
//
//   Remove a processed simplified clause from its set and add it to a
//   temporary set.
//
// Global Variables: -
//
// Side Effects    : Kills children, modifes clause counter
//
/----------------------------------------------------------------------*/

void ClauseMoveSimplified(Clause_p clause, ClauseSet_p tmp_set) 
{
   Eqn_p handle;
   
   ClauseKillChildren(clause);
   ClauseSetExtractEntry(clause);

   for(handle = clause->literals; handle; handle=handle->next)
   {
      handle->lterm = term_unrestrict(handle->bank, handle->lterm);
      handle->rterm = term_unrestrict(handle->bank, handle->rterm);
   }
   DocClauseQuoteDefault(6, clause, "back_simplified");
   ClauseSetInsert(tmp_set, clause);
}


/*-----------------------------------------------------------------------
//
// Function: RemoveRewritableClauses()
//
//   Remove all clauses which can be rewritten with new_demod.
//
// Global Variables: -
//
// Side Effects    : As specified...
//
/----------------------------------------------------------------------*/

bool RemoveRewritableClauses(OCB_p ocb, ClauseSet_p from, ClauseSet_p into,
			     Clause_p new_demod, SysDate nf_date)
{
   PStack_p stack = PStackAlloc();
   Clause_p handle;
   bool     res;

   res = FindRewritableClauses(ocb, from, stack, new_demod, nf_date);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);

      ClauseMoveSimplified(handle, into);
   }
   PStackFree(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetUnitSimplify()
//
//   Try to simplify all clauses in set by performing matching unit
//   resolution with simplifier. Move affected clauses from set into
//   tmp_set. Return number of clauses moved.
//
// Global Variables: -
//
// Side Effects    : Changes clauses and set.
//
/----------------------------------------------------------------------*/

long ClauseSetUnitSimplify(ClauseSet_p set, Clause_p simplifier,
			   ClauseSet_p tmp_set)
{
   Clause_p handle, move;
   long res = 0,tmp;
   
   handle = set->anchor->succ; 
   while(handle!=set->anchor)
   {
      tmp = ClauseUnitSimplifyTest(handle, simplifier);
      if(tmp)
      {
	 move = handle;
	 handle = handle->succ;	
	 ClauseMoveSimplified(move, tmp_set);
	 res++;
      }
      else
      {
	 handle = handle->succ;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: RemoveContextualSRClauses()
//
//   Move clauses that simplifier can contextually simplify-reflect
//   from from into into. Return number of clauses moved.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

long RemoveContextualSRClauses(ClauseSet_p from,
			       ClauseSet_p into,
			       Clause_p simplifier)
{
   PStack_p stack = PStackAlloc();
   long res = 0;
   Clause_p handle;
   
   ClauseSetFindContextSRClauses(from, simplifier, stack);
   
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      if(handle->set == from) /* Clauses may be found more than once
				 by ClauseSetFindContextSRClauses() */
      {
	 ClauseMoveSimplified(handle, into);
	 res++;
      }
   }
   PStackFree(stack);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


