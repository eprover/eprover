/*-----------------------------------------------------------------------

  File  : ccl_clausefunc.c

  Author: Stephan Schulz

  Contents

  Clause functions that need to know about sets.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: New, partitioned ccl_clausesets.h

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
// Function: ClauseCanonCompareRef()
//
///  Compare two indirectly pointed to clauses with
//   ClauseStructWeightLexCompare().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCanonCompareRef(const void *clause1ref, const void* clause2ref)
{
   const Clause_p* c1 = clause1ref;
   const Clause_p* c2 = clause2ref;

   return CMP(ClauseStructWeightLexCompare(*c1, *c2),0);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveLiteralRef()
//
//   Remove *lit from clause, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause and possibly clause->set->literals
//
/----------------------------------------------------------------------*/

void ClauseRemoveLiteralRef(Clause_p clause, Eqn_p *lit)
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
   clause->weight -= EqnStandardWeight(handle);
   EqnListDeleteElement(lit);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveLiteral()
//
//   Remove lit from clause, adjusting counters as necessary. This is
//   a lot less efficient then ClauseRemoveLiteralRef(), as we have to
//   search for the literal.
//
// Global Variables: -
//
// Side Effects    : Changes clause and possibly clause->set->literals
//
/----------------------------------------------------------------------*/

void ClauseRemoveLiteral(Clause_p clause, Eqn_p lit)
{
   EqnRef handle = &(clause->literals);

   while(*handle!=lit)
   {
      assert(*handle);
      handle = &((*handle)->next);
   }
   ClauseRemoveLiteralRef(clause, handle);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFlipLiteralSign()
//
//   Change the sign of lit, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause.
//
/----------------------------------------------------------------------*/

void ClauseFlipLiteralSign(Clause_p clause, Eqn_p lit)
{
   if(EqnIsPositive(lit))
   {
      clause->pos_lit_no--;
      clause->neg_lit_no++;
   }
   else
   {
      clause->neg_lit_no--;
      clause->pos_lit_no++;
   }
   EqnFlipProp(lit, EPIsPositive);
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

   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));

   removed += EqnListRemoveResolved(&(clause->literals));
   removed += EqnListRemoveDuplicates(clause->literals);

   if(removed)
   {
      clause->neg_lit_no = 0;
      clause->pos_lit_no = 0;
      handle = clause->literals;
      ClauseDelProp(clause, CPInitial|CPLimitedRW);

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
   if(removed)
   {
      ClausePushDerivation(clause, DCNormalize, NULL, NULL);
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
//   ClauseStructWeightLexCompare().
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
   ClauseSetSort(set, ClauseCanonCompareRef);

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
      ClauseDelProp(clause, CPInitial|CPLimitedRW);
      DocClauseModification(GlobalOut, OutputLevel, clause,
                            inf_ac_resolution, NULL, sig, NULL);
      ClausePushACResDerivation(clause, sig);
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
      if(XOR(positive,tmp)&&EqnSubsumeP(simpl,*handle))
      {
         return true;
      }
      handle = &((*handle)->next);
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseArchive()
//
//   Move clause into the archive. Create a fresh copy pointing to the
//   old clause in its derivation and return it. Also set the
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseArchive(ClauseSet_p archive, Clause_p clause)
{
   Clause_p newclause;

   assert(archive);
   assert(clause);

   newclause = ClauseFlatCopy(clause);
   ClausePushDerivation(newclause, DCCnfQuote, clause, NULL);

   ClauseSetInsert(archive, clause);

   return newclause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseArchiveCopy()
//
//   Create an archive copy of clause in archive. The
//   archive copy inherits info and derivation. The original loses
//   info, and gets a new derivation that points to the archive
//   copy. Returns pointer to the archived copy.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseArchiveCopy(ClauseSet_p archive, Clause_p clause)
{
   Clause_p archclause;

   assert(archive);
   assert(clause);

   archclause = ClauseFlatCopy(clause);

   archclause->info = clause->info;
   archclause->derivation = clause->derivation;
   clause->info       = NULL;
   clause->derivation = NULL;
   ClausePushDerivation(clause, DCCnfQuote, archclause, NULL);
   ClauseSetInsert(archive, archclause);

   return archclause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetArchiveCopy()
//
//   Create an archive copy of each clause in set in archive. The
//   archive copy inherits info and derivation. The original loses
//   info, and gets a new derivation that points to the archive copy.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseSetArchiveCopy(ClauseSet_p archive, ClauseSet_p set)
{
   Clause_p handle;

   assert(archive);
   assert(set);

   for(handle = set->anchor->succ; handle!= set->anchor;
       handle = handle->succ)
   {
      ClauseArchiveCopy(archive, handle);
   }
}


/*-----------------------------------------------------------------------
//
// Function:ClauseIsOrphaned()
//
//   Return true if the clause is orphaned, i.e. if one of the direct
//   premises of the original generating inferences that generated it
//   has been back-simplified.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsOrphaned(Clause_p clause)
{
   PStackPointer i = 1;
   DerivationCode op;
   Clause_p parent;

   assert(clause);

   //clause = follow_quote_chain(clause);

   if(!clause->derivation)
   {
      return false;
   }
   if(PStackEmpty(clause->derivation))
   {
      return false;
   }

   op = PStackElementInt(clause->derivation, 0);

   if(!DCOpIsGenerating(op))
   {
      return false;
   }
   if(DCOpHasCnfArg1(op))
   {
      parent = PStackElementP(clause->derivation, 1);
      if(ClauseQueryProp(parent,CPIsDead))
      {
         return true;
      }
      i++;
   }
   if(DCOpHasCnfArg2(op))
   {
      parent = PStackElementP(clause->derivation, 2);
      if(ClauseQueryProp(parent,CPIsDead))
      {
         return true;
      }
      i++;
   }
   while(i<PStackGetSP(clause->derivation) &&
         ((op = PStackElementInt(clause->derivation, i))==DCCnfAddArg))
   {
      i++;
      parent = PStackElementP(clause->derivation, i);
      if(ClauseQueryProp(parent,CPIsDead))
      {
         return true;
      }
      i++;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetDeleteOrphans()
//
//   Remove all orphaned clauses, returning the number of clauses
//   eliminated.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetDeleteOrphans(ClauseSet_p set)
{
   Clause_p handle;

   assert(set);
   assert(!set->demod_index);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      if(ClauseIsOrphaned(handle))
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
// Function: PStackClausePrint()
//
//   Print the clauses on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackClausePrint(FILE* out, PStack_p stack, char* extra)
{
   PStackPointer i;
   Clause_p clause;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      clause = PStackElementP(stack, i);
      ClausePrint(out, clause, true);
      if(extra)
      {
         fprintf(out, "%s", extra);
      }
      fputc('\n', out);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseEliminateNakedBooleanVariables()
//
//   If the clause containts boolean variables X and ~X, convert the
//   clause to {$true}. If the clause C contains only X replace the
//   clause with C[X |-> $false]. If the clause C contains only ~X replace
//   C with C[X |-> $true].
//
// Global Variables: -
//
// Side Effects    : literals field may be changed
//
/----------------------------------------------------------------------*/

bool ClauseEliminateNakedBooleanVariables(Clause_p clause)
{
   assert(!ClauseIsEmpty(clause));

   PStack_p all_lits       = ClauseToStack(clause);
   Eqn_p    lit            = NULL;
   Term_p   var            = NULL;
   bool     eliminated_var = false;
   const TB_p   bank       = clause->literals->bank;
   const Term_p true_term  = bank->true_term;
   const Term_p false_term = bank->false_term;
   Eqn_p    res            = NULL;
   Subst_p  subst          = SubstAlloc();

   while(!PStackEmpty(all_lits))
   {
      lit = PStackPopP(all_lits);

      if(EqnIsBoolVar(lit))
      {
         assert(TermIsVar(lit->lterm));
         var = lit->lterm;

         if(EqnIsPositive(lit))
         {
            if(var->binding && var->binding == true_term)
            {
               // there was a negative equation previously that bound
               // this variable -- which means we have X and ~X.
               EqnListFree(clause->literals);
               clause->literals = EqnCreateTrueLit(bank);
               assert(eliminated_var);
               break;
            }
            else
            {
               if(!var->binding)
               {
                  SubstAddBinding(subst, var, false_term);
               }
               EqnDelProp(lit, EPIsPositive);
               lit->lterm = true_term; // now lit becomes false and will be deleted 
               eliminated_var = true;
            }
         }
         else
         {
            if(var->binding && var->binding == false_term)
            {
               // analogous to the previous case
               EqnListFree(clause->literals);
               clause->literals = EqnCreateTrueLit(bank);
               assert(eliminated_var);
               break;
            }
            else
            {
               if(!var->binding)
               {
                  SubstAddBinding(subst, var, true_term);
               }
               lit->lterm = true_term;
               eliminated_var = true;
            }
         }
      }
   }

   if(eliminated_var)
   {
      res = EqnListCopyOpt(clause->literals);
      EqnListFree(clause->literals);
      clause->literals = res;
      ClauseRemoveSuperfluousLiterals(clause);
   }

   PStackFree(all_lits);
   SubstDelete(subst);
   return eliminated_var;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
