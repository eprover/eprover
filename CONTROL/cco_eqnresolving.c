/*-----------------------------------------------------------------------

File  : cco_eqnresolving.c

Author: Stephan Schulz

Contents

  Functions controlling equality resolution.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 17:17:57 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "cco_eqnresolving.h"



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
// Function: ComputeAllEqnResolvents()
//
//   Compute all equality resolvents of clause and put them into
//   store. Returns number of clauses generated.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes to term bank and
//                   possibly freshvars, output documentation.
//
/----------------------------------------------------------------------*/

long ComputeAllEqnResolvents(TB_p bank, Clause_p clause, ClauseSet_p
              store, VarBank_p freshvars)
{
   Clause_p    resolvent;
   Eqn_p       test;
   ClausePos_p pos;
   long        resolv_count = 0;

   if(clause->neg_lit_no && !ClauseQueryProp(clause,CPNoGeneration))
   {
      pos = ClausePosAlloc();

      test = ClausePosFirstEqResLiteral(clause, pos);

      while(test)
      {
    resolvent = ComputeEqRes(bank, pos, freshvars);
    if(resolvent)
    {
       resolv_count++;
       resolvent->parent1 = clause;
       resolvent->proof_depth = clause->proof_depth+1;
       resolvent->proof_size  = clause->proof_size+1;
       ClauseSetTPTPType(resolvent, ClauseQueryTPTPType(clause));
       ClauseSetProp(resolvent, ClauseGiveProps(clause, CPIsSOS));
       ClauseRegisterChild(clause, resolvent);
       DocClauseCreationDefault(resolvent, inf_eres, clause, NULL);
            ClausePushDerivation(resolvent, DCEqRes, clause, NULL);
       ClauseSetInsert(store, resolvent);
    }
    test = ClausePosNextEqResLiteral(pos);
      }
      ClausePosFree(pos);
   }
   return resolv_count;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseERNormalizeVar()
//
//   Perform destructive equality resolution inferences on negative
//   pure variable literals (or, if strong, on negative literals with
//   at least one variable side), insert the final result into
//   store. Return the number of inferences.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

long ClauseERNormalizeVar(TB_p bank, Clause_p clause, ClauseSet_p
           store, VarBank_p freshvars, bool strong)
{
   long count = 0;

   if(clause->neg_lit_no && !ClauseQueryProp(clause,CPNoGeneration))
   {
      Eqn_p       lit;
      Clause_p    handle;
      ClausePos_p pos = ClausePosAlloc();
      bool        found = true;

      while(found)
      {
    found = false;
    for(lit = clause->literals; lit; lit = lit->next)
    {
       if(EqnIsNegative(lit)&&
          (EqnIsPureVar(lit) || (strong&&EqnIsPartVar(lit))))
       {
          pos->clause  = clause;
          pos->literal = lit;
          handle = ComputeEqRes(bank, pos, freshvars);
          if(handle)
          {
        found = true;
        count++;
                  clause->proof_depth++;
                  clause->proof_size++;
                  EqnListFree(clause->literals);
                  clause->literals = handle->literals;
                  ClauseRecomputeLitCounts(clause);

                  handle->literals = NULL;
                  ClauseFree(handle);

        DocClauseModificationDefault(clause, inf_eres, clause);
                  ClausePushDerivation(clause, DCDesEqRes, NULL, NULL);

        break;
          }
       }
    }
      }
      if(count)
      {
    ClauseSetInsert(store, clause);
      }
      ClausePosFree(pos);
   }
   return count;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


