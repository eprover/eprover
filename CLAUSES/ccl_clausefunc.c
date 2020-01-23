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
// Function: unif_all_pairs()
//
//   Assuming that stack contains [s1, t1, s2, t2, ..., sn, tn]
//   computes simultaneous unifier of s1 =?= t1, ..., sn =?= tn
//   and stores it in subst.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool unif_all_pairs(PStack_p pairs, Subst_p subst)
{
   assert(PStackGetSP(pairs) % 2 == 0);
   PStackPointer pos = PStackGetSP(subst);
   bool unifies = true;

   while(unifies && !PStackEmpty(pairs))
   {
      Term_p s = PStackPopP(pairs);
      Term_p t = PStackPopP(pairs);

      unifies = SubstMguComplete(s, t, subst);
   }

   if (!unifies)
   {
      SubstBacktrackToPos(subst, pos);
   }
   return unifies;
}


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


/*-----------------------------------------------------------------------
//
// Function: ClauseRecognizeInjectivity()
//
//   Create a clause that postulates existence of an inverse function
//   for a given expression. In other words:
//
//           f s1 ... X ... sn != f t1 ... Y ... tn \/ X = Y
//    --------------------------------------------------------------
//            inv_f_i(f sigma(s1) ... X ... sigma(sn)) = X
//
//   where sigma is the **simultaneous** unifier of 
//   s1 =?= t1 ... sn =?= tn, and X,Y do not appear in any of the
//   s1, ..., sn, t1, ..., tn.
//   If sigma was a variable renaming, then resulting clause is tagged with
//   CPPureInjectivity. If inference was unsucessful, NULL is returned.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseRecognizeInjectivity(TB_p terms, Clause_p clause)
{
   assert(clause);
   Clause_p res = NULL;

   if(clause->pos_lit_no == 1 && clause->neg_lit_no == 1)
   {
      Eqn_p pos_lit = EqnIsPositive(clause->literals) ? 
                        clause->literals : clause->literals->next;
      Eqn_p neg_lit = EqnIsNegative(clause->literals) ? 
                        clause->literals : clause->literals->next;
      assert(EqnIsPositive(pos_lit));
      assert(EqnIsNegative(neg_lit));
      
      if (EqnIsEquLit(pos_lit) && EqnIsEquLit(neg_lit) &&
          TermIsVar(pos_lit->lterm) && TermIsVar(pos_lit->rterm) && 
          pos_lit->lterm != pos_lit->rterm &&
          !TermIsTopLevelVar(neg_lit->lterm) && !TermIsTopLevelVar(neg_lit->rterm)
          && neg_lit->lterm->f_code == neg_lit->rterm->f_code
          && neg_lit->lterm->arity > 0)
      {
         assert(neg_lit->lterm->arity == neg_lit->rterm->arity);

         int idx_var_occ = -1; // index where X or Y appears in f s1 ... X ... sn
         for (int i=0; idx_var_occ == -1 && i<neg_lit->lterm->arity; i++)
         {
            if (neg_lit->lterm->args[i] == pos_lit->lterm && 
                neg_lit->rterm->args[i] == pos_lit->rterm)
            {
               idx_var_occ = i;
            } 
            else if (neg_lit->lterm->args[i] == pos_lit->rterm &&
                     neg_lit->rterm->args[i] == pos_lit->lterm)
            {
               idx_var_occ = i;
            }
         }
         if (idx_var_occ != -1)
         {
            // collect pairs s_i =?= t_i on the stack.
            PStack_p arg_pairs = PStackAlloc();
            Subst_p subst = SubstAlloc();
            bool no_occurs = true; 

            for(int i=0; no_occurs && i<neg_lit->lterm->arity; i++)
            {
               if (i!=idx_var_occ)
               {
                  no_occurs = 
                     !TermHasFCode(neg_lit->lterm->args[i], pos_lit->lterm->f_code) &&
                     !TermHasFCode(neg_lit->rterm->args[i], pos_lit->lterm->f_code) &&
                     !TermHasFCode(neg_lit->lterm->args[i], pos_lit->rterm->f_code) &&
                     !TermHasFCode(neg_lit->rterm->args[i], pos_lit->rterm->f_code);
                  if (no_occurs)
                  {
                     PStackPushP(arg_pairs, neg_lit->lterm->args[i]);
                     PStackPushP(arg_pairs, neg_lit->rterm->args[i]);
                  }
               }
            }

            if (no_occurs && unif_all_pairs(arg_pairs, subst))
            {
               // substitution did not bind X or Y
               assert(!pos_lit->lterm->binding);
               assert(!pos_lit->rterm->binding);

               // f (sigma s_1) ... X ... (sigma s_n)
               Term_p inverse_arg = TBInsert(terms, neg_lit->lterm, DEREF_ALWAYS);
               // X 
               Term_p inverse_var = neg_lit->lterm->args[idx_var_occ];
               assert(inverse_var == inverse_arg->args[idx_var_occ]);

               Type_p args[1] = {neg_lit->lterm->type};

               FunCode new_inv_skolem_sym = 
                  SigGetNewTypedSkolem(terms->sig, args, 1, pos_lit->lterm->type);
               Term_p inv_skolem_term = TermTopAlloc(new_inv_skolem_sym, 1);
               inv_skolem_term->args[0] = inverse_arg;
               inv_skolem_term->type = pos_lit->lterm->type;
               inv_skolem_term = TBTermTopInsert(terms, inv_skolem_term);

               Eqn_p eqn = EqnAlloc(inv_skolem_term, inverse_var, terms, true);
               res = ClauseAlloc(eqn);
               res->proof_depth = clause->proof_depth+1;
               res->proof_size  = clause->proof_size+1;
               ClauseSetTPTPType(res, ClauseQueryTPTPType(clause));
               ClauseSetProp(res, ClauseGiveProps(clause, CPIsSOS));
               // TODO: Clause documentation is not implemented at the moment.
               // DocClauseCreationDefault(clause, inf_efactor, clause, NULL);
               ClausePushDerivation(res, DCInvRec, clause, NULL);
            }

            PStackFree(arg_pairs);
            SubstDelete(subst);
         }
      }
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ReplaceInjectivityDefs()
//
//   Replaces defintions of injectivity by clauses that
//   define inverse operators.
//
// Global Variables: -
//
// Side Effects    : Changes set and term bank.
//
/----------------------------------------------------------------------*/

long ClauseSetReplaceInjectivityDefs(ClauseSet_p set, ClauseSet_p archive, TB_p terms)
{
   Clause_p handle, next;
   ClauseSet_p tmp  = ClauseSetAlloc();
   long count = 0;

   assert(set);
   assert(!set->demod_index);

   handle = set->anchor->succ;
   while(handle != set->anchor)
   {
      assert(handle);
      next = handle->succ;

      Clause_p repl = ClauseRecognizeInjectivity(terms, handle);

      if(repl)
      {
         ClauseSetMoveClause(archive, handle);
         ClauseSetInsert(tmp, repl);
         count++;
      }
      handle = next;
   }

   ClauseSetInsertSet(set, tmp);
   assert(ClauseSetEmpty(tmp));
   ClauseSetFree(tmp);

   return count;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
