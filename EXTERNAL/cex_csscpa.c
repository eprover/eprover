/*-----------------------------------------------------------------------

File  : cex_csscpa.c

Author: Stephan Schulz, Geoff Sutcliffe

Contents

  Functions realizing the CSSCPA control component.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Apr  9 23:57:00 GMT 2000
    New

-----------------------------------------------------------------------*/

#include "cex_csscpa.h"



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
// Function: ClauseStatusString()
//
//   Return a string of the clause status
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
char* ClauseStatusString(ClauseStatusType clause_status)
{
   switch(clause_status)
   {
   case contradicts:
    return "contradicts";
   case rejected:
    return "rejected";
   case improved:
    return "improved";
   case forced:
    return "forced";
   case requested:
    return "requested";
   default:
    return "unknown";
   }

}
/*-----------------------------------------------------------------------
//
// Function: print_csscpa_state()
//
//   Print the clause status and state statistics given.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/
static void print_csscpa_state(CSSCPAState_p state,
                ClauseStatusType clause_status,
                Clause_p clause)
{
   fprintf(GlobalOut,COMCHAR" CSSCPAState: %-10s",ClauseStatusString(clause_status));
   fprintf(GlobalOut,
      " by %d, %ld, %ld, %ld (system, clauses,literals,weight)\n",
      clause ? ClauseQueryCSSCPASource(clause) : 0,
      state->clauses,
      state->literals,
      state->weight);
   fflush(GlobalOut); /* Redundant, but nevertheless. The output
          will probably be parsed after each new
          clause. */
}


/*-----------------------------------------------------------------------
//
// Function: collect_subsumed()
//
//   Push all clauses in set that are subsumed by clause onto
//   subsumed. Return weight of all these clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static long collect_subsumed(Clause_p clause, ClauseSet_p set,
              PStack_p subsumed)
{
   long res = 0;
   Clause_p handle, position;

   position = set->anchor->succ;

   if(ClauseIsUnit(clause))
   {
      while((handle = ClauseSetFindUnitSubsumedClause(set, position,
                        clause)))
      {
    PStackPushP(subsumed, handle);
    res+=handle->weight;
    position=handle->succ;
      }
   }
   else
   {
      while((handle = ClauseSetFindSubsumedClause(set, position,
                    clause)))
      {
    PStackPushP(subsumed, handle);
    res+=handle->weight;
    position=handle->succ;
      }

   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: find_unit_contradiction()
//
//   Given a (unit) clause and a clause set, check any of the unit
//   clauses with opposite sign in set for unifiability. Return the
//   first clause that unifies, otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static Clause_p find_unit_contradiction(Clause_p clause,CSSCPAState_p state)
{
   Clause_p    handle;
   Eqn_p       lit;
   ClauseSet_p set;

   assert(clause && ClauseIsUnit(clause));

   lit = clause->literals;
   if(EqnIsPositive(lit))
   {
      set = state->neg_units;
   }
   else
   {
      set = state->pos_units;
   }

   for(handle = set->anchor->succ; handle != set->anchor; handle =
     handle->succ)
   {
      assert(ClauseIsUnit(handle));
      assert(XOR(EqnIsPositive(lit), ClauseIsPositive(handle)));

      if(EqnUnifyP(lit, handle->literals))
      {
    return handle;
      }
   }
   return NULL;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: CSSCPAStateAlloc()
//
//   Allocate an empty, allocated CSSCPA state.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CSSCPAState_p CSSCPAStateAlloc(void)
{
   CSSCPAState_p handle = CSSCPAStateCellAlloc();

   handle->sig       = SigAlloc(TypeBankAlloc());
   handle->terms     = TBAlloc(handle->sig);
   handle->tmp_terms = TBAlloc(handle->sig);
   handle->pos_units = ClauseSetAlloc();
   handle->neg_units = ClauseSetAlloc();
   handle->non_units = ClauseSetAlloc();
   handle->pos_units->demod_index = PDTreeAlloc(handle->terms);
   handle->neg_units->demod_index = PDTreeAlloc(handle->terms);
   handle->literals  = 0;
   handle->clauses   = 0;
   handle->weight    = 0;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: CSSCPAStateFree()
//
//   Free a CSSCPAState and return associated data structures.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void CSSCPAStateFree(CSSCPAState_p junk)
{
   ClauseSetFree(junk->pos_units);
   ClauseSetFree(junk->neg_units);
   ClauseSetFree(junk->non_units);
   junk->terms->sig = NULL;
   TBFree(junk->terms);
   junk->tmp_terms->sig = NULL;
   TBFree(junk->tmp_terms);
   TypeBankFree(junk->sig->type_bank);
   SigFree(junk->sig);
   CSSCPAStateCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: CSSCPAProcessClause()
//
//   Process a clause for CSSCPA:
//   - If it is subsumed or tautological, delete it.
//   - If accept is true or clause subsumes clauses with a higher
//     combined weight than clause, remove all clauses subsume by
//     clause and add clause to state.
//   - Otherwise delete clause.
/   Returns true if clause has been accepted.
//
// Global Variables: -
//
// Side Effects    : Only the state changes above.
//
/----------------------------------------------------------------------*/

bool CSSCPAProcessClause(CSSCPAState_p state, Clause_p clause,
         bool accept, float weight_delta, float average_delta)
{
   PStack_p subsumed;
   long     sub_weight;
   Clause_p handle;
   ClauseStatusType clause_status = unknown;

   if (accept)
   {
      clause_status = forced;
   }

   if(ClauseIsTautology(state->tmp_terms,clause))
   {
      clause_status = rejected;
      if(OutputLevel)
      {
    fprintf(GlobalOut, COMCHAR" Clause %ld rejected (Tautology)\n",
       clause->ident);
      }
      ClauseFree(clause);
   }
   if(clause_status != rejected)
   {
      clause->weight = ClauseStandardWeight(clause);

      if((clause->pos_lit_no &&
          (handle=UnitClauseSetSubsumesClause(state->pos_units,
                  clause))) ||
         (clause->neg_lit_no &&
          (handle=UnitClauseSetSubsumesClause(state->neg_units,
                  clause))) ||
         ((ClauseLiteralNumber(clause)>1) &&
          (handle=ClauseSetSubsumesClause(state->non_units, clause))))
      {
         if(OutputLevel)
         {
       fprintf(GlobalOut, COMCHAR" Clause %ld rejected (subsumed by %ld)\n",
          clause->ident,handle->ident);
         }
         clause_status = rejected;
         ClauseFree(clause);
      }
   }

   if(clause_status != rejected)
   {
      subsumed = PStackAlloc();
      sub_weight = 0;

      if(ClauseIsUnit(clause)&&ClauseIsPositive(clause))
      {
         sub_weight+=collect_subsumed(clause,state->pos_units,subsumed);
      }
      else if(ClauseIsUnit(clause))
      {
         sub_weight+=collect_subsumed(clause,state->neg_units,subsumed);
      }
      sub_weight+=collect_subsumed(clause,state->non_units,subsumed);
      if((sub_weight-clause->weight) > (weight_delta*state->weight))
      {
         clause_status = improved;
      }
      else if(state->clauses &&
         (((state->weight + clause->weight)/
      (state->clauses + 1.0))
          <
          ((1.0-average_delta)*state->weight/state->clauses)))
      {
    clause_status = improved;
      }
      else
      {
    if(ClauseIsUnit(clause))
    {
       if(find_unit_contradiction(clause,state))
       {
/*----Added by Geoff */
          clause_status = contradicts;
          OUTPRINT(1, COMCHAR" Unit contradiction found!\n");
       }
    }
      }

/*----Added by Geoff */
      if(clause_status == contradicts || clause_status == forced || clause_status == improved)
      {
         while(!PStackEmpty(subsumed))
         {
       handle = PStackPopP(subsumed);
       state->clauses--;
       state->literals -= ClauseLiteralNumber(handle);
       state->weight   -= handle->weight;
            if(OutputLevel)
            {
          fprintf(GlobalOut,
             COMCHAR" Clause %ld removed from list (subsumed by %ld)\n",
             handle->ident, clause->ident);
            }
       ClauseSetDeleteEntry(handle);
         }
         state->clauses++;
         state->literals += ClauseLiteralNumber(clause);
         state->weight   += clause->weight;

         if(ClauseIsUnit(clause)&&ClauseIsPositive(clause))
         {
       ClauseSetIndexedInsertClause(state->pos_units,clause);
         }
         else if(ClauseIsUnit(clause))
    {
       ClauseSetIndexedInsertClause(state->neg_units,clause);
    }
    else
    {
       ClauseSetIndexedInsertClause(state->non_units,clause);
    }
         if(OutputLevel)
         {
       fprintf(GlobalOut,
          COMCHAR" Clause %ld accepted from %d (%s)\n",
          clause->ident, ClauseQueryCSSCPASource(clause),
            ClauseStatusString(clause_status));
         }
      }
      else
      {
         if(OutputLevel)
         {
       fprintf(GlobalOut, COMCHAR" Clause %ld rejected (weighty)\n",
          clause->ident);
         }
         clause_status = rejected;
      }
      PStackFree(subsumed);
   }

/*----Added by Geoff */
   if (clause_status == contradicts || clause_status == improved)
   {
      if (OutputLevel)
      {
         print_csscpa_state(state,clause_status, clause);
      }
   }

/*----Added by Geoff */
   fflush(GlobalOut);
   return (clause_status == contradicts) || (clause_status == improved) || (clause_status == forced);
}


/*-----------------------------------------------------------------------
//
// Function: CSSCPALoop()
//
//   Read CSSCPA-clause commands and process them. Terminate if no
//   input remains.
//
// Global Variables: -
//
// Side Effects    : Input, Output, memory operations.
//
/----------------------------------------------------------------------*/

void CSSCPALoop(Scanner_p in, CSSCPAState_p state)
{
   Clause_p         handle;
   bool             accept;
   long             source;
   float            weight_delta, average_delta;

   while(!TestInpTok(in, NoToken))
   {
      if(TestInpId(in, "output_level"))
      {
         NextToken(in);
         if(AktToken(in)->numval == 1 || AktToken(in)->numval == 0)
         {
             OutputLevel = AktToken(in)->numval == 1;
         }
    AcceptInpTok(in,PosInt);
         continue;
      }
      /*----User requests a state update */
      if(TestInpId(in, "state"))
      {
         NextToken(in);
         AcceptInpTok(in, Colon);
         print_csscpa_state(state,requested, NULL);
         continue;
      }
      /* What now follows is evil, but about the only way to reliably
    get around CLIB's input buffering: */
      if(TestInpId(in, "Please"))
      {
    AcceptInpId(in, "Please");
    AcceptInpId(in, "process");
    AcceptInpId(in, "clauses");
    AcceptInpId(in, "now");
    AcceptInpTok(in, Comma);
    AcceptInpId(in, "I");
    AcceptInpId(in, "beg");
    AcceptInpId(in, "you");
    AcceptInpTok(in, Comma);
    AcceptInpId(in, "great");
    AcceptInpId(in, "shining");
    AcceptInpId(in, "CSSCPA");
    AcceptInpTok(in, Comma);
    AcceptInpId(in, "wonder");
    AcceptInpId(in, "of");
    AcceptInpId(in, "the");
    AcceptInpId(in, "world");
    AcceptInpTok(in, Comma);
    AcceptInpId(in, "most");
    AcceptInpId(in, "beautiful");
    AcceptInpId(in, "program");
    AcceptInpId(in, "ever");
    AcceptInpId(in, "written");
    AcceptInpTok(in, Fullstop);
    continue;
      }

      CheckInpId(in, "accept|check");
      accept = false;
      if(TestInpId(in, "accept"))
      {
    accept = true;
      }
      NextToken(in);
      source = CP_CSSCPA_Unkown;
      if(TestInpId(in, "from"))
      {
    NextToken(in);
    source = AktToken(in)->numval;
    if((source < 2) || (source > 15))
    {
       AktTokenError(in, "CSSCPA source specifier must "
           "be in the range 2...15", false);
    }
    AcceptInpTok(in, PosInt);
      }
      else
      {
    source = 0;
      }

      /* Here is my suggestion: In order to stay compatible, I require
    an extra keyword here, otherwise the deltas are 0.0 (or
    whatever you like). Compare with >=, so that the default is
    to accept!

         Format: "improve(weight_delta, average_delta) */

      if(TestInpId(in, "improve"))
      {
    NextToken(in);
    AcceptInpTok(in, OpenBracket);
    weight_delta = ParseFloat(in);
    AcceptInpTok(in, Comma);
    average_delta = ParseFloat(in);
    AcceptInpTok(in, CloseBracket);
      }
      else
      {
    weight_delta = 0.0;
    average_delta = 0.0;
      }
      AcceptInpTok(in, Colon);
      handle = ClauseParse(in, state->terms);
      ClauseSetCSSCPASource(handle,source);
      CSSCPAProcessClause(state, handle, accept, weight_delta, average_delta);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
