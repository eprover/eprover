/*-----------------------------------------------------------------------

File  : ccl_clausefunc.c

Author: Stephan Schulz

Contents
 
  Clause functions that need to know about sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Aug  7 00:02:44 CEST 2001
    New, partitioned ccl_clausesets.h

-----------------------------------------------------------------------*/


#include "ccl_clausefunc.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: clause_canon_compare()
//
//   Compare two indirectly pointed to clauses with
//   ClauseStructWeightCompare().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int clause_canon_compare(const Clause_p *c1, const Clause_p *c2)
{
   return ClauseStructWeightCompare(*c1, *c2);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveLiteral()
//
//   Remove *lit from clause, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause and possibly clause->set->literals
//
/----------------------------------------------------------------------*/

void ClauseRemoveLiteral(Clause_p clause, Eqn_p *lit)
{
   Eqn_p handle = *lit;

   if(EqnIsPositive(handle))
   {
      clause->pos_lit_no--;
   }
   else
   {
      clause->neg_lit_no--;
   }
   if(clause->set)
   {
      clause->set->literals--;
   }
   EqnListDeleteElement(lit);   
}


/*-----------------------------------------------------------------------
//
// Function: ClauseKillChildren()
//
//   Remove and delete children of a clause.
//
// Global Variables: -
//
// Side Effects    : Changes term sets
//
/----------------------------------------------------------------------*/

void ClauseKillChildren(Clause_p clause)
{
   Clause_p current;

   while(clause->children)
   {
      current = clause->children->key;
      assert(clause == current->parent1 || clause == current->parent2);
      /* printf("ClauseKillChildren(%d) -> %d\n",
	 (int)clause,(int)current); */
      ClauseDetachParents(current);
      if(!ClauseQueryProp(current, CPIsProtected))
      {
	 ClauseSetDeleteEntry(current);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveSuperfluousLiterals()
//
//   Remove duplicate and trivial negative literals from the
//   clause. Return number of removed literals.
//
// Global Variables: -
//
// Side Effects    : Changes clause, termbanks
//
/----------------------------------------------------------------------*/

int ClauseRemoveSuperfluousLiterals(Clause_p clause)
{
   Eqn_p handle;
   int   removed = 0;

   removed += EqnListRemoveResolved(&(clause->literals), TBTermEqual);
   removed += EqnListRemoveDuplicates(clause->literals, TBTermEqual);
   
   if(removed)
   {
      clause->neg_lit_no = 0;
      clause->pos_lit_no = 0;   
      handle = clause->literals;
      
      while(handle)
      {
	 if(EqnIsPositive(handle))
	 {
	    clause->pos_lit_no++;
	 }
	 else
	 {
	    clause->neg_lit_no++;
	 }
	 handle = handle->next;
      }
      
      if(clause->set)
      {
	 clause->set->literals-=removed;
      }
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetRemoveSuperflousLiterals()
//
//   For all clauses in set remove the trivial and duplicated
//   literals. Return number of literals removed.
//
// Global Variables: -
//
// Side Effects    : Only as described
//
/----------------------------------------------------------------------*/

long ClauseSetRemoveSuperfluousLiterals(ClauseSet_p set)
{
   Clause_p handle;
   long res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
	  handle->succ)
   {
      res += ClauseRemoveSuperfluousLiterals(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCanonize()
//
//   Canonize a clause set by canonizing all
//   clauses, and sorting them in the order defined by
//   ClauseStructWeightCompare(). 
//
// Global Variables: -
//
// Side Effects    : Memory usage.
//
/----------------------------------------------------------------------*/
 
void ClauseSetCanonize(ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor;
       handle = handle->succ)
   {
      ClauseRemoveSuperfluousLiterals(handle);
      ClauseCanonize(handle);
   }
   ClauseSetSort(set, clause_canon_compare);
   
   /* printf("Canonized: \n");
      ClauseSetPrint(stdout, set, true); */
}

/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveACResolved()
//
//   Remove AC-resolved literals.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

int ClauseRemoveACResolved(Clause_p clause)
{
   int   removed = 0;
   Sig_p sig;

   if(clause->neg_lit_no==0)
   {
      return 0;
   }
   sig = clause->literals->bank->sig;
   removed += EqnListRemoveACResolved(&(clause->literals));
   clause->neg_lit_no -= removed ;
   if(removed)
   {
      DocClauseModification(GlobalOut, OutputLevel, clause,
			    inf_ac_resolution, NULL, sig, NULL);	 
   }
   if(clause->set)
   {
      clause->set->literals-=removed;
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseUnitSimplifyTest()
//
//   Return true if clause can be simplified by a top-simplify-reflect
//   step with the (non-orientable) unit clause simplifier.
//
// Global Variables: -
//
// Side Effects    : Changes clause and values in set
//
/----------------------------------------------------------------------*/

bool ClauseUnitSimplifyTest(Clause_p clause, Clause_p simplifier)
{
   bool positive,tmp;
   EqnRef handle;
   Eqn_p  simpl;
   
   assert(ClauseIsUnit(simplifier));
   simpl = simplifier->literals;
   assert(EqnIsNegative(simpl)||!EqnIsOriented(simpl));
   
   positive = EqnIsPositive(simpl);
   
   if(EQUIV(positive, ClauseIsPositive(clause)))
   {
      return 0;
   }

   handle = &(clause->literals);

   while(*handle)
   {
      tmp = EqnIsPositive(*handle);
      if(XOR(positive,tmp)&&EqnSubsumeP(simpl,*handle,TBTermEqual))
      {
	 return true;
      }
      handle = &((*handle)->next);
   }
   return false;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






