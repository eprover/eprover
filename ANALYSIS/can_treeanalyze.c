/*-----------------------------------------------------------------------

File  : can_treeanalyze.c

Author: Stephan Schulz

Contents

  Analyzing the proof state.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar 23 02:21:00 MET 1999
    New

-----------------------------------------------------------------------*/

#include "can_treeanalyze.h"

#ifdef NEVER_DEFINED

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
// Function: select_examples_from_proof()
//
//   Select (up to) target_number training examples from infstate that
//   describes a proof. Return number of examples picked.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long select_examples_from_proof(InfState_p state, long
                   target_number)
{
   long res,
        max_distance,
        rest = 0,
        oracle,
        distance = PROOF_DIST_INFINITY,
        i;
   PDArray_p dist = PDIntArrayAlloc(10,5);
   CompClause_p clause;

   max_distance  = ProofDistanceDistrib(state, CPIsSelected, dist);

   res = 0;
   for(i=0; i<=max_distance; i++)
   {
      oracle = res + PDArrayElementInt(dist, i);
      if(oracle > target_number)
      {
    rest = target_number - res;
    distance = i;
    break;
      }
      res = oracle;
   }

   res = 0;
   for(i=0; i<state->clause_store->size; i++)
   {
      clause = PDArrayElementP(state->clause_store,i);
      if(clause && ClauseQueryProp(clause, CPIsSelected))
      {
    if(clause->stats.proof_distance < distance)
    {
       ClauseSetProp(clause, CPOpFlag);
       res++;
    }
    if(rest && (clause->stats.proof_distance == distance))
    {
       ClauseSetProp(clause, CPOpFlag);
       rest--;
       res++;
    }
      }
   }
   PDArrayFree(dist);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: select_examples_from_non_proof()
//
//   Select (up to) target_number training examples from infstate that
//   does not describe a proof. Return number of examples picked.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long select_examples_from_non_proof(InfState_p state, long
                  target_number)
{
   long res = 0, i;
   CompClause_p clause;

   for(i=0; i<state->clause_store->size; i++)
   {
      clause = PDArrayElementP(state->clause_store,i);
      if(clause&&ClauseQueryProp(clause, CPIsSelected))
      {
    if(res == target_number)
    {
       break;
    }
    res++;
    clause->stats.proof_distance = PROOF_DIST_DEFAULT;
    ClauseSetProp(clause, CPOpFlag);
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ProofMarkProofClauses()
//
//   Mark all clauses used for finding a proof (or other final
//   clauses).
//
// Global Variables: -
//
// Side Effects    : Sets properties in clauses.
//
/----------------------------------------------------------------------*/

long ProofMarkProofClauses(InfState_p proof)
{
   long i;
   CompClause_p clause;
   PStack_p jobs = PStackAlloc(), traverse;
   PTree_p node;
   long proof_nodes = 0;

   for(i=0; i<proof->clause_store->size; i++)
   {
      clause = PDArrayElementP(proof->clause_store,i);
      if(clause)
      {
    if(ClauseQueryProp(clause, CPIsFinal))
    {
       PStackPushP(jobs, clause);
    }
      }
   }
   while(!PStackEmpty(jobs))
   {
      clause = PStackPopP(jobs);
      if(ClauseQueryProp(clause, CPIsProofClause))
      {
    continue;
      }
      ClauseSetProp(clause, CPIsProofClause);
      proof_nodes++;

      traverse = PTreeTraverseInit(clause->g_parents);
      while((node = PTreeTraverseNext(traverse)))
      {
    PStackPushP(jobs, node->key);
      }
      PTreeTraverseExit(traverse);

      traverse = PTreeTraverseInit(clause->s_parents);
      while((node = PTreeTraverseNext(traverse)))
      {
    PStackPushP(jobs, node->key);
      }
      PTreeTraverseExit(traverse);
   }
   PStackFree(jobs);
   return proof_nodes;
}


/*-----------------------------------------------------------------------
//
// Function: ProofSetClauseStatistics()
//
//   Set the parent-number values in all clauses in state to the
//   supplied values.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ProofSetClauseStatistics(InfState_p proof, long s_used, long
               s_unused, long g_used, long g_unused,
               long proof_distance)
{
   long i;
   CompClause_p clause;

   for(i=0; i<proof->clause_store->size; i++)
   {
      clause = PDArrayElementP(proof->clause_store,i);
      if(clause)
      {
    clause->stats.simplify_used   = s_used;
    clause->stats.simplify_unused = s_unused;
    clause->stats.generate_used   = g_used;
    clause->stats.generate_unused = g_unused;
    clause->stats.proof_distance  = proof_distance;
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ProofComputeParentNumbers()
//
//   For each clause in state, compute the number of direct successors
//   it generated. Assumes that these numbers are reset.
//
// Global Variables: -
//
// Side Effects    : Changes parent values.
//
/----------------------------------------------------------------------*/

void ProofComputeParentNumbers(InfState_p proof)
{
   long i;
   CompClause_p clause, parent;
   PStack_p traverse;
   PTree_p node;

   for(i=0; i<proof->clause_store->size; i++)
   {
      clause = PDArrayElementP(proof->clause_store,i);
      if(clause)
      {
    if(ClauseQueryProp(clause, CPIsProofClause))
    {
       traverse = PTreeTraverseInit(clause->g_parents);
       while((node = PTreeTraverseNext(traverse)))
       {
          parent = node->key;
          parent->stats.generate_used++;
       }
       PTreeTraverseExit(traverse);
       traverse = PTreeTraverseInit(clause->s_parents);
       while((node = PTreeTraverseNext(traverse)))
       {
          parent = node->key;
          parent->stats.simplify_used++;
       }
       PTreeTraverseExit(traverse);
    }
    else
    {
       traverse = PTreeTraverseInit(clause->g_parents);
       while((node = PTreeTraverseNext(traverse)))
       {
          parent = node->key;
          parent->stats.generate_unused++;
       }
       PTreeTraverseExit(traverse);
       traverse = PTreeTraverseInit(clause->s_parents);
       while((node = PTreeTraverseNext(traverse)))
       {
          parent = node->key;
          parent->stats.simplify_unused++;
       }
       PTreeTraverseExit(traverse);
    }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ProofComputeDistance()
//
//   Compute the distance (i.e. the length of the longest chain of
//   clauses used in generating inferences starting at proof clauses
//   and stopping at the given clause) of all clauses in state from
//   the proof. Assumes that proof clauses are marked as such, and
//   makes use of the fact that all clauses used as premise in an
//   inference have smaller idents that the conclusion.
//
// Global Variables: -
//
// Side Effects    : Changes proof distance field.
//
/----------------------------------------------------------------------*/

void ProofComputeDistance(InfState_p state)
{
   long         i, distance;
   CompClause_p clause, parent;
   PStack_p     traverse;
   PTree_p      cell;

   for(i=0; i<state->clause_store->size; i++)
   {
      clause = PDArrayElementP(state->clause_store,i);
      if(clause)
      {
    if(ClauseQueryProp(clause, CPIsProofClause))
    {
       clause->stats.proof_distance = 0;
    }
    else
    {
       distance = 0;
       traverse = PTreeTraverseInit(clause->g_parents);
       while((cell = PTreeTraverseNext(traverse)))
       {
          parent = cell->key;
          distance = MAX(distance, parent->stats.proof_distance);
       }
       PTreeTraverseExit(traverse);
       clause->stats.proof_distance = distance+1;
    }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ProofDistanceDistrib()
//
//   Find the distribution of the distance from the proof in the
//   clauses with all properties in filter set. Return longest
//   value. Assumes that distrib is empty (i.e. freshly allocated or
//   reinitialized to NULL), and that proof distances are
//   precomputed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ProofDistanceDistrib(InfState_p state, ClauseProperties filter,
           PDArray_p distrib)
{
   long         i, old_val, res = 0;
   CompClause_p clause;

   for(i=0; i<state->clause_store->size; i++)
   {
      clause = PDArrayElementP(state->clause_store,i);
      if(clause)
      {
    if(ClauseQueryProp(clause, filter) &&
       (clause->stats.proof_distance!=PROOF_DIST_INFINITY))
    {
       old_val = PDArrayElementInt(distrib,
               clause->stats.proof_distance)+1;
       PDArrayAssignInt(distrib, clause->stats.proof_distance,
                old_val);
       res = MAX(res, clause->stats.proof_distance);
    }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: InfStateSelectExamples()
//
//   Select example clauses from an inf-state. If infstate describes a
//   successful search, select proof clauses (or clauses used to
//   derive the saturated system) and up to
///  proof_clauses*neg_proportion nearest neighbours, otherwise select
//   the neg_examples first clauses.
//
// Global Variables: -
//
// Side Effects    : Sets CPOpFlag in clauses.
//
/----------------------------------------------------------------------*/

long InfStateSelectExamples(InfState_p state, double neg_proportion, long
   neg_examples)
{
   long proof_clauses, total_clauses;
   long res;

   InfStateClausesDelProp(state, CPOpFlag);
   proof_clauses = ProofMarkProofClauses(state);
   ProofComputeParentNumbers(state);

   if(proof_clauses)
   {
      ProofComputeDistance(state);
      total_clauses = proof_clauses*(1+neg_proportion);
      res = select_examples_from_proof(state, total_clauses);
   }
   else
   {
      res = select_examples_from_non_proof(state,neg_examples);
   }
   return res;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


