/*-----------------------------------------------------------------------

File  : cco_simplification.c

Author: Stephan Schulz

Contents

  Control of simplified clauses

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
// Function: ClauseMoveSimplified()
//
//   Remove a processed simplifiable clause from its set and add it to
//   a temporary set.
//
// Global Variables: -
//
// Side Effects    : Kills children, modifes clause counter
//
/----------------------------------------------------------------------*/

void ClauseMoveSimplified(GlobalIndices_p gindices,
                          Clause_p clause,
                          ClauseSet_p tmp_set,
                          ClauseSet_p archive)
{
   // printf("# Removing %p from %p: ", clause, clause->set);ClausePrint(stdout, clause, true);printf("\n");
   ClauseKillChildren(clause);
   ClauseSetExtractEntry(clause);
   GlobalIndicesDeleteClause(gindices, clause);
   DocClauseQuoteDefault(6, clause, "simplifiable");
   if(BuildProofObject||ClauseIsDemodulator(clause))
   {
      Clause_p new_clause = ClauseFlatCopy(clause);
      ClauseSetInsert(archive, clause);
      ClausePushDerivation(new_clause, DCCnfQuote, clause, NULL);
      ClauseSetInsert(tmp_set, new_clause);
   }
   else
   {
      ClauseSetInsert(tmp_set, clause);
   }
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
                             ClauseSet_p archive,
              Clause_p new_demod, SysDate nf_date,
                             GlobalIndices_p gindices)
{
   PStack_p stack = PStackAlloc();
   Clause_p handle;
   bool     res;

   res = FindRewritableClauses(ocb, from, stack, new_demod, nf_date);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);

      ClauseMoveSimplified(gindices, handle, into, archive);
   }
   PStackFree(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: RemoveRewritableClausesIndexed()
//
//   Remove all clauses in gindices->bw_rw_index which can be
//   rewritten with new_demod.
//
// Global Variables: -
//
// Side Effects    : As specified...
//
/----------------------------------------------------------------------*/

bool RemoveRewritableClausesIndexed(OCB_p ocb, ClauseSet_p into,
                                    ClauseSet_p archive,
                                    Clause_p new_demod, SysDate nf_date,
                                    GlobalIndices_p gindices)
{
   PStack_p stack = PStackAlloc();
   Clause_p handle;
   bool     res;

   res = FindRewritableClausesIndexed(ocb, gindices->bw_rw_index,
                                      stack, new_demod, nf_date);
   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);
      // printf("# XXX RWRemoving %p from %p with index %p (flag %d): ", handle, handle->set, gindices, ClauseQueryProp(handle, CPIsGlobalIndexed)); ClausePrint(stdout, handle, true); printf("\n");
      ClauseDelProp(handle, CPRWDetected);
      ClauseMoveSimplified(gindices, handle, into, archive);
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
            ClauseSet_p tmp_set, ClauseSet_p archive,
                           GlobalIndices_p gindices)
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
    ClauseMoveSimplified(gindices, move, tmp_set, archive);
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
                               ClauseSet_p archive,
                Clause_p simplifier,
                               GlobalIndices_p gindices)
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
    ClauseMoveSimplified(gindices, handle, into, archive);
    res++;
      }
   }
   PStackFree(stack);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


