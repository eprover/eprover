/*-----------------------------------------------------------------------

File  : ccl_unit_simplify.c

Author: Stephan Schulz

Contents

  Functions for doing unit cut-off and unit-subsumption with indexed
  mixed clause sets.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 23 02:00:52 CEST 2002
    New

-----------------------------------------------------------------------*/

#include "ccl_unit_simplify.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* UnitSimplifyNames[]=
{
   "NoSimplify",
   "TopSimplify",
   "FullSimplify",
   NULL
};

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
// Function: FindTopSimplifyingUnit()
//
//   Find a unit s=t (or s!=t) in units such that sigma(s)=t1 and
//   sigma(t)=t2 for some sigma.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ClausePos_p FindTopSimplifyingUnit(ClauseSet_p units, Term_p t1, Term_p t2)
{
   Subst_p     subst = SubstAlloc();
   ClausePos_p pos, res = NULL;

   assert(TermStandardWeight(t1) == TermWeight(t1,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(TermStandardWeight(t2) == TermWeight(t2,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(units && units->demod_index);

   PDTreeSearchInit(units->demod_index, t1, PDTREE_IGNORE_NF_DATE, false);

   while((pos = PDTreeFindNextDemodulator(units->demod_index, subst)))
   {
      if(SubstMatchComplete(ClausePosGetOtherSide(pos), t2, subst))
      {
        // if the problem is not HO, we match completely.
        assert(pos->clause->set == units);
        res = pos;
        break;
      }
   }
   PDTreeSearchExit(units->demod_index);
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FindSignedTopSimplifyingUnit()
//
//   Find a unit s=t (or s!=t) in units such that sigma(s)=t1 and
//   sigma(t)=t2 for some sigma. Return only clauses with sign sign.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
ClausePos_p FindSignedTopSimplifyingUnit(ClauseSet_p units, Term_p t1, Term_p t2, bool sign)
{
   Subst_p     subst = SubstAlloc();
   ClausePos_p pos, res = NULL;

   assert(TermStandardWeight(t1) == TermWeight(t1,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(TermStandardWeight(t2) == TermWeight(t2,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(units && units->demod_index);

   PDTreeSearchInit(units->demod_index, t1, PDTREE_IGNORE_NF_DATE, false);

   while((pos = PDTreeFindNextDemodulator(units->demod_index, subst)))
   {
      if( EQUIV(EqnIsPositive(pos->literal), sign)
          && (SubstMatchComplete(ClausePosGetOtherSide(pos), t2, subst)))
      {
        // if the problem is not HO, we match completely.
        assert(pos->clause->set == units);
        res = pos;
        break;
      }
   }
   PDTreeSearchExit(units->demod_index);
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FindSimplifyingUnit()
//
//   Return a unit clause with from set that can simplify or subsume
//   t1=t2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ClausePos_p FindSimplifyingUnit(ClauseSet_p set, Term_p t1, Term_p t2,
            bool positive_only)
{
   Term_p   tmp1, tmp2 = NULL;
   int      i;
   ClausePos_p res = NULL;

   if(positive_only)
   {
      res = FindSignedTopSimplifyingUnit(set, t1, t2, true);
   }
   else
   {
      res = FindTopSimplifyingUnit(set, t1, t2);
   }
   
   if(!SimplifyFailed(res))
   {
      return res;
   }

   while(SimplifyFailed(res))
   {
      if(TermIsTopLevelFreeVar(t1) || TermIsTopLevelFreeVar(t2) ||
         TermIsLambda(t1) || TermIsLambda(t2) ||
         t1->f_code != t2->f_code || !t1->arity)
      {
        break;
      }
      tmp1 = NULL; /* Used to determine if another position has
            already been found */

      assert(t1!=t2);
      for(i=0; i<t1->arity; i++)
      {
         if(t1->args[i] != t2->args[i])
         {
            if(tmp1)
            {
               tmp2 = NULL; /* Signal that more than one conflict
                     exists */
               break;
            }
            tmp1 = t1->args[i];
            tmp2 = t2->args[i];
         }
      }
      if(!tmp2)
      {
         break;
      }
      t1 = tmp1;
      t2 = tmp2;
      res = FindSignedTopSimplifyingUnit(set, t1, t2, true);
      if(problemType == PROBLEM_HO && !SimplifyFailed(res))
      {
         return res;
      }
   }
   return res;
}





/*-----------------------------------------------------------------------
//
// Function: ClauseSimplifyWithUnitSet()
//
//   Simplify a clause with the (indexed) units from set. Performs
//   simplify-reflect and subsumption steps. simplify-reflect is
//   controlled by the value of how. If clause is subsumed by a unit,
//   return false, otherwise return true.
//
// Global Variables: -
//
// Side Effects    : Changes clause, may cause output.
//
/----------------------------------------------------------------------*/

bool ClauseSimplifyWithUnitSet(Clause_p clause, ClauseSet_p unit_set,
                               UnitSimplifyType how)
{
   Eqn_p *handle;
   ClausePos_p res;

   assert(clause);
   assert(unit_set && unit_set->demod_index);
   assert(how);

   handle = &(clause->literals);
   while(*handle)
   {
      if(how == TopLevelUnitSimplify)
      {
         res = FindTopSimplifyingUnit(unit_set,
                                       (*handle)->lterm,
                                       (*handle)->rterm);
      }
      else
      {
         res = FindSimplifyingUnit(unit_set,
                    (*handle)->lterm,
                    (*handle)->rterm, false);
      }
      if(!SimplifyFailed(res))
      {
         ClausePos_p pos = res;
         assert(ClauseIsUnit(pos->clause));
         if(EQUIV(EqnIsPositive(*handle),
             EqnIsPositive(pos->literal)))
         {
            DocClauseQuote(GlobalOut, OutputLevel, 6, clause,
                 "subsumed by unprocessed unit",
                 pos->clause);
            if(!ClauseIsUnit(clause)&&
               ClauseStandardWeight(clause)==ClauseStandardWeight(pos->clause))
            {
               ClauseSetProp(pos->clause, CPIsProtected);
            }
            ClauseSetProp(pos->clause, ClauseQueryProp(clause, CPIsSOS));
            return false;
         }
         ClauseDelProp(clause, CPLimitedRW);
         ClauseRemoveLiteralRef(clause, handle);
         DocClauseModification(GlobalOut, OutputLevel, clause,
                     inf_simplify_reflect, pos->clause,
                     NULL, "cut with unprocessed unit");
      }
      else
      {
         handle = &((*handle)->next);
      }
   }
   return true;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/







