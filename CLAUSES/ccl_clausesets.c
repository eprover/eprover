/*-----------------------------------------------------------------------

  File  : ccl_clausesets.c

  Author: Stephan Schulz

  Contents

  Implementation of clause sets (doubly linked lists), with optional
  extras (in particular various indices)

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sun May 10 03:03:20 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_clausesets.h"



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
// Function: print_var_pattern()
//
//   Print a template for a function/predicate symbol.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_var_pattern(FILE* out, char* symbol, int arity, char*
                              var, char* alt_var, int exception)
{
   int i;
   char* prefix = "";
   alt_var = !alt_var ? "" : alt_var; // silence compiler warning

   fprintf(out, "%s(", symbol);

   for(i=1; i<= arity; i++)
   {
      fputs(prefix, out);
      if(i==exception)
      {
         fputs(alt_var, out);
      }
      else
      {
         fprintf(out, "%s%d", var, i);
      }
      prefix = ",";
   }
   fprintf(out, ")");
}


/*-----------------------------------------------------------------------
//
// Function:  eq_func_axiom_print()
//
//   Print the LOP substitutivity axiom(s) for a function symbol.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void eq_func_axiom_print(FILE* out, char* symbol, int arity,
                                bool single_subst)
{
   int i;
   char *prefix = "";

   if(single_subst)
   {
      for(i=1; i<=arity; i++)
      {
         fprintf(out, "equal(");
         print_var_pattern(out, symbol, arity, "X", "Y", i);
         fprintf(out, ",");
         print_var_pattern(out, symbol, arity, "X", "Z", i);
         fprintf(out, ") <- ");
         fprintf(out, "equal(Y,Z).\n");
      }
   }
   else
   {
      fprintf(out, "equal(");
      print_var_pattern(out, symbol, arity, "X", NULL,0);
      fprintf(out, ",");
      print_var_pattern(out, symbol, arity, "Y", NULL,0);
      fprintf(out, ") <- ");
      for(i=1; i<=arity; i++)
      {
         fprintf(out, "%sequal(X%d,Y%d)", prefix, i, i);
         prefix = ",";
      }
      fprintf(out, ".\n");
   }
}

/*-----------------------------------------------------------------------
//
// Function:  eq_pred_axiom_print()
//
//   Print the LOP substitutivity axiom(s) for a predicate symbol.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void eq_pred_axiom_print(FILE* out, char* symbol, int arity,
                                bool single_subst)
{
   int i;

   if(single_subst)
   {
      for(i=1; i<=arity; i++)
      {
         print_var_pattern(out, symbol, arity, "X", "Y", i);
         fprintf(out, " <- ");
         print_var_pattern(out, symbol, arity, "X", "Z", i);
         fprintf(out, ", equal(Y,Z).\n");
      }
   }
   else
   {
      print_var_pattern(out, symbol, arity, "X", NULL,0);
      fprintf(out, " <- ");
      print_var_pattern(out, symbol, arity, "Y", NULL,0);
      for(i=1; i<=arity; i++)
      {
         fprintf(out, ",equal(X%d,Y%d)", i, i);
      }
      fprintf(out, ".\n");
   }
}


/*-----------------------------------------------------------------------
//
// Function:  tptp_eq_func_axiom_print()
//
//   Print the TPTP substitutivity axiom(s) for a function symbol.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void tptp_eq_func_axiom_print(FILE* out, char* symbol, int arity,
                                     bool single_subst)
{
   int i;

   if(single_subst)
   {
      for(i=1; i<=arity; i++)
      {
         fprintf(out, "input_clause(eq_subst_%s%d, axiom, [++equal(",
                 symbol, i);
         print_var_pattern(out, symbol, arity, "X", "Y", i);
         fprintf(out, ",");
         print_var_pattern(out, symbol, arity, "X", "Z", i);
         fprintf(out, "),");
         fprintf(out, "--equal(Y,Z)]).\n");
      }
   }
   else
   {
      fprintf(out, "input_clause(eq_subst_%s, axiom, [++equal(",
              symbol);
      print_var_pattern(out, symbol, arity, "X", NULL,0);
      fprintf(out, ",");
      print_var_pattern(out, symbol, arity, "Y", NULL,0);
      fprintf(out, ")");
      for(i=1; i<=arity; i++)
      {
         fprintf(out, ",--equal(X%d,Y%d)", i, i);
      }
      fprintf(out, "]).\n");
   }
}

/*-----------------------------------------------------------------------
//
// Function:  tptp_eq_pred_axiom_print()
//
//   Print the TPTP substitutivity axiom(s) for a predicate symbol.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void tptp_eq_pred_axiom_print(FILE* out, char* symbol, int arity,
                                     bool single_subst)
{
   int i;

   if(single_subst)
   {
      for(i=1; i<=arity; i++)
      {
         fprintf(out, "input_clause(eq_subst_%s%d, axiom, [++",
                 symbol, i);
         print_var_pattern(out, symbol, arity, "X", "Y", i);
         fprintf(out, ",--");
         print_var_pattern(out, symbol, arity, "X", "Z", i);
         fprintf(out, ",--equal(Y,Z)]).\n");
      }
   }
   else
   {
      fprintf(out, "input_clause(eq_subst_%s, axiom, [++",
              symbol);
      print_var_pattern(out, symbol, arity, "X", NULL,0);
      fprintf(out, ",--");
      print_var_pattern(out, symbol, arity, "Y", NULL,0);
      for(i=1; i<=arity; i++)
      {
         fprintf(out, ",--equal(X%d,Y%d)", i, i);
      }
      fprintf(out, "]).\n");
   }
}


/*-----------------------------------------------------------------------
//
// Function: clause_set_extract_entry()
//
//   Remove a plain clause from a plain clause set.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

static void clause_set_extract_entry(Clause_p clause)
{
   int     i;
   Eval_p *root;

   assert(clause);
   assert(clause->set);
   assert(clause->set->members);

   if(clause->evaluations)
   {
      for(i=0; i<clause->evaluations->eval_no; i++)
      {
         root = (void*)&PDArrayElementP(clause->set->eval_indices, i);
         // This may fail (silently) if the clause evaluation was
         // added to a clause already in a set!
         EvalTreeExtractEntry(root,
                              clause->evaluations,
                              i);
      }
   }
   clause->pred->succ = clause->succ;
   clause->succ->pred = clause->pred;
   clause->set->literals-=ClauseLiteralNumber(clause);
   clause->set->members--;
   clause->set = NULL;
   clause->succ = NULL;
   clause->pred = NULL;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClauseSetAlloc()
//
//   Allocate an empty clause set that uses SysDate for (logical)
//   time-keeping.
//
// Global Variables: -
//
// Side Effects    : Memory management
//
/----------------------------------------------------------------------*/

ClauseSet_p ClauseSetAlloc(void)
{
   ClauseSet_p handle;

   handle = ClauseSetCellAlloc();

   handle->members = 0;
   handle->literals = 0;
   handle->anchor = ClauseCellAlloc();
   handle->anchor->literals = NULL;
   handle->anchor->pred = handle->anchor->succ = handle->anchor;
   handle->date = SysDateCreationTime();
   SysDateInc(&handle->date);
   handle->demod_index = NULL;
   handle->fvindex = NULL;

   handle->eval_indices = PDArrayAlloc(4,4);
   handle->eval_no = 0;

   handle->identifier = DStrAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFreeClauses()
//
//   Delete all clauses in set.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void ClauseSetFreeClauses(ClauseSet_p set)
{
   Clause_p handle;

   assert(set);

   while(!ClauseSetEmpty(set))
   {
      handle = ClauseSetExtractFirst(set);
      ClauseFree(handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFree()
//
//   Delete a clauseset.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseSetFree(ClauseSet_p junk)
{
   assert(junk);

   ClauseSetFreeClauses(junk);
   if(junk->demod_index)
   {
      PDTreeFree(junk->demod_index);
   }

   if(junk->fvindex)
   {
      FVIAnchorFree(junk->fvindex);
   }
   PDArrayFree(junk->eval_indices);
   ClauseCellFree(junk->anchor);
   DStrFree(junk->identifier);
   ClauseSetCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetStackCardinality()
//
//   Assume stack is a stack of clause sets. Return the number of
//   clauses in all the sets.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetStackCardinality(PStack_p stack)
{
   ClauseSet_p handle;
   PStackPointer i;
   long res = 0;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      handle = PStackElementP(stack, i);
      res += ClauseSetCardinality(handle);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetGCMarkTerms()
//
//   Mark all terms in the clause set for the garbage collection.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseSetGCMarkTerms(ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseGCMarkTerms(handle);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetInsert()
//
//   Insert a clause as the last clause into the clauseset.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

void ClauseSetInsert(ClauseSet_p set, Clause_p newclause)
{
   int    i;
#ifndef NDEBUG
   Eval_p test;
#endif
   Eval_p *root;

   assert(!newclause->set);

   newclause->succ = set->anchor;
   newclause->pred = set->anchor->pred;
   set->anchor->pred->succ = newclause;
   set->anchor->pred = newclause;
   newclause->set = set;
   set->members++;
   set->literals+=ClauseLiteralNumber(newclause);
   if(newclause->evaluations)
   {
      for(i=0; i<newclause->evaluations->eval_no; i++)
      {
         root = (void*)&(PDArrayElementP(newclause->set->eval_indices,i));
#ifndef NDEBUG
         test =
#endif
            EvalTreeInsert(root, newclause->evaluations, i);
         assert(!test);
      }
      set->eval_no = MAX(newclause->evaluations->eval_no, set->eval_no);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetInsertSet()
//
//   Move all clauses from from into set (leaving from empty, but not
//   deleted).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetInsertSet(ClauseSet_p set, ClauseSet_p from)
{
   Clause_p handle;
   long res = 0;

   while(!ClauseSetEmpty(from))
   {
      handle = ClauseSetExtractFirst(from);
      ClauseSetInsert(set, handle);
      res++;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetPDTIndexedInsert()
//
//   Insert a demodulator into the set and the sets index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetPDTIndexedInsert(ClauseSet_p set, Clause_p newclause)
{
   ClausePos_p pos;

   assert(set->demod_index);
   assert(ClauseIsUnit(newclause));

   ClauseSetInsert(set, newclause);
   pos          = ClausePosCellAlloc();
   pos->clause  = newclause;
   pos->literal = newclause->literals;
   pos->side    = LeftSide;
   pos->pos     = NULL;
   if(!PDTreeInsert(set->demod_index, pos))
   {
      // not inserted
      ClausePosCellFree(pos);
   }
   if(!EqnIsOriented(newclause->literals))
   {
      pos          = ClausePosCellAlloc();
      pos->clause  = newclause;
      pos->literal = newclause->literals;
      pos->side    = RightSide;
      pos->pos     = NULL;
      if(!PDTreeInsert(set->demod_index, pos))
      {
         // not inserted
         ClausePosCellFree(pos);
      }
   }
   ClauseSetProp(newclause, CPIsDIndexed);
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetIndexedInsert()
//
//   Insert an FVPackedClause clause into the set, taking care od of
//   all existing indexes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetIndexedInsert(ClauseSet_p set, FVPackedClause_p newclause)
{
   if(!set->demod_index)
   {
      ClauseSetInsert(set, newclause->clause);
   }
   else
   {
      ClauseSetPDTIndexedInsert(set, newclause->clause);
   }
   if(set->fvindex)
   {
      FVIndexInsert(set->fvindex, newclause);
      ClauseSetProp(newclause->clause, CPIsSIndexed);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetIndexedInsertClause()
//
//   Insert a plain clause into the set, taking care od of
//   all existing indexes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetIndexedInsertClause(ClauseSet_p set, Clause_p newclause)
{
   FVPackedClause_p pclause = FVIndexPackClause(newclause, set->fvindex);
   assert(newclause->weight == ClauseStandardWeight(newclause));
   ClauseSetIndexedInsert(set, pclause);
   FVUnpackClause(pclause);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetIndexedInsertClauseSet()
//
//   Update the standard weight of all clauses in source and insert
//   them into set (and the indices of set).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseSetIndexedInsertClauseSet(ClauseSet_p set, ClauseSet_p source)
{
   Clause_p handle;

   while(!ClauseSetEmpty(source))
   {
      handle = ClauseSetExtractFirst(source);
      handle->weight = ClauseStandardWeight(handle);
      ClauseSetIndexedInsertClause(set, handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetExtractEntry()
//
//   Remove a (possibly indexed) clause from a clause set.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetExtractEntry(Clause_p clause)
{
   assert(clause);
   assert(clause->set);

   /* ClausePCLPrint(stdout, clause, true); */

   if(ClauseQueryProp(clause, CPIsDIndexed))
   {
      assert(clause->set->demod_index);
      if(clause->set->demod_index)
      {
         assert(ClauseIsUnit(clause));
         PDTreeDelete(clause->set->demod_index, clause->literals->lterm,
                      clause);
         if(!EqnIsOriented(clause->literals))
         {
            PDTreeDelete(clause->set->demod_index,
                         clause->literals->rterm, clause);
         }
         ClauseDelProp(clause, CPIsDIndexed);
      }
   }
   if(ClauseQueryProp(clause, CPIsSIndexed))
   {
      FVIndexDelete(clause->set->fvindex, clause);
      ClauseDelProp(clause, CPIsSIndexed);
   }
   clause_set_extract_entry(clause);
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetExtractFirst()
//
//   Extract the first element of the set and return it. Return NULL
//   if set is empty.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetExtractFirst(ClauseSet_p set)
{
   Clause_p handle;

   if(ClauseSetEmpty(set))
   {
      return NULL;
   }
   handle = set->anchor->succ;
   assert(handle->set == set);
   ClauseSetExtractEntry(handle);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetDeleteEntry()
//
//   Delete a clause from the clause set.
//
// Global Variables: -
//
// Side Effects    : Changes tree, memory operations
//
/----------------------------------------------------------------------*/

void ClauseSetDeleteEntry(Clause_p clause)
{
   assert(clause);
   ClauseSetExtractEntry(clause);
   ClauseFree(clause);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindBest()
//
//   Find the best clause (i.e. the clause with the smallest
//   evaluation).
//
// Global Variables: -
//
// Side Effects    : Changes clause set
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindBest(ClauseSet_p set, int idx)
{
   Clause_p clause;
   Eval_p   evaluation;

   /* printf("I: %d", idx); */
   evaluation =
      EvalTreeFindSmallest(PDArrayElementP(set->eval_indices, idx), idx);

   if(!evaluation)
   {
      assert(set->anchor->succ == set->anchor);
      return NULL;
   }
   assert(evaluation->object);
   clause = evaluation->object;
   assert(clause->set == set);
   return clause;
}




/*-----------------------------------------------------------------------
//
// Function: ClauseSetPrint()
//
//   Print the clause set to the given stream.
//
// Global Variables: Only for term output
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPrint(FILE* out, ClauseSet_p set, bool fullterms)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClausePrint(out, handle, fullterms);
      fputc('\n', out);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetTSTPPrint()
//
//   Print the clause set in TSTP format to the given stream.
//
// Global Variables: Only for term output
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetTSTPPrint(FILE* out, ClauseSet_p set, bool fullterms)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseTSTPPrint(out, handle, fullterms, true);
      fputc('\n', out);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetPrintPrefix()
//
//   Print the clause set, one clause per line, with prefix prefix on
//   each line.
//
// Global Variables: For format (read only)
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPrintPrefix(FILE* out, char* prefix, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      fputs(prefix, out);
      ClausePrint(out, handle, true);
      fputc('\n', out);
   }
}




/*-----------------------------------------------------------------------
//
// Function: ClauseSetSort()
//
//   Sort a clause set according to the comparison function
//   given. Note: This is unnecssarily inefficient for evaluated
//   clauses! Reimplement if you need to use it for large evaluatied
//   sets!
//
// Global Variables: -
//
// Side Effects    : Memory operations, will reorganize evaluation
//                   tree
//
/----------------------------------------------------------------------*/

void ClauseSetSort(ClauseSet_p set, ComparisonFunctionType cmp_fun)
{
   PStack_p stack = PStackAlloc();
   Clause_p clause;
   PStackPointer i;

   while((clause = ClauseSetExtractFirst(set)))
   {
      clause->weight = ClauseStandardWeight(clause);
      PStackPushP(stack, clause);
   }
   assert(ClauseSetEmpty(set));

   PStackSort(stack, cmp_fun);

   for(i=0; i<PStackGetSP(stack); i++)
   {
      clause = PStackElementP(stack,i);
      ClauseSetInsert(set, clause);
   }
   /* ClauseSetPrint(GlobalOut, set, true); */
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetSetProp()
//
//   Set prop in all clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetSetProp(ClauseSet_p set, FormulaProperties prop)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseSetProp(handle, prop);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetDelProp()
//
//   Delete prop in all clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetDelProp(ClauseSet_p set, FormulaProperties prop)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseDelProp(handle, prop);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetSetTPTPType()
//
//   Set TPTP type in all clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetSetTPTPType(ClauseSet_p set, FormulaProperties type)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseSetTPTPType(handle, type);
   }
}




/*-----------------------------------------------------------------------
//
// Function: ClauseSetMarkCopies()
//
//   Mark clauses that are equivalent (modulo ClauseCompareFun) to
//   clauses that occur earlier in set. Returns number of marked
//   clauses.
//
// Global Variables: -
//
// Side Effects    : Sets property, memory operations, deletes parents
//                   of unmarked copies.
//
/----------------------------------------------------------------------*/

long ClauseSetMarkCopies(ClauseSet_p set)
{
   long res = 0;
   Clause_p handle, exists;
   PTree_p  store = NULL;

   assert(set);

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if((exists = PTreeObjStore(&store, handle,
                                 ClauseCompareFun)))
      {
         //if(!ClauseParentsAreSubset(exists, handle))
         //{
         //ClauseDetachParents(exists);
         //}
         ClauseSetProp(handle, CPDeleteClause);
         res++;
      }
   }
   PTreeFree(store);

   return res;
}





/*-----------------------------------------------------------------------
//
// Function: ClauseSetDeleteMarkedEntries()
//
//   Remove all clauses with property CPDeleteClause set. Returns
//   number of deleted clauses.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long ClauseSetDeleteMarkedEntries(ClauseSet_p set)
{
   long deleted = 0;
   Clause_p clause, handle;

   assert(set);

   handle = set->anchor->succ;

   while(handle != set->anchor)
   {
      clause = handle;
      handle = handle->succ;

      if(ClauseQueryProp(clause, CPDeleteClause))
      {
         deleted++;
         ClauseSetDeleteEntry(clause);
      }
   }
   return deleted;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetDeleteCopies()
//
//   Delete all but one occurence of a clause in set.
//
// Global Variables: -
//
// Side Effects    : Changes clause set, changes parent-rlation of
//                   kept copies.
//
/----------------------------------------------------------------------*/

long ClauseSetDeleteCopies(ClauseSet_p set)
{
   long res1, res2;

   assert(set);

   res1 = ClauseSetMarkCopies(set);
   res2 = ClauseSetDeleteMarkedEntries(set);
   UNUSED(res1); UNUSED(res2); assert(res1==res2);

   return res1;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetDeleteNonUnits()
//
//   Remove all non-empty-non-unit-clauses from set, return number of
//   clauses eliminated.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetDeleteNonUnits(ClauseSet_p set)
{
   Clause_p handle;

   assert(set);
   assert(!set->demod_index);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      if(ClauseLiteralNumber(handle)>1)
      {
         ClauseSetProp(handle,CPDeleteClause);
      }
      else
      {
         ClauseDelProp(handle,CPDeleteClause);
      }
      handle = handle->succ;
   }
   return ClauseSetDeleteMarkedEntries(set);
}




/*-----------------------------------------------------------------------
//
// Function: ClauseSetGetTermNodes()
//
//   Count the nodes of terms in the clauses of set as though they
//   were unshared.
//
// Global Variables: -
//
// Side Effects    : Potentially via ClauseWeight()
//
/----------------------------------------------------------------------*/

long ClauseSetGetTermNodes(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle != set->anchor; handle =
          handle->succ)
   {
      res += ClauseWeight(handle, 1, 1, 1, 1, 1, 1, true);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetMarkSOS()
//
//   Mark Set-of-Support clauses in set with CPIsSOS. Return size of
//   SOS.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetMarkSOS(ClauseSet_p set, bool tptp_types)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle != set->anchor; handle =
          handle->succ)
   {
      if((tptp_types && (ClauseQueryTPTPType(handle) == CPTypeConjecture))||
         (!tptp_types && (ClauseIsGoal(handle))))
      {
         ClauseSetProp(handle, CPIsSOS);
         res++;
      }
      else
      {
         ClauseDelProp(handle, CPIsSOS);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetTermSetProp()
//
//   Set prop in all term nodes in clause set.
//
// Global Variables: -
//
// Side Effects    : No serious ones
//
/----------------------------------------------------------------------*/

void ClauseSetTermSetProp(ClauseSet_p set, TermProperties prop)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle != set->anchor; handle =
          handle->succ)
   {
      ClauseTermSetProp(handle, prop);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetTBTermPropDelCount()
//
//   Delete prop in all term cells, return number of props encountered
//
// Global Variables: -
//
// Side Effects    : No serious ones
//
/----------------------------------------------------------------------*/

long ClauseSetTBTermPropDelCount(ClauseSet_p set, TermProperties prop)
{
   Clause_p handle;
   long res = 0;

   for(handle = set->anchor->succ; handle != set->anchor; handle =
          handle->succ)
   {
      res += ClauseTBTermDelPropCount(handle, prop);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetGetSharedTermNodes()
//
//   Return the number of shared term nodes used by set.
//
// Global Variables: -
//
// Side Effects    : Changes TPOpFlag
//
/----------------------------------------------------------------------*/

long ClauseSetGetSharedTermNodes(ClauseSet_p set)
{
   long res;

   ClauseSetTermSetProp(set, TPOpFlag);
   res = ClauseSetTBTermPropDelCount(set, TPOpFlag);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetParseList()
//
//   Parse a list of clauses into the set. Clauses are not
//   evaluated. Returns number of clauses parsed.
//
// Global Variables: Only for parsing of terms.
//
// Side Effects    : Input, memory operations, changes set.
//
/----------------------------------------------------------------------*/

long ClauseSetParseList(Scanner_p in, ClauseSet_p set, TB_p bank)
{
   long     count = 0;
   Clause_p clause;

   while(ClauseStartsMaybe(in))
   {
      clause = ClauseParse(in, bank);
      count++;
      ClauseSetInsert(set, clause);
   }
   return count;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetMarkMaximalTerms()
//
//   Orient all literals and mark all maximal terms and literals in
//   the set.
//
// Global Variables: -
//
// Side Effects    : Orients and marks literals
//
/----------------------------------------------------------------------*/

void ClauseSetMarkMaximalTerms(OCB_p ocb, ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseMarkMaximalTerms(ocb, handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetSortLiterals()
//
//   Sort literals in all clauses by cmp_fun.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseSetSortLiterals(ClauseSet_p set, ComparisonFunctionType cmp_fun)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseSortLiterals(handle, cmp_fun);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetListGetMaxDate()
//
//   Return the oldest date of the first limit elements from set of
//   demodulators in the array demodulators.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

SysDate ClauseSetListGetMaxDate(ClauseSet_p *demodulators, int limit)
{
   int i;
   SysDate res = SysDateCreationTime();

   for(i=0; i<limit; i++)
   {
      assert(demodulators[i]);
      res = SysDateMaximum(res, demodulators[i]->date);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFind()
//
//   Given a clause and a clause set, try to find the clause in the
//   set. This is only useful for debugging, as usually clause should
//   know about the set it is in!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFind(ClauseSet_p set, Clause_p clause)
{
   Clause_p handle, res = NULL;

   assert(set);
   assert(clause);

   for(handle = set->anchor->succ; handle!=set->anchor;
       handle=handle->succ)
   {
      if(handle == clause)
      {
         res = handle;
         assert(clause->set == set);
         break;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindById()
//
//   Given a clause ident and a clause set, try to find the clause in
//   the set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindById(ClauseSet_p set, long ident)
{
   Clause_p handle, res = NULL;

   assert(set);

   for(handle = set->anchor->succ; handle!=set->anchor;
       handle=handle->succ)
   {
      if(handle->ident == ident)
      {
         res = handle;
         assert(handle->set == set);
         break;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetRemoveEvaluations()
//
//   Remove all evaluations from the clauses in set.
//
// Global Variables: -
//
// Side Effects    : Memory operations...
//
/----------------------------------------------------------------------*/

void ClauseSetRemoveEvaluations(ClauseSet_p set)
{
   int i;
   Clause_p handle;
   for(i=0; i<set->eval_indices->size; i++)
   {
      PDArrayAssignP(set->eval_indices, i, NULL);
   }
   for(handle = set->anchor->succ; handle!=set->anchor;
       handle=handle->succ)
   {
      EvalsFree(handle->evaluations);
      handle->evaluations = NULL;
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFilterTrivial()
//
//   Given a clause set, remove all trivial tautologies from
//   it. Return number of clauses removed.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long ClauseSetFilterTrivial(ClauseSet_p set)
{
   Clause_p handle, next;
   long count = 0;

   assert(set);
   assert(!set->demod_index);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      next = handle->succ;

      assert(handle);

      if(ClauseIsTrivial(handle))
      {
         ClauseSetDeleteEntry(handle);
         count++;
      }
      handle = next;
   }
   return count;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFilterTautologies()
//
//   Given a clause set, remove all tautologies from it. Return number
//   of clauses removed.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long ClauseSetFilterTautologies(ClauseSet_p set, TB_p work_bank)
{
   Clause_p handle, next;
   long count = 0;

   assert(set);
   assert(!set->demod_index);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      next = handle->succ;

      assert(handle);

      if(ClauseIsTautology(work_bank, handle))
      {
         ClauseSetDeleteEntry(handle);
         count++;
      }
      handle = next;
   }
   return count;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindMaxStandardWeight()
//
//   Return a pointer to a clause with the largest standard weight
//   among clauses in set (or NULL if set is empty).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindMaxStandardWeight(ClauseSet_p set)
{
   long max_weight = 0, weight;
   Clause_p handle, res = NULL;

   assert(set);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      weight = ClauseStandardWeight(handle);
      if(weight > max_weight)
      {
         res = handle;
         max_weight = weight;
      }
      handle = handle->succ;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindEqDefinition()
//
//   If set contains an equality definition at or after start, return
//   the potential matching side (as a reduced clause position),
//   otherwise NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ClausePos_p ClauseSetFindEqDefinition(ClauseSet_p set, int min_arity,
                                      Clause_p start)
{
   Clause_p handle;
   ClausePos_p res = NULL;
   EqnSide side;

   if(!start)
   {
      start = set->anchor->succ;
   }

   for(handle = start;
       handle!=set->anchor;
       handle = handle->succ)
   {
      side = ClauseIsEqDefinition(handle, min_arity);
      if(side!=NoSide)
      {
         res = ClausePosCellAlloc();
         res->clause = handle;
         res->literal = handle->literals;
         res->side = side;
         res->pos = NULL;
         break;
      }
   }
   /* if(res)
      {
      printf(COMCHAR" EqDef found: ");
      ClausePrint(stdout, res->clause, true);
      printf(" Side %d\n", res->side);
      } */
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClausesSetDocInital()
//
//   If level >= 2, print all clauses as axioms.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetDocInital(FILE* out, long level, ClauseSet_p set)
{
   Clause_p handle;

   if(level>=2)
   {
      for(handle = set->anchor->succ; handle!=set->anchor; handle =
             handle->succ)
      {
         DocClauseCreation(out, OutputLevel, handle,
                           inf_initial, NULL, NULL,
                           NULL);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetPropDocQuote()
//
//   Quote all clauses in set for which all props are set.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPropDocQuote(FILE* out, long level, FormulaProperties prop,
                           ClauseSet_p set, char* comment)
{
   Clause_p handle;

   if(level>=2)
   {
      for(handle = set->anchor->succ; handle!=set->anchor; handle =
             handle->succ)
      {
         if(ClauseQueryProp(handle, prop))
         {
            DocClauseQuote(out, level, 2, handle, comment , NULL);
         }
      }
   }
}

#ifndef NDBUG

/*-----------------------------------------------------------------------
//
// Function: ClauseSetVerifyDemod()
//
//   Return true if pos->clause is in clause set, is a demodulator,
//   and if pos describes a potential maximal side in clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseSetVerifyDemod(ClauseSet_p demods, ClausePos_p pos)
{
   assert(demods);
   assert(pos);
   assert(TermPosIsTopPos(pos->pos));

   if(!ClauseSetFind(demods, pos->clause))
   {
      return false;
   }
   if(!ClauseIsDemodulator(pos->clause))
   {
      return false;
   }
   if((pos->side == RightSide) && EqnIsOriented(pos->clause->literals))
   {
      return false;
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: PDTreeVerifyIndex()
//
//   Check if all clauses in index are in demod as well.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PDTreeVerifyIndex(PDTree_p tree, ClauseSet_p demods)
{
   PStack_p stack = PStackAlloc(), trav;
   PDTNode_p handle;
   long             i;
   IntMapIter_p     iter;
   ClausePos_p      pos;
   PTree_p          entry;
   bool             res = true;

   PStackPushP(stack, tree->tree);

   while(!PStackEmpty(stack))
   {
      handle = PStackPopP(stack);

      if(!handle->entries)
      {
         iter = IntMapIterAlloc(handle->f_alternatives, 0, LONG_MAX);
         while((handle = IntMapIterNext(iter, &i)))
         {
            if(handle)
            {
               PStackPushP(stack, handle);
            }
         }
         IntMapIterFree(iter);

         PStack_p map_iter = PStackAlloc();
          map_iter = PObjMapTraverseInit(handle->v_alternatives, map_iter);
         while((handle = PObjMapTraverseNext(map_iter, NULL)))
         {
            PStackPushP(stack, handle);
         }
         PObjMapTraverseExit(map_iter);
         map_iter = PObjMapTraverseInit(handle->db_alternatives, map_iter);
         while((handle = PObjMapTraverseNext(map_iter, NULL)))
         {
            PStackPushP(stack, handle);
         }
         PObjMapTraverseExit(map_iter);
         PStackFree(map_iter);
      }
      else
      {
         trav = PTreeTraverseInit(handle->entries);
         while((entry = PTreeTraverseNext(trav)))
         {
            pos = entry->key;
            if(!ClauseSetFind(demods, pos->clause))
            {
               res = false;
               /* TODO: In HO it is possible that a clause will
                  not be indexed */
               /* printf("Fehler: %d\n", (int)pos->clause);
                  ClausePrint(stdout, pos->clause, true);
                  printf("\n"); */
            }
         }
         PTreeTraverseExit(trav);
      }
   }
   PStackFree(stack);

   return res;
}

#endif

/*-----------------------------------------------------------------------
//
// Function: EqAxiomsPrint()
//
//   Print the equality axioms (symmetry, transitivity, refexivity,
//   substitutivity) for the given signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EqAxiomsPrint(FILE* out, Sig_p sig, bool single_subst)
{
   FunCode i;
   int arity;

   if(OutputFormat == TPTPFormat)
   {
      fprintf(out,
              "input_clause(eq_reflexive, axiom, [++equal(X,X)]).\n"
              "input_clause(eq_symmetric, axiom,"
              " [++equal(X,Y),--equal(Y,X)]).\n"
              "input_clause(eq_transitive, axiom,"
              " [++equal(X,Z),--equal(X,Y),--equal(Y,Z)]).\n");
      for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
      {
         if((arity=SigFindArity(sig, i)))
         {
            if(SigIsPredicate(sig,i))
            {
               tptp_eq_pred_axiom_print(out, SigFindName(sig,i),
                                        arity, single_subst);
            }
            else
            {
               tptp_eq_func_axiom_print(out, SigFindName(sig,i),
                                        arity, single_subst);
            }
         }
      }
   }
   else if(OutputFormat == TSTPFormat)
   {
      Error("Adding of equality axioms not (yet) supported for TSTP/TPTP-3 format.",
            OTHER_ERROR);
   }
   else
   {
      fprintf(out,
              "equal(X,X) <- .\n"
              "equal(X,Y) <- equal(Y,X).\n"
              "equal(X,Z) <- equal(X,Y), equal(Y,Z).\n");
      for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
      {
         if((arity=SigFindArity(sig, i)))
         {
            if(SigIsPredicate(sig,i))
            {
               eq_pred_axiom_print(out, SigFindName(sig,i), arity,
                                   single_subst);
            }
            else
            {
               eq_func_axiom_print(out, SigFindName(sig,i), arity,
                                   single_subst);
            }
         }
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetAddSymbolDistribution()
//
//   Count the occurrences of function symbols in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetAddSymbolDistribution(ClauseSet_p set, long *dist_array)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseAddSymbolDistribution(handle, dist_array);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetAddTypeDistribution()
//
//   Count the occurrences of types of function symbols in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetAddTypeDistribution(ClauseSet_p set, long *type_array)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseAddTypeDistribution(handle, type_array);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetAddConjSymbolDistribution()
//
//   Count the occurrences of function symbols in conjectures in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetAddConjSymbolDistribution(ClauseSet_p set, long *dist_array)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         ClauseAddSymbolDistribution(handle, dist_array);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetAddAxiomSymbolDistribution()
//
//   Count the occurrences of function symbols in non-conjectures in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetAddAxiomSymbolDistribution(ClauseSet_p set, long *dist_array)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(!ClauseIsConjecture(handle))
      {
         ClauseAddSymbolDistribution(handle, dist_array);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetComputeFunctionRanks()
//
//   Assign to each function symbol a uniq number based on its
//   position in the clause set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetComputeFunctionRanks(ClauseSet_p set, long *rank_array,
                                   long* count)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseComputeFunctionRanks(handle, rank_array, count);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindFreqSymbol()
//
//   Find the most/least frequent non-special, non-predicate symbol of the
//   given arity in the clause set.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

FunCode ClauseSetFindFreqSymbol(ClauseSet_p set, Sig_p sig, int arity,
                                bool least)
{
   FunCode       i, selected=0;
   long          *dist_array,freq=least?LONG_MAX:0;

   dist_array = SizeMalloc((sig->f_count+1) * sizeof(long));

   for(i=0; i<=sig->f_count; i++)
   {
      dist_array[i] = 0;
   }
   ClauseSetAddSymbolDistribution(set, dist_array);

   for(i=sig->internal_symbols+1; i<= sig->f_count; i++)
   {
      if((SigFindArity(sig,i)==arity) && !SigIsPredicate(sig,i) &&
         !SigIsSpecial(sig,i))
      {
         if((least && (dist_array[i]<=freq)) ||
            (dist_array[i]>=freq))
         {
            freq = dist_array[i];
            selected = i;
         }
      }
   }
   SizeFree(dist_array, (sig->f_count+1)*sizeof(long));

   return selected;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetMaxVarNumber()
//
//   Return the largest number of variables occurring in any clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetMaxVarNumber(ClauseSet_p set)
{
   long res=0, tmp;
   PTree_p tree;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor; handle =
          handle->succ)
   {
      tree = NULL;
      tmp = ClauseCollectVariables(handle, &tree);
      PTreeFree(tree);
      res = MAX(res, tmp);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindCharFreqVectors()
//
//   Compute the characteristic frequency vectors for set. Vectors are
//   re-initialized. Returns number of clauses in set.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

long ClauseSetFindCharFreqVectors(ClauseSet_p set, FreqVector_p fsum,
                                  FreqVector_p fmax, FreqVector_p fmin,
                                  FVCollect_p cspec)
{
   Clause_p handle;
   FreqVector_p current;

   assert(set && fsum && fmax && fmin);

   FreqVectorInitialize(fsum, 0);
   FreqVectorInitialize(fmax, 0);
   FreqVectorInitialize(fmin, LONG_MAX);

   for(handle = set->anchor->succ;
       handle!= set->anchor;
       handle = handle->succ)
   {
      current = VarFreqVectorCompute(handle, cspec);
      FreqVectorAdd(fsum, fsum, current);
      FreqVectorMax(fmax, fmax, current);
      FreqVectorMin(fmin, fmin, current);
      FreqVectorFree(current);
   }
   return set->members;
}


/*-----------------------------------------------------------------------
//
// Function: PermVectorCompute()
//
//   Given a clause set and parameters for an index, compute a
//   suitable permutation vector (may be NULL if the parameters do not call
//   for a permutation vector!)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PermVector_p PermVectorCompute(ClauseSet_p set, FVCollect_p cspec,
                               bool eliminate_uninformative)
{
   PermVector_p res;
   FreqVector_p fsum, fmax, fmin;
   long vlen;

   if(cspec->features == FVINoFeatures)
   {
      return NULL;
   }

   if(cspec->features == FVICollectFeatures)
   {
      vlen = cspec->res_vec_len;
   }
   else
   {
      vlen = FVSize(cspec->max_symbols, cspec->features);
   }
   fsum = FreqVectorAlloc(vlen);
   fmax = FreqVectorAlloc(vlen);
   fmin = FreqVectorAlloc(vlen);

   ClauseSetFindCharFreqVectors(set,
                                fsum,
                                fmax,
                                fmin,
                                cspec);

   res = PermVectorComputeInternal(fmax, fmin, fsum,
                                   cspec->max_symbols,
                                   eliminate_uninformative);
   FreqVectorFree(fsum);
   FreqVectorFree(fmax);
   FreqVectorFree(fmin);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFVIndexify()
//
//   Remove all clauses from set and insert them again as indexed
//   clauses. Return number of clauses in set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetFVIndexify(ClauseSet_p set)
{
   PStack_p stack = PStackAlloc();
   Clause_p clause;

   assert(set);
   assert(set->fvindex);

   while((clause = ClauseSetExtractFirst(set)))
   {
      PStackPushP(stack, clause);
   }
   while(!PStackEmpty(stack))
   {
      clause = PStackPopP(stack);
      assert(clause->weight == ClauseStandardWeight(clause));
      ClauseSetIndexedInsertClause(set, clause);
   }
   PStackFree(stack);
   return set->members;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetNewTerms()
//
//   Substitute all clause in set with otherwise identical copies
//   taking terms from the new termbank.
//
// Global Variables: -
//
// Side Effects    : May indexify the clause set.
//
/----------------------------------------------------------------------*/

long ClauseSetNewTerms(ClauseSet_p set, TB_p terms)
{
   PStack_p stack = PStackAlloc();
   Clause_p clause, copy;

   assert(set);

   while((clause = ClauseSetExtractFirst(set)))
   {
      PStackPushP(stack, clause);
   }
   while(!PStackEmpty(stack))
   {
      clause = PStackPopP(stack);
      copy = ClauseCopy(clause, terms);
      assert(copy->weight == ClauseStandardWeight(copy));
      ClauseSetIndexedInsertClause(set, copy);
      ClauseFree(clause);
   }
   PStackFree(stack);
   return set->members;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSetSplitConjectures()
//
//   Find all (real or negated) conjectures in set and sort them into
//   conjectures. Collect the rest in rest. Return number of
//   conjectures found.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetSplitConjectures(ClauseSet_p set,
                               PList_p conjectures, PList_p rest)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         PListStoreP(conjectures, handle);
         res++;
      }
      else
      {
         PListStoreP(rest, handle);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetStandardWeight()
//
//   Return the sum of the standardweight of all clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long long ClauseSetStandardWeight(ClauseSet_p set)
{
   Clause_p  handle;
   long long res = 0;

   for(handle = set->anchor->succ;
       handle!=set->anchor;
       handle = handle->succ)
   {
      res += ClauseStandardWeight(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetDerivationStackStatistics()
//
//   Compute and print the stack depth distribution of the clauses in
//   set.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseSetDerivationStackStatistics(ClauseSet_p set)
{
   Clause_p handle;
   long     i;
   PDArray_p dist = PDArrayAlloc(8,8);
   double    sum = 0.0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle = handle->succ)
   {
      if(handle->derivation)
      {
         PDArrayElementIncInt(dist, PStackGetSP(handle->derivation), 1);
      }
      else
      {
         PDArrayElementIncInt(dist, 0, 1);
      }
   }
   for(i=0; i<PDArraySize(dist); i++)
   {
      printf(COMCHAR" %5ld: %6ld\n", i, PDArrayElementInt(dist,i));
      sum += PDArrayElementInt(dist,i)*i;
   }
   printf(COMCHAR" Average over %ld clauses: %f\n",
          ClauseSetCardinality(set),
          sum/ClauseSetCardinality(set));
   PDArrayFree(dist);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetPushClauses()
//
//   Push all clauses in set onto stack. Return number pushed.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetPushClauses(PStack_p stack, ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle = handle->succ)
   {
      PStackPushP(stack, handle);
      res++;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetDefaultWeighClauses()
//
//   Set the (standard) weight in all clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseSetDefaultWeighClauses(ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle = handle->succ)
   {
      handle->weight = ClauseStandardWeight(handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountConjectures()
//
//   Count and return number of conjectures (and negated_conjectures)
//   in set. Also find number of hypotheses,  and add it to *hypos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountConjectures(ClauseSet_p set, long* hypos)
{
   long ret = 0;
   Clause_p handle;


   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         ret++;
      }
      if(ClauseIsHypothesis(handle))
      {
         (*hypos)++;
      }
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseConjectureOrder()
//
//   Return the maximal order of the symbols that appear in the conjecture.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseConjectureOrder(ClauseSet_p set)
{
   int ord = 0;
   Clause_p handle;

   for(handle = set->anchor->succ;
       handle != set->anchor;
       handle = handle->succ)
   {
      for(Eqn_p lit = handle->literals; lit; lit = lit->next)
      {
         Sig_p sig = lit->bank->sig;
         ord = MAX(ord, MAX(TermComputeOrder(sig, lit->lterm),
                            TermComputeOrder(sig, lit->rterm)));
      }

   }
   return ord;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetIsUntyped
//
//   Returns true iff all clauses of the set are untyped
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
bool ClauseSetIsUntyped(ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle = handle->succ)
   {
      if (!ClauseIsUntyped(handle))
      {
         return false;
      }
   }
   return true;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
