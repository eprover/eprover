/*-----------------------------------------------------------------------

  File  : ccl_satinterface.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Functions for efficient conversion of the proof state to
  propositional clauses and submission to a SAT solver.


  Copyright 2017 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Sat Sep 16 16:52:43 CEST 2017

  -----------------------------------------------------------------------*/

#include "ccl_satinterface.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define PICOSAT_BUFSIZE 200

char* GroundingStratNames[] =
{
   "NoGrounding",
   "PseudoVar",
   "FirstConst",
   "ConjMinMinFreq",
   "ConjMaxMinFreq",
   "ConjMinMaxFreq",
   "ConjMaxMaxFreq",
   "GlobalMax",
   "GlobalMin",
   NULL
};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: sat_translate_literal()
//
//    Translate a full E literal into a propositional literal.
//
// Global Variables: -
//
// Side Effects    : May alloc and/or updare renumber index, memory
//                   operations, inserts terms into eqn->bank.
//
/----------------------------------------------------------------------*/

int sat_translate_literal(Eqn_p eqn, SatClauseSet_p set)
{
   int  atom = 0;
   long lit_code;
   Term_p lterm, rterm, lit_term;

   assert(eqn);
   assert(set);


   if(EqnIsEquLit(eqn))
   {
      lterm = TBInsertInstantiated(eqn->bank, eqn->lterm);
      rterm = TBInsertInstantiated(eqn->bank, eqn->rterm);
      // We normalize literals to avoid spurious failures
      if(PGreater(lterm, rterm))
      {
         lit_term = EqnTermsTBTermEncode(eqn->bank, lterm, rterm, true, PENormal);
      }
      else
      {
         lit_term = EqnTermsTBTermEncode(eqn->bank, lterm, rterm, true, PEReverse);
      }
   }
   else
   {
      lit_term = TBInsertInstantiated(eqn->bank, eqn->lterm);
   }
   lit_code = lit_term->entry_no;

   if(!set->renumber_index)
   {
      set->renumber_index = PDRangeArrAlloc(lit_code, 0);
   }
   atom = PDRangeArrElementInt(set->renumber_index, lit_code);
   if(!atom)
   {
      atom = ++set->max_lit;
      PDRangeArrAssignInt(set->renumber_index, lit_code, atom);
   }
   if(EqnIsPositive(eqn))
   {
      return atom;
   }
   return -atom;
}

/*-----------------------------------------------------------------------
//
// Function: litstate_add_satclause()
//
//   Add the literals of a clause to the literal state array (bit 0
//   indicates presence of positive instances of the atom, bit 1
//   represents pesence of negative instances).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void litstate_add_satclause(int* state, SatClause_p clause)
{
   int i, lit;
   assert(state);
   assert(clause);

   for(i=0; i<clause->lit_no; i++)
   {
      lit = clause->literals[i];
      if(lit > 0)
      {
         state[lit] |= 1;
      }
      else
      {
         state[-lit] |= 2;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: litstate_check_pure()
//
//   Given a SatClause and a literal state array, check if any of the
//   literals in the clause is pure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool litstate_check_pure(int* state, SatClause_p clause)
{
   int i, lit;
   assert(state);
   assert(clause);

   for(i = 0; i<clause->lit_no; i++)
   {
      lit = clause->literals[i];
      if(lit > 0)
      {
         assert(state[lit]);
         if(state[lit]!=3)
         {
            return true;
         }
      }
      else
      {
         assert(state[-lit]);
         if(state[-lit]!=3)
         {
            return true;
         }
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_min_max_freq()
//
//   Prefer conjecture symbols, among those the ones rarest in
//   conjectures, and among those the ones most frequent overall,
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_min_max_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   if(conj_dist_array[f1] && !conj_dist_array[f2])
   {
      return true;
   }
   if(!conj_dist_array[f1] && conj_dist_array[f2])
   {
      return false;
   }
   return (conj_dist_array[f1] < conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] > dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_max_max_freq()
//
//   Prefer symbols based on lexicographic comparision of conjecture
//   count, total count.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_max_max_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (conj_dist_array[f1] > conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] > dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_min_min_freq()
//
//   Prefer conjecture symbols, among those rare conjecture symbols,
//   and among those overall rare symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_min_min_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   if(conj_dist_array[f1] && !conj_dist_array[f2])
   {
      return true;
   }
   if(!conj_dist_array[f1] && conj_dist_array[f2])
   {
      return false;
   }
   return (conj_dist_array[f1] < conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] < dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_conj_max_min_freq()
//
//   Prefer symbols based on lexicographic comparision of conjecture
//   count, -total count.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_conj_max_min_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (conj_dist_array[f1] > conj_dist_array[f2])||
      ((conj_dist_array[f1] = conj_dist_array[f2]) &&
       (dist_array[f1] < dist_array[f2]));
}


/*-----------------------------------------------------------------------
//
// Function: prefer_global_max_freq()
//
//   Prefer most frequent symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_global_max_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (dist_array[f1] > dist_array[f2]);
}


/*-----------------------------------------------------------------------
//
// Function: prefer_global_min_freq()
//
//   Prefer least frequent symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool prefer_global_min_freq(FunCode f1, FunCode f2, long* conj_dist_array,
                              long* dist_array)
{
   return (dist_array[f1] < dist_array[f2]);
}




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SatClauseAlloc()
//
//   Allocate an empy, unlinked propositional clause with space for a
//   given number of literals. Note that the literals are not
//   initialized (not even to 0).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SatClause_p SatClauseAlloc(int lit_no)
{
   SatClause_p handle = SatClauseCellAlloc();

   handle->has_pure_lit = false;
   handle->lit_no       = lit_no;
   handle->literals     = SizeMalloc(handle->lit_no*sizeof(int));
   handle->source       = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseFree()
//
//    Free the SatClause.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void SatClauseFree(SatClause_p junk)
{
   assert(junk);

   SizeFree(junk->literals, junk->lit_no*sizeof(int));
   SatClauseCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetAlloc()
//
//   Allocate a SatClauseSet. This is much less flexible than full
//   clause sets (clauses can only be added), and also carries some
//   admin information for the translation from normal clauses to
//   propositional clauses.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

SatClauseSet_p SatClauseSetAlloc(void)
{
   SatClauseSet_p set = SatClauseSetCellAlloc();

   set->max_lit = 0;
   set->renumber_index = NULL; // We create this lazily when we know
                               // the first index!
   set->set = PStackAlloc();

   return set;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetFree()
//
//   Free a SatClauseSet (including the SatClauses).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SatClauseSetFree(SatClauseSet_p junk)
{
   assert(junk);
   SatClause_p clause;

   if(junk->renumber_index)
   {
      PDRangeArrFree(junk->renumber_index);
   }
   while(!PStackEmpty(junk->set))
   {
      clause = PStackPopP(junk->set);
      SatClauseFree(clause);
   }
   PStackFree(junk->set);
   SatClauseSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseCreateAndStore()
//
//    Encode the instantiated clause as a SatClause, store it in set,
//    and return it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

SatClause_p SatClauseCreateAndStore(Clause_p clause, SatClauseSet_p set)
{
   int i;
   Eqn_p lit;
   SatClause_p handle;

   assert(clause);
   assert(set);

   printf("# PGClause: ");
   ClausePrint(stdout, clause, true);
   printf("\n=>");
   EqnListPrintDeref(stdout, clause->literals, "|", DEREF_ONCE);
   printf("\n");

   handle = SatClauseAlloc(ClauseLiteralNumber(clause));
   handle->source = clause;
   for(i=0, lit=clause->literals;
       lit;
       i++, lit=lit->next)
   {
      assert(i<handle->lit_no);
      handle->literals[i] = sat_translate_literal(lit, set);
   }
   PStackPushP(set->set, handle);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: SatClausePrint()
//
//   Print a sat clause in DIMACS format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClausePrint(FILE* out, SatClause_p satclause)
{
   int i;

   assert(satclause);

   for(i=0; i<satclause->lit_no; i++)
   {
      fprintf(out, "%d ", satclause->literals[i]);
   }
   fprintf(out, "0\n");
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseSetPrint()
//
//   Print a SatClauseSet.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClauseSetPrint(FILE* out, SatClauseSet_p set)
{
   PStackPointer i;

   assert(set);

   fprintf(out, "p cnf %d %ld\n", set->max_lit, PStackGetSP(set->set));

   for(i=0; i<PStackGetSP(set->set); i++)
   {
      SatClausePrint(out, PStackElementP(set->set, i));
   }
   //printf("0\n");
}



/*-----------------------------------------------------------------------
//
// Function: SatClauseSetPrintNonPure()
//
//   Print a SatClauseSet.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void SatClauseSetPrintNonPure(FILE* out, SatClauseSet_p set, long pure)
{
   PStackPointer i;
   SatClause_p clause;

   assert(set);

   fprintf(out, "c Skipping %ld pure clauses\n", pure);
   fprintf(out, "p cnf %d %ld\n", set->max_lit, PStackGetSP(set->set)-pure);

   for(i=0; i<PStackGetSP(set->set); i++)
   {
      clause = PStackElementP(set->set, i);
      if(!clause->has_pure_lit)
      {
         SatClausePrint(out, clause);
      }
   }
   //printf("0\n");
}




/*-----------------------------------------------------------------------
//
// Function: SatClauseSetImportClauseSet()
//
//   Import all (instanciated) clauses from set into satset. Return
//   number of clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetImportClauseSet(SatClauseSet_p satset, ClauseSet_p set)
{
   Clause_p handle;
   assert(satset);
   assert(set);

   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      SatClauseCreateAndStore(handle, satset);
   }
   return ClauseSetCardinality(set);
}




/*-----------------------------------------------------------------------
//
// Function: SubstPseudoGroundVarBank()
//
//   Create a substitution binding all variables of a given sort to
//   the smallest (first) variable of that sort (to be interpreted as
//   an anonymous constant - this can be seen as a complete
//   (pseudo-)grounding of all terms, literals, and clauses using this
//   variable bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p SubstPseudoGroundVarBank(VarBank_p vars)
{
   Subst_p subst = SubstAlloc();
   VarBankStack_p varstack;
   long i, j, size;
   Term_p current, norm;

   assert(vars);

   for (i=0; i < PDArraySize(vars->varstacks); i++)
   {
      varstack = PDArrayElementP(vars->varstacks, i);
      // printf("# varstack: %p\n", varstack);
      if(varstack)
      {
         size = PStackGetSP(varstack);
         // printf("# varstack size: %ld\n", size);
         if(size)
         {
            norm = PStackElementP(varstack,0);
            // printf("# varstack[1]: %p\n", norm);
            for(j=0; j< size; j++)
            {
               current = PStackElementP(varstack,j);
               // printf("# varstack[%ld]: %p\n", j, current);
               if(current)
               {
                  SubstAddBinding(subst, current, norm);
               }
            }
         }
      }
   }
   return subst;
}


/*-----------------------------------------------------------------------
//
// Function: SubstGroundVarBankFirstConst()
//
//   Create a substitution binding each variable to the first constant
//   of the proper sort.
//
// Global Variables: 0
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p SubstGroundVarBankFirstConst(TB_p terms, bool norm_const)
{
   Subst_p subst = SubstAlloc();
   VarBank_p vars = terms->vars;
   VarBankStack_p varstack;
   long i, j, size;
   Term_p current, norm, backup;

   assert(vars);

   for (i=0; i < PDArraySize(vars->varstacks); i++)
   {
      varstack = PDArrayElementP(vars->varstacks, i);
      // printf("# varstack: %p\n", varstack);
      if (varstack)
      {
         size = PStackGetSP(varstack);
         // printf("# varstack size: %ld\n", size);
         if(size)
         {
            backup = PStackElementP(varstack,0);

            norm = TBGetFirstConstTerm(terms, i);
            if(!norm)
            {
               norm = backup;
            }
            else if(norm_const)
            {
               norm = TermFollowRWChain(norm);
            }
            // printf("# varstack[1]: %p\n", norm);
            for(j=0; j< size; j++)
            {
               current = PStackElementP(varstack,j);
               // printf("# varstack[%ld]: %p\n", j, current);
               if(current)
               {
                  SubstAddBinding(subst, current, norm);
               }
            }
         }
      }
   }
   return subst;
}


/*-----------------------------------------------------------------------
//
// Function: SubstGroundFreqBased()
//
//   Generate a grounding substitution using occurance-count based
//   preference functions.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p SubstGroundFreqBased(TB_p terms, ClauseSet_p clauses,
                             FunConstCmpFunType is_better, bool norm_const)
{
   Subst_p subst = SubstAlloc();
   VarBank_p vars = terms->vars;
   VarBankStack_p varstack;
   long i, j, size, dist_arr_size;
   Term_p current, norm, backup;
   long *conj_dist_array, *dist_array;

   assert(vars);

   dist_arr_size = (terms->sig->f_count+1) * sizeof(long);
   conj_dist_array = SizeMalloc(dist_arr_size);
   dist_array      = SizeMalloc(dist_arr_size);
   memset(conj_dist_array, 0, dist_arr_size);
   memset(dist_array, 0, dist_arr_size);

   ClauseSetAddSymbolDistribution(clauses, dist_array);
   ClauseSetAddConjSymbolDistribution(clauses, conj_dist_array);

   for (i=0; i < PDArraySize(vars->varstacks); i++)
   {
      varstack = PDArrayElementP(vars->varstacks, i);
      // printf("# varstack: %p\n", varstack);
      if (varstack)
      {
         size = PStackGetSP(varstack);
         // printf("# varstack size: %ld\n", size);
         if(size)
         {
            backup = PStackElementP(varstack,0);

            norm = TBGetFreqConstTerm(terms, i, conj_dist_array, dist_array, is_better);
            if(!norm)
            {
               norm = backup;
            }
            else if(norm_const)
            {
               norm = TermFollowRWChain(norm);
            }
            // printf("# varstack[1]: %p\n", norm);
            for(j=0; j< size; j++)
            {
               current = PStackElementP(varstack,j);
               // printf("# varstack[%ld]: %p\n", j, current);
               if(current)
               {
                  SubstAddBinding(subst, current, norm);
               }
            }
         }
      }
   }
   SizeFree(conj_dist_array, dist_arr_size);
   SizeFree(dist_array, dist_arr_size);

   return subst;
}

/*-----------------------------------------------------------------------
//
// Function: SatClauseSetImportProofState()
//
//   Import the all pseudo-grounded clauses in the proof state into
//   satset.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetImportProofState(SatClauseSet_p satset, ProofState_p state,
                                  GroundingStrategy strat, bool norm_const)
{
   long    res = 0;
   Subst_p pseudogroundsubst = NULL;
   assert(satset);
   assert(state);

   printf("# SatClauseSetImportProofState()\n");

   switch(strat)
   {
   case GMPseudoVar:
         pseudogroundsubst = SubstPseudoGroundVarBank(state->terms->vars);
         break;
   case GMFirstConst:
         pseudogroundsubst = SubstGroundVarBankFirstConst(state->terms,
                                                          norm_const);
         break;
   case GMConjMinMinFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_min_min_freq,
                                                  norm_const);
         break;
   case GMConjMaxMinFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_max_min_freq,
                                                  norm_const);
         break;
   case GMConjMinMaxFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_min_max_freq,
                                                  norm_const);
         break;
   case GMConjMaxMaxFreq:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_conj_max_max_freq,
                                                  norm_const);
         break;
   case GMGlobalMax:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_global_max_freq,
                                                  norm_const);
         break;
   case GMGlobalMin:
         pseudogroundsubst = SubstGroundFreqBased(state->terms,
                                                  state->axioms,
                                                  prefer_global_min_freq,
                                                  norm_const);
         break;
   default:
         assert(false && "Unimplmented grounding strategy");
         break;
   }
   // printf("# Pseudogrounded()\n");

   res += SatClauseSetImportClauseSet(satset, state->processed_pos_rules);
   res += SatClauseSetImportClauseSet(satset, state->processed_pos_eqns);
   res += SatClauseSetImportClauseSet(satset, state->processed_neg_units);
   res += SatClauseSetImportClauseSet(satset, state->processed_non_units);
   res += SatClauseSetImportClauseSet(satset, state->unprocessed);

   SubstDelete(pseudogroundsubst);
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: SatClauseSetMarkPure()
//
//   Mark all clauses in satset that have pure literals.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long SatClauseSetMarkPure(SatClauseSet_p satset)
{
   long pure_clauses = 0;
   int *litstate;
   PStackPointer i;
   SatClause_p clause;

   assert(satset);

   litstate = SizeMalloc(sizeof(int)*(satset->max_lit+1));
   memset(litstate, 0, sizeof(int)*(satset->max_lit+1));


   for(i=0; i< PStackGetSP(satset->set); i++)
   {
      clause = PStackElementP(satset->set, i);
      clause->has_pure_lit = false;
      litstate_add_satclause(litstate, clause);
      clause->has_pure_lit = false;
   }

   for(i=0; i< PStackGetSP(satset->set); i++)
   {
      clause = PStackElementP(satset->set, i);
      if(litstate_check_pure(litstate, clause))
      {
         clause->has_pure_lit = true;
         pure_clauses++;
      }
   }

   SizeFree(litstate, sizeof(int)*(satset->max_lit+1));

   return pure_clauses;
}


/*-----------------------------------------------------------------------
//
// Function: SatClauseSetCheckUnsat()
//
//   Check the satset for unsatisfiability. Return the empty clause if
//   unsat can be shown, NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : Runs external SAT solver, file operations, ...
//
/----------------------------------------------------------------------*/

Clause_p SatClauseSetCheckUnsat(SatClauseSet_p satset)
{
   static char buffer[PICOSAT_BUFSIZE];

   long pure = 0;
   Clause_p res = NULL;
   char *file, *data;
   FILE *fp, *picores;
   DStr_p cmd = DStrAlloc();

   pure = SatClauseSetMarkPure(satset);

   // printf("# SatCheck()... %ld marked pure\n", pure);

   file = TempFileName();
   fp   = SecureFOpen(file, "w");
   SatClauseSetPrintNonPure(fp, satset, pure);
   SecureFClose(fp);

   DStrAppendStr(cmd, "picosat -L 1 ");
   DStrAppendStr(cmd, file);
   picores = popen(DStrView(cmd), "r");

   while((data = fgets(buffer, PICOSAT_BUFSIZE, picores)))
   {
      //fprintf(GlobalOut, "# PICO: %s", data);
      if(strcmp(data, "s UNSATISFIABLE\n") == 0)
      {
         fprintf(GlobalOut, "# SatCheck found unsatisfiable ground set");
         res = EmptyClauseAlloc();
      }
   }
   pclose(picores);
   TempFileRemove(file);
   FREE(file);
   DStrFree(cmd);
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
