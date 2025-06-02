/*-----------------------------------------------------------------------

  File  : ccl_clauses.c

  Author: Stephan Schulz

  Contents

  Clauses - Infrastructure functions

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Apr 16 19:38:16 MET DST 1998

  -----------------------------------------------------------------------*/


#include "ccl_clauses.h"
#include "ccl_tformulae.h"
#include "cte_lambda.h"
#include "ccl_clausesets.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

bool   ClausesHaveLocalVariables = true; /* Reassign variable numbers
                                            for each clause. This
                                            optimizes sharing, but
                                            will also loose
                                            inter-clause relationships
                                            needed for the analysis
                                            program */
bool   ClausesHaveDisjointVariables = false; /* Make certain that each
                                                newly parsed clause
                                                has fresh variables */


static long global_clause_counter = LONG_MIN; /* Counts calls to
                                                 ClauseAlloc(),
                                                 generates internal
                                                 identifiers. */

long ClauseIdentCounter = 0; /* Used to generate new clause idents on
                                the fly. */


#ifdef CLAUSE_PERM_IDENT
static long clause_perm_ident_counter = 0; /* Used to generate new
                                              permanent idents on
                                              the fly. */
#endif

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: foundEqLitLater
//
// Checks whether there is a literal (say L) in lits2 which is
// equivalent wrt. an ordering (given in ocb) to the first literal in
// lits1 (say M). In this case, the property `EPHasEquiv' is set for L
// and M.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool foundEqLitLater(OCB_p ocb, Eqn_p lits1, Eqn_p lits2)
{
   while (lits2)
   {
      if ((!EqnHasEquiv(lits2)) &&
          (LiteralCompare(ocb, lits1, lits2) == to_equal))
      {
         EqnSetProp(lits1, EPHasEquiv);
         EqnSetProp(lits2, EPHasEquiv);
         return true;
      }
      lits2 = lits2->next;
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: clause_copy_meta()
//
//   Return a copy of the clause cell, but without literals.
//
// Global Variables: -
//w
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p clause_copy_meta(Clause_p clause)
{
   Clause_p handle = ClauseCellAlloc();

   handle->ident       = clause->ident;
   handle->neg_lit_no  = clause->neg_lit_no;
   handle->pos_lit_no  = clause->pos_lit_no;
   handle->evaluations = NULL;
   handle->set         = NULL;
   handle->properties  = clause->properties;
   handle->info        = NULL;
   handle->derivation  = NULL;
   handle->feature_vec = NULL;
   handle->create_date = clause->create_date;
   handle->date        = clause->date;
   handle->proof_depth = clause->proof_depth;
   handle->proof_size  = clause->proof_size;
   handle->pred        = NULL;
   handle->succ        = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: clause_collect_posneg_vars()
//
//   Collect all the variables in positive literals of clause in
//   pos_vars, the ones of negative literals in neg_vars.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void clause_collect_posneg_vars(Clause_p clause,
                                PTree_p *pos_vars,
                                PTree_p *neg_vars)
{
   Eqn_p handle;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         (void)EqnCollectVariables(handle, pos_vars);
      }
      else
      {
         (void)EqnCollectVariables(handle, neg_vars);
      }
   }
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TSTPSkipSource()
//
//   Skip a TSTP source field.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

void TSTPSkipSource(Scanner_p in)
{
   AcceptInpTok(in, Identifier|PosInt);
   if(TestInpTok(in, OpenBracket))
   {
      ParseSkipParenthesizedExpr(in);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetTPTPType()
//
//   Set the TPTP type of a clause.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseSetTPTPType(Clause_p clause, FormulaProperties type)
{
   ClauseDelProp(clause,CPTypeMask);
   ClauseSetProp(clause, type);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCellAlloc()
//
//   Allocate a clause cell. This is a thin wrapper only relevant when
//   perm-idents are enabled for debugging.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseCellAlloc(void)
{
   Clause_p handle = ClauseCellAllocRaw();

#ifdef CLAUSE_PERM_IDENT
   handle->perm_ident = clause_perm_ident_counter++;
#endif


   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: EmptyClauseAlloc()
//
//   Return a pointer to an empty clause initialized with rational
//   values.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

Clause_p EmptyClauseAlloc(void)
{
   Clause_p handle = ClauseCellAlloc();

   handle->literals    = NULL;
   handle->neg_lit_no  = 0;
   handle->pos_lit_no  = 0;
   handle->weight      = 0;
   handle->evaluations = NULL;
   handle->properties  = CPIgnoreProps;
   handle->info        = NULL;
   handle->derivation  = NULL;
   handle->create_date = 0;
   handle->date        = SysDateCreationTime();
   handle->proof_depth = 0;
   handle->proof_size  = 0;
   handle->feature_vec = NULL;
   handle->set         = NULL;
   handle->pred        = NULL;
   handle->succ        = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseAlloc()
//
//   Create a new clause with the literal list list. Does sort literal
//   list by pos/neg-literals for easier comparison, does not use
//   EqnList functions because I'm a stupid performance freak.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseAlloc(Eqn_p literals)
{
   Clause_p handle = EmptyClauseAlloc();
   Eqn_p pos_lits=NULL, *pos_append = &pos_lits;
   Eqn_p neg_lits=NULL, *neg_append = &neg_lits;
   Eqn_p next;

   handle->ident = ++global_clause_counter;

   while(literals)
   {
      next = literals->next;
      if(EqnIsPositive(literals))
      {
         handle->pos_lit_no++;
         *pos_append = literals;
         pos_append = &((*pos_append)->next);
      }
      else
      {
         handle->neg_lit_no++;
         *neg_append = literals;
         neg_append = &((*neg_append)->next);
      }
      literals = next;
   }
   *pos_append = neg_lits;
   *neg_append = NULL;
   handle->literals = pos_lits;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRecomputeLitCounts()
//
//   Recompute the literal counts in clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseRecomputeLitCounts(Clause_p clause)
{
   Eqn_p handle;

   clause->pos_lit_no = 0;
   clause->neg_lit_no = 0;

   for(handle=clause->literals;
       handle;
       handle=handle->next)
   {
      if(EqnIsPositive(handle))
      {
         clause->pos_lit_no++;
      }
      else
      {
         clause->neg_lit_no++;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsTrivial()
//
//   Return true if the clause is trivial (because it has a trivial
//   true literal or propositionally conflicting literals).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsTrivial(Clause_p clause)
{
   if(EqnListFindTrue(clause->literals))
   {
      return true;
   }
   if(clause->pos_lit_no && clause->neg_lit_no)
   {
      if(ClauseLiteralNumber(clause) > EQN_LIST_LONG_LIMIT)
      {
         return EqnLongListIsTrivial(clause->literals);
      }
      return EqnListIsTrivial(clause->literals);
   }
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseHasMaxPosEqLit()
//
//   Return true if the clause has a maximal positive equational
//   literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseHasMaxPosEqLit(Clause_p clause)
{
   Eqn_p handle = clause->literals;

   while(handle)
   {
      if(EqnIsMaximal(handle) &&
         EqnIsEquLit(handle) &&
         EqnIsPositive(handle))
      {
         return true;
      }
      handle = handle->next;
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSortLiterals(Clause_p clause)
//
//   Sort literal order in clause according to the given comparison
//   function.
//
// Global Variables: -
//
// Side Effects    : Memory operations, reorders literals
//
/----------------------------------------------------------------------*/

Clause_p ClauseSortLiterals(Clause_p clause, ComparisonFunctionType cmp_fun)
{
   int lit_no = ClauseLiteralNumber(clause);

   if(lit_no >1)
   {
      int arr_size = lit_no*sizeof(Eqn_p), i;
      Eqn_p *sort_array = SizeMalloc(arr_size);
      Eqn_p handle;

      for(i=0, handle = clause->literals;
          handle;
          i++, handle = handle->next)
      {
         assert(i<lit_no);
         handle->pos = i;
         sort_array[i] = handle;
      }
      qsort(sort_array, lit_no, sizeof(Eqn_p), cmp_fun);

      clause->literals = EqnListFromArray(sort_array, lit_no);

      SizeFree(sort_array, arr_size);
   }
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCanonize(Clause_p clause)
//
//   Try to bring the clause into a canonical representation. Terms
//   are ordered by standard weight (except that $true is always
//   minimal), literals are ordered by sign, equality, standard
//   weight. Clauses should have no trivial literals!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseCanonize(Clause_p clause)
{
   Eqn_p handle = clause->literals;

   while(handle)
   {
      EqnCanonize(handle);
      handle = handle->next;
   }
   ClauseSortLiterals(clause, EqnCanonCompareRef);

   return clause;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseIsSorted()
//
//   Return true if clause is in order with respect to the
//   (quasi-)order defined by cmpfun.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool ClauseIsSorted(Clause_p clause, ComparisonFunctionType cmp_fun)
{
   Eqn_p handle;

   assert(clause);

   handle = clause->literals;

   if(ClauseLiteralNumber(clause) > 1)
   {
      assert(handle);
      while(handle->next)
      {
         assert(handle);
         assert(handle->next);
         if(cmp_fun(&(handle), &(handle->next))>0)
         {
            return false;
         }
         handle = handle->next;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStructWeightCompare()
//
//   Compare two clauses based on structure. Clauses are assumed to be
//   canonized and have correct weight. The ordering is:
//
//   Positive < mixed < negative
//   smaller number of neg literals < greater number of neg literals
//   smaller number of literals < greater number of literals
//   StandardWeight
//   Lexical extension of structural
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseStructWeightCompare(Clause_p c1, Clause_p c2)
{
   long tmp1 = 1, tmp2 = 1, res;
   Eqn_p handle1, handle2;

   assert(c1->weight == ClauseStandardWeight(c1));
   assert(c2->weight == ClauseStandardWeight(c2));

   if(ClauseIsPositive(c1))
   {
      tmp1 = 0;
   }
   else if(ClauseIsNegative(c1))
   {
      tmp1 = 2;
   }
   if(ClauseIsPositive(c2))
   {
      tmp2 = 0;
   }
   else if(ClauseIsNegative(c2))
   {
      tmp2 = 2;
   }
   res = tmp1 - tmp2;
   if(res)
   {
      return res;
   }
   res = c1->neg_lit_no - c2->neg_lit_no;
   if(res)
   {
      return res;
   }
   res = c1->pos_lit_no - c2->pos_lit_no; /* We know neg.lits are equal! */
   if(res)
   {
      return res;
   }
   res = c1->weight - c2->weight;
   if(res)
   {
      return res;
   }
   for(handle1=c1->literals, handle2=c2->literals;
       handle1;
       handle1=handle1->next,handle2=handle2->next)
   {
      assert(handle2);

      res = EqnStructWeightCompare(handle1, handle2);
      if(res)
      {
         return res;
      }
   }
   return 0;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseStructWeightLexCompare()
//
//   Compare two clauses based on structure, break ties by lexical
//   comparison, then by clause id.
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseStructWeightLexCompare(Clause_p c1, Clause_p c2)
{
   Eqn_p handle1, handle2;
   long res = ClauseStructWeightCompare(c1, c2);

   if(res)
   {
      return res;
   }
   for(handle1=c1->literals, handle2=c2->literals;
       handle1;
       handle1=handle1->next,handle2=handle2->next)
   {
      assert(handle2);

      res = EqnStructWeightLexCompare(handle1, handle2);
      if(res)
      {
         return res;
      }
   }
   return c1->ident - c2->ident;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseIsACRedundant()
//
//   Return true if clause is redundant with respect to AC symbols. It
//   is redundant if it is non-unit and has an AC-trivial literal or
//   if it is an AC-trivial unit with more than two function symbols.
//
// Global Variables: -
//
// Side Effects    : Only indirectly
//
/----------------------------------------------------------------------*/

bool ClauseIsACRedundant(Clause_p clause)
{
   if(ClauseIsUnit(clause)&&ClauseIsPositive(clause))
   {
      if(EqnStandardWeight(clause->literals)<=
         (4*DEFAULT_FWEIGHT+6*DEFAULT_VWEIGHT))
      {
         return false;
      }
   }
   return EqnListIsACTrivial(clause->literals);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFree()
//
//   Free a clause. Does not take care of parents, children, etc., but
//   releases the memory directly taken by the clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseFree(Clause_p junk)
{
   assert(!junk->set);
   EvalsFree(junk->evaluations);
   EqnListFree(junk->literals);
   ClauseInfoFree(junk->info);
   if(junk->derivation)
   {
      PStackFree(junk->derivation);
   }
   if(junk->feature_vec)
   {
      FixedDArrayFree(junk->feature_vec);
   }
   ClauseCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsSemFalse()
//
//   Return true if the clause has only PseudoLiterals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsSemFalse(Clause_p clause)
{
   Eqn_p handle;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      if(!EqnQueryProp(handle, EPPseudoLit))
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsSemEmpty()
//
//   Return true if the clause has only simple answer literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseIsSemEmpty(Clause_p clause)
{
   Eqn_p handle;

   for(handle=clause->literals; handle; handle = handle->next)
   {
      if(!EqnIsSimpleAnswer(handle))
      {
         return false;
      }
   }
   return true;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseIsRangeRestricted()
//
//   Return true if clause is range-restricted, i.e. if all variables
//   occuring in the tail (negative literals) also occur in the head
//   (positive literals).
//
// Global Variables: -
//
// Side Effects    : Changes TPOpFlag und TPCheckFlag in affected
//                   term cells
//
/----------------------------------------------------------------------*/

bool ClauseIsRangeRestricted(Clause_p clause)
{
   PTree_p pos_vars = NULL, neg_vars = NULL;
   bool    res;

   //printf("RR: "); ClausePrintAxiom(stdout, clause, true);
   if(ClauseIsPositive(clause) || ClauseIsGround(clause))
   {
      //printf("=> Ttrue\n");
      return true;
   }
   else if(ClauseIsNegative(clause))
   {
      //printf("=> Tfalse ");
      return false;
   }
   clause_collect_posneg_vars(clause, &pos_vars, &neg_vars);

   res = PTreeIsSubset(neg_vars, &pos_vars);

   PTreeFree(pos_vars);
   PTreeFree(neg_vars);

   //printf("=> %s\n", res?"true":"false");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsAntiRangeRestricted()
//
//   Return true if clause is anti-range-restricted, i.e. if all
//   variables occuring in the head also occur in the tail.
//
// Global Variables: -
//
// Side Effects    : Changes TPOpFlag und TPCheckFlag in affected
//                   term cells
//
/----------------------------------------------------------------------*/

bool ClauseIsAntiRangeRestricted(Clause_p clause)
{
   PTree_p pos_vars = NULL, neg_vars = NULL;
   bool    res;

   if(ClauseIsNegative(clause) || ClauseIsGround(clause))
   {
      return true;
   }
   else if(ClauseIsPositive(clause))
   {
      return false;
   }

   clause_collect_posneg_vars(clause, &pos_vars, &neg_vars);
   res = PTreeIsSubset(pos_vars, &neg_vars);

   PTreeFree(pos_vars);
   PTreeFree(neg_vars);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseIsStronglyRangeRestricted()
//
//   Return true if clause is strongly range-restricted, i.e. if
//   exactly the same variables occur in the tail and in the head.
//
// Global Variables: -
//
// Side Effects    : Changes TPOpFlag und TPCheckFlag in affected
//                   term cells
//
/----------------------------------------------------------------------*/

bool ClauseIsStronglyRangeRestricted(Clause_p clause)
{
   PTree_p pos_vars = NULL, neg_vars = NULL;
   bool res;

   //printf("StrongRR: "); ClausePrintAxiom(stdout, clause, true);
   if(ClauseIsEmpty(clause) || ClauseIsGround(clause))
   {
      //printf("=> Ttrue\n");
      return true;
   }
   else if(ClauseIsPositive(clause))
   {
      //printf("=> Tfalse\n");
      return false;
   }
   else if(ClauseIsNegative(clause))
   {
      return false;
   }

   assert(clause->literals);

   clause_collect_posneg_vars(clause, &pos_vars, &neg_vars);
   res = PTreeEquiv(pos_vars, neg_vars);

   PTreeFree(pos_vars);
   PTreeFree(neg_vars);
   //printf("=> %s\n", res?"true":"false");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsEqDefinition()
//
//   If clause is a positive unit definition f(X1....Xn)=t (f not in
//   t), return the definitional side, otherwise NoSide.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

EqnSide ClauseIsEqDefinition(Clause_p clause, int min_arity)
{
   assert(clause);

   if(ClauseIsUnit(clause))
   {
      return EqnIsDefinition(clause->literals, min_arity);
   }
   return NoSide;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseExtractHODefinition()
//
//   Given a
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseExtractHODefinition(Clause_p clause, EqnSide def_side, Term_p *lside, Term_p* rside)
{
   assert(clause);
   assert(def_side == LeftSide || def_side == RightSide);
   PStack_p vars = PStackAlloc();
   const Eqn_p lit = clause->literals;
   assert(lside != &(lit->lterm) && rside != &(lit->rterm));
   Term_p def_term = def_side == LeftSide ? lit->lterm : lit->rterm;
   for(int i=0; i<def_term->arity; i++)
   {
      assert(TermIsFreeVar(def_term->args[i]));
      PStackPushP(vars, def_term->args[i]);
   }
   Term_p other_term = def_side == LeftSide ? lit->rterm : lit->lterm;
   Term_p abstracted_term = AbstractVars(lit->bank, other_term, vars);
   Term_p symbol = TermTopAlloc(def_term->f_code, 0);
   symbol->type = abstracted_term->type;
   assert(symbol->type == SigGetType(lit->bank->sig, symbol->f_code));
   symbol = TBTermTopInsert(lit->bank, symbol);
   PStackFree(vars);

   *lside = symbol;
   *rside = abstracted_term;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCopy()
//
//   Create a semantically equivalent clause and return a pointer to
//   it. Does not copy parents, children, etc. Terms in the original
//   clause are interpreted as instantiated, and are created in or
//   retrived from the new bank. Evaluations are not copied, and
//   neither is info.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseCopy(Clause_p clause, TB_p bank)
{
   Clause_p handle = clause_copy_meta(clause);

   handle->literals = EqnListCopy(clause->literals, bank);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFlatCopy()
//
// As ClauseCopy(), but use the same bank as in clause, and ignore
// instantiations.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseFlatCopy(Clause_p clause)
{
   Clause_p handle = clause_copy_meta(clause);

   handle->literals = EqnListFlatCopy(clause->literals);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseCopyOpt()
//
//   Copy a (possibly instantiated) clause using the "same term bank"
//   optimizations.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseCopyOpt(Clause_p clause)
{
   Clause_p handle = clause_copy_meta(clause);

   handle->literals = EqnListCopyOpt(clause->literals);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseCopyDisjoint()
//
//   Create a variable-disjoint copy of clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseCopyDisjoint(Clause_p clause)
{
   Clause_p handle = clause_copy_meta(clause);

   handle->literals = EqnListCopyDisjoint(clause->literals);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSkolemize();
//
//   Return a skolemized copy of clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseSkolemize(Clause_p clause, TB_p bank)
{
   Subst_p  subst = SubstAlloc();
   Eqn_p    handle;
   Clause_p res;

   assert(clause);
   assert(bank);

   for(handle=clause->literals; handle; handle=handle->next)
   {
      EqnSkolemSubst(handle, subst, bank->sig);
   }
   res = ClauseCopy(clause,bank);
   SubstDeleteSkolem(subst);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrintList()
//
//   Print a clause as a declarative list of literals.
//
// Global Variables: Only indirectly (Equation and term formats)
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintList(FILE* out, Clause_p clause, bool fullterms)
{
   EqnListPrint(out, clause->literals, "; ", false, fullterms);
   fprintf(out, " <-.");
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrintAxiom()
//
//   Print a clause as a declarative axiom in normal form, i.e. print
//   positive literals first, then <-, then negative literals.
//
// Global Variables: Only indirectly (Equation and term formats)
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintAxiom(FILE* out, Clause_p clause, bool fullterms)
{
   Eqn_p handle;
   int   i;

   i = 0;
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         EqnPrint(out, handle, false, fullterms);
         i++;
         if(i<clause->pos_lit_no)
         {
            fprintf(out, "; ");
         }
      }
   }
   fprintf(out, " <- ");

   i = 0;
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(!EqnIsPositive(handle))
      {
         EqnPrint(out, handle, true, fullterms);
         i++;
         if(i<clause->neg_lit_no)
         {
            fprintf(out, ", ");
         }
      }
   }
   fprintf(out, ".");
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrintRule()
//
//   Print a clause as a rule, with the head literal as the conclusion
//   and the remaining literals as preconditions.  If a
//   clause has a single literal only, print it as a fact.
//
// Global Variables: Only indirectly (Equation and term formats)
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintRule(FILE* out, Clause_p clause, bool fullterms)
{
   if(clause->literals)
   {
      EqnPrint(out, clause->literals, false, fullterms);
      if(clause->literals->next)
      {
         fprintf(out, " <- ");
         EqnListPrint(out, clause->literals->next, ", ", true,
                      fullterms);
      }
   }
   else
   {
      fprintf(out, " <- ");
   }
   fprintf(out, ".");
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrintGoal()
//
//   Print a clause as a goal, i.e. put all literals behind <-.
//
// Global Variables: Only indirectly (Equation and term formats)
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintGoal(FILE* out, Clause_p clause, bool fullterms)
{
   fprintf(out, "<- ");
   EqnListPrint(out, clause->literals, ", ", true, fullterms);
   fprintf(out, ".");
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrintQuery()
//
//   Print a clause as a procedural query, i.e. put all literals
//   behind ?-.
//
// Global Variables: Only indirectly (Equation and term formats)
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintQuery(FILE* out, Clause_p clause, bool fullterms)
{
   assert(ClauseLiteralNumber(clause));

   fprintf(out, "?- ");
   EqnListPrint(out, clause->literals, ", ", true, fullterms);
   fprintf(out, ".");
}

/*-----------------------------------------------------------------------
//
// Function: ClausePrintTPTPFormat()
//
//   Print a clause in TPTP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintTPTPFormat(FILE* out, Clause_p clause)
{
   char* typename;
   int   source;

   switch(ClauseQueryTPTPType(clause))
   {
   case CPTypeAxiom:
         typename = "axiom";
         break;
   case CPTypeHypothesis:
         typename = "hypothesis";
         break;
   case CPTypeConjecture:
   case CPTypeNegConjecture:
         typename = "conjecture";
         break;
   case CPTypeLemma:
         typename = "lemma";
         break;
   case CPTypeWatchClause:
         typename = "watchlist";
         break;
   default:
         typename = "unknown";
         break;
   }
   source = ClauseQueryCSSCPASource(clause);

   if(clause->ident >= 0)
   {
      fprintf(out, "input_clause(c_%d_%ld,%s,[",
              source, clause->ident, typename);
   }
   else
   {
      fprintf(out, "input_clause(i_%d_%ld,%s,[",
              source, clause->ident-LONG_MIN, typename);
   }
   EqnListPrint(out, clause->literals, ",", false, true);
   fprintf(out, "]).");
}

/*-----------------------------------------------------------------------
//
// Function: ClausePrintLOPFormat()
//
//   Print a clause in LOP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintLOPFormat(FILE* out, Clause_p clause, bool fullterms)
{
   if(((ClauseQueryTPTPType(clause) == CPTypeConjecture)||
       (ClauseQueryTPTPType(clause) == CPTypeNegConjecture) )&&
      !ClauseIsEmpty(clause))
   {
      ClausePrintQuery(out, clause, fullterms);
   }
   else
   {
      ClausePrintAxiom(out, clause, fullterms);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClausePrint()
//
//   Print a clause in the most canonical representation.
//
// Global Variables: OutputFormat
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrint(FILE* out, Clause_p clause, bool fullterms)
{
   if(out == stderr)
   {
      fprintf(stderr, "ClausePrint(stderr,...)\n");
   }

   /* fprintf(out, "(%ld, %ld)", clause->proof_depth,
      clause->proof_size); */

/* #define PRINT_SOS_PROP 1 */
#ifdef PRINT_SOS_PROP
   if(ClauseQueryProp(clause, CPIsSOS))
   {
      fprintf(out, "/* SoS */");
   }
   else
   {
      fprintf(out, "/* --- */");
   }
#endif

   if(OutputFormat == TPTPFormat)
   {
      ClausePrintTPTPFormat(out, clause);
   }
   else if(OutputFormat == TSTPFormat)
   {
      ClauseTSTPPrint(out, clause, fullterms, true);
   }
   else
   {
      ClausePrintLOPFormat(out, clause, fullterms);
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClausePrintDBG()
//
//   Print a clause in the form useful for debugging.
//
// Global Variables: OutputFormat
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePrintDBG(FILE* out, Clause_p clause)
{
   fprintf(out, "thf(cl%ld, plain, ",
           clause->ident >= 0 ? clause->ident : clause->ident-LONG_MIN);
   if(clause->literals)
   {
      EqnPrintDBG(out, clause->literals);
      for(Eqn_p lit = clause->literals->next; lit; lit = lit->next)
      {
         DBG_PRINT(out, " | ", EqnPrintDBG(out, lit), "");
      }
   }
   fprintf(out, " ).");
}


/*-----------------------------------------------------------------------
//
// Function: ClausePCLPrint()
//
//   Print a clause in PCL format.
//
// Global Variables: OutputFormat
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClausePCLPrint(FILE* out, Clause_p clause, bool fullterms)
{
   IOFormat tptp_format_hack = OutputFormat;

#ifdef PRINT_SOS_PROP
   if(ClauseQueryProp(clause, CPIsSOS))
   {
      fprintf(out, "/* SoS */");
   }
   else
   {
      fprintf(out, "/* --- */");
   }
#endif
/* #define PCLPRINTDEGBUG */
#ifdef PCLPRINTDEGBUG
   if(clause->literals)
   {
      fprintf(out, "/* %d; %lu; %lu(%lu)=%lu(%lu)*/",
              ClauseQueryProp(clause, CPLimitedRW),
              clause->date,
              clause->literals->lterm->entry_no,
              clause->literals->lterm->rw_data.nf_date[RewriteAdr(FullRewrite)],
              clause->literals->rterm->entry_no,
              clause->literals->rterm->rw_data.nf_date[RewriteAdr(FullRewrite)]
         );
   }
#endif
   OutputFormat = TPTPFormat;
   fputc('[', out);
   EqnListPrint(out, clause->literals, ",", false, fullterms);
   fputc(']', out);
   OutputFormat = tptp_format_hack;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTSTPCorePrint()
//
//   Print a core clause in TSTP format.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseTSTPCorePrint(FILE* out, Clause_p clause, bool fullterms)
{
   fputc('(', out);
   if(ClauseIsEmpty(clause))
   {
      fprintf(out, "$false");
   }
   else
   {
      EqnListTSTPPrint(out, clause->literals, "|", fullterms);
   }
   fputc(')', out);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseTSTPPrint()
//
//   Print a clause in TSTP format. If complete is true, terminate
//   clause properly, otherwise stop after the logical part.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseTSTPPrint(FILE* out, Clause_p clause, bool fullterms, bool complete)
{
   int source;
   char *typename = "plain", *kind = "cnf";
   bool is_untyped = ClauseIsUntyped(clause);
   TFormula_p form = NULL;

   // quantify and print as TFF formula
   if(!is_untyped && problemType == PROBLEM_FO)
   {
      kind = "tcf";
   }
   else if(problemType == PROBLEM_HO)
   {
      kind = "thf";
   }

   switch(ClauseQueryTPTPType(clause))
   {
   case CPTypeAxiom:
         if(ClauseQueryProp(clause, CPInputFormula))
         {
            typename = "axiom";
         }
         break;
   case CPTypeHypothesis:
         typename = "hypothesis";
         break;
   case CPTypeConjecture:
         typename = "conjecture";
         break;
   case CPTypeLemma:
         typename = "lemma";
         break;
   case CPTypeWatchClause:
         typename = "watchlist";
         break;
   case CPTypeNegConjecture:
         typename = "negated_conjecture";
         break;
   default:
         break;
   }
   source = ClauseQueryCSSCPASource(clause);
   if(clause->ident >= 0)
   {
      fprintf(out, "%s(c_%d_%ld, ",
              kind,
              source,
              clause->ident);
   }
   else
   {
      fprintf(out, "%s(i_%d_%ld, ",
              kind,
              source,
              clause->ident-LONG_MIN);
   }
   fprintf(out, "%s, ", typename);

   if(ClauseIsEmpty(clause) || (is_untyped && problemType != PROBLEM_HO))
   {
      ClauseTSTPCorePrint(out, clause, fullterms);
   }
   else
   {
      // Print as universally quantified formula
      assert(clause->literals);
      form = TFormulaClauseEncode(clause->literals->bank, clause);
      form = TFormulaClosure(clause->literals->bank, form, true);

      TFormulaTPTPPrint(out, clause->literals->bank, form, fullterms, false);
      // TermPrintDbg(out, form, clause->literals->bank->sig, DEREF_NEVER);
      // handled by GC, no need to free
   }

   if(complete)
   {
      fprintf(out, ").");
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStartsMaybe()
//
//   Return true if a clause possibly starts on the current position
//   in the input, i.e. if TermStartToken, TildeSign, ?-, or <- are
//   present on the input stream.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseStartsMaybe(Scanner_p in)
{
   if(TestInpTok(in, TermStartToken|TildeSign))
   {
      return true;
   }
   if(TestInpTok(in, LesserSign|QuestionMark))
   {
      if(!LookToken(in,1)->skipped && TestTok(LookToken(in, 1), Hyphen))
      {
         return true;
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseTypeParse()
//
//   Parse a clause type specifier and return a matching type.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

FormulaProperties ClauseTypeParse(Scanner_p in, char *legal_types)
{
   FormulaProperties res;

   CheckInpId(in, legal_types);

   if(TestInpId(in,
                "axiom|definition|theorem"))
   {
      res = CPTypeAxiom;
      if(problemType == PROBLEM_HO && TestInpId(in, "definition"))
      {
         res = res | CPIsLambdaDef;
      }
   }
   else if(TestInpId(in, "question"))
   {
      res = CPTypeQuestion;
   }
   else if(TestInpId(in, "conjecture"))
   {
      res = CPTypeConjecture;
   }
   else if(TestInpId(in, "assumption|negated_conjecture"))
   {
      res = CPTypeNegConjecture;
   }
   else if(TestInpId(in, "hypothesis"))
   {
      res = CPTypeHypothesis;
   }
   else if(TestInpId(in, "lemma"))
   {
      res = CPTypeLemma;
   }
   else if(TestInpId(in, "watchlist"))
   {
      res = CPTypeWatchClause;
   }
   else
   {
      res = CPTypeUnknown;
   }
   AcceptInpTok(in, Ident);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseParse()
//
//   Parse a clause.
//
// Global Variables: ClausesHaveLocalVariables,
//                   ClausesHaveDisjointVariables
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseParse(Scanner_p in, TB_p bank)
{
   Eqn_p    concl, precond;
   bool     procedural = false;
   FormulaProperties type = CPTypeAxiom;
   FormulaProperties input = CPInputFormula;
   Clause_p handle;
   ClauseInfo_p info;

   if(ClausesHaveLocalVariables)
   {
      VarBankClearExtNames(bank->vars);
   }
   if(ClausesHaveDisjointVariables)
   {
      VarBankClearExtNamesNoReset(bank->vars);
   }
   info = ClauseInfoAlloc(NULL, DStrView(AktToken(in)->source),
                          AktToken(in)->line, AktToken(in)->column);
   if(ScannerGetFormat(in) == TPTPFormat)
   {
      AcceptInpId(in, "input_clause");
      AcceptInpTok(in, OpenBracket);
      info->name = DStrCopy(AktToken(in)->literal);
      AcceptInpTok(in, Name);
      AcceptInpTok(in, Comma);
      type = ClauseTypeParse(in, "axiom|hypothesis|conjecture|lemma|unknown|watchlist");
      if(type == CPTypeConjecture)
      {
         type = CPTypeNegConjecture; /* Old TPTP syntax lies ;-) */
      }
      AcceptInpTok(in, Comma);
      AcceptInpTok(in, OpenSquare);
      concl = EqnListParse(in, bank, Comma);
      AcceptInpTok(in, CloseSquare);
      AcceptInpTok(in, CloseBracket);
   }
   else if(ScannerGetFormat(in) == TSTPFormat)
   {
      AcceptInpId(in, "cnf");
      AcceptInpTok(in, OpenBracket);
      info->name = DStrCopy(AktToken(in)->literal);
      AcceptInpTok(in, Name|PosInt|SQString);
      AcceptInpTok(in, Comma);

      /* This is hairy! E's internal types do not map very well to
         TSTP types, and E uses the "initial" properties in ways that
         make it highly desirable that anything in the input is
         actually initial (the CPInitialProperty is actually set in
         all clauses in the initial unprocessed clause set. */
      type = ClauseTypeParse(in,
                             "axiom|definition|theorem|assumption|"
                             "hypothesis|negated_conjecture|"
                             "lemma|unknown|plain|watchlist");
      AcceptInpTok(in, Comma);
      if(TestInpTok(in, OpenBracket))
      {
         AcceptInpTok(in, OpenBracket);
         concl = EqnListParse(in, bank, Pipe);
         AcceptInpTok(in, CloseBracket);
      }
      else
      {
         concl = EqnListParse(in, bank, Pipe);
      }
      if(TestInpTok(in, Comma))
      {
         AcceptInpTok(in, Comma);
         TSTPSkipSource(in);
         if(TestInpTok(in, Comma))
         {
            AcceptInpTok(in, Comma);
            CheckInpTok(in, OpenSquare);
            ParseSkipParenthesizedExpr(in);
         }
      }
      AcceptInpTok(in, CloseBracket);
   }
   else
   {
      concl = EqnListParse(in, bank, Semicolon);
      //printf("Parsed EqnList\n");
      if(TestInpTok(in, Colon))
      {
         if(EqnListLength(concl)>1)
         {
            AktTokenError(in,
                          "Procedural rule cannot have more than one "
                          "head literal",
                          false);
         }
         procedural = true;
      }
      else if(TestInpTok(in, QuestionMark))
      {
         if(EqnListLength(concl)>0)
         {
            AktTokenError(in,
                          "Query should consist only of tail literals",
                          false);
         }
         type = CPTypeNegConjecture;
         /* printf("CPTypeConjecture\n"); */
      }
      if(TestInpTok(in, Fullstop))
      {
         if(EqnListLength(concl)>1)
         {
            AktTokenError(in,
                          "Procedural fact cannot have more than one "
                          "literal",
                          false);
         }
         procedural = true;
      }
      else
      {
         AcceptInpTok(in, LesserSign | Colon | QuestionMark);
         AcceptInpTokNoSkip(in, Hyphen);

         precond = EqnListParse(in, bank, Comma);

         if(procedural && EqnListLength(precond)==0)
         {
            AktTokenError(in,
                          "Procedural rule or query needs at least one "
                          "tail literal (Hey! I did not make this "
                          " syntax! -StS)",
                          false);
         }
         EqnListNegateEqns(precond);
         EqnListAppend(&concl, precond);
      }
   }
   AcceptInpTok(in, Fullstop);
   handle = ClauseAlloc(concl);
   ClauseSetTPTPType(handle, type);
   ClauseSetProp(handle, CPInitial|input);
   handle->info = info;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePCLParse()
//
//   Parse a clause in PCL format, i.e. as TPTP literal list.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClausePCLParse(Scanner_p in, TB_p bank)
{
   Clause_p handle;
   Eqn_p    list;

   assert(ScannerGetFormat(in) == TPTPFormat);
   assert(in);
   assert(bank);

   if(ClausesHaveLocalVariables)
   {
      VarBankClearExtNames(bank->vars);
   }
   AcceptInpTok(in, OpenSquare);
   list = EqnListParse(in, bank, Comma);
   AcceptInpTok(in, CloseSquare);
   handle=ClauseAlloc(list);
   ClauseSetTPTPType(handle,
                     handle->pos_lit_no?
                     CPTypeAxiom:CPTypeConjecture);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseMarkMaximalTerms()
//
//   Orient literals, mark maximal literals.
//
// Global Variables: -
//
// Side Effects    : Changes clause structure, orients equations
//
/----------------------------------------------------------------------*/

void ClauseMarkMaximalTerms(OCB_p ocb, Clause_p clause)
{
   EqnListOrient(ocb, clause->literals);
   // printf("Litno: %d\n", ClauseLiteralNumber(clause));
   EqnListMaximalLiterals(ocb, clause->literals);
   ClauseSetProp(clause, CPIsOriented);
}



/*-----------------------------------------------------------------------
//
// Function: ClauseParentsAreSubset()
//
//   Return true if parents of clause1 are a subset of clause2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

/* bool ClauseParentsAreSubset(Clause_p clause1, Clause_p clause2) */
/* { */
/*    bool sub1 = false, sub2 = false; */

/*    if(clause1->parent1) */
/*    { */
/*       if((clause1->parent1==clause2->parent1)|| */
/*          (clause1->parent1==clause2->parent2)) */
/*       { */
/*          sub1=true; */
/*       } */
/*    } */
/*    else */
/*    { */
/*       sub1=true; */
/*    } */
/*    if(clause1->parent2) */
/*    { */
/*       if((clause1->parent2==clause2->parent1)|| */
/*          (clause1->parent2==clause2->parent2)) */
/*       { */
/*          sub2=true; */
/*       } */
/*    } */
/*    else */
/*    { */
/*       sub2=true; */
/*    } */
/*    return sub1&&sub2; */
/* } */


/*-----------------------------------------------------------------------
//
// Function: ClauseAddEvalCell()
//
//   Add an evaluation cell (as the first evaluation) to a clause.
//
// Global Variables: -
//
// Side Effects    : Changes eval list, makes evaluation->object point
//                   to clause.
//
/----------------------------------------------------------------------*/
void ClauseAddEvalCell(Clause_p clause, Eval_p evaluation)
{
   evaluation->object  = clause;
   clause->evaluations = evaluation;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveEvaluations()
//
//   Remove the evaluations from a clause and free the EvalCells.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseRemoveEvaluations(Clause_p clause)
{
   EvalsFree(clause->evaluations);
   clause->evaluations = NULL;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseWeight()
//
//   Compute the weight of a clause by counting function symbols and
//   variables and applying various modifiers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseWeight(Clause_p clause, double max_term_multiplier,
                    double max_literal_multiplier, double
                    pos_multiplier, long vweight, long fweight,
                    double app_var_mult, bool count_eq_encoding)
{
   Eqn_p  handle;
   double res = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += LiteralWeight(handle, max_term_multiplier,
                           max_literal_multiplier, pos_multiplier,
                           vweight, fweight, app_var_mult, count_eq_encoding);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFunWeight()
//
//   Compute the weight of a clause by summing weights for individual
//   function symbols and variables and applying various modifiers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseFunWeight(Clause_p clause, double max_term_multiplier,
                       double max_literal_multiplier, double
                       pos_multiplier, long vweight, long flimit,
                       long *fweights, long default_fweight, double app_var_mult,
                       long *typefreqs)
{
   Eqn_p  handle;
   double res = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += LiteralFunWeight(handle, max_term_multiplier,
                              max_literal_multiplier, pos_multiplier,
                              vweight, flimit, fweights,
                              default_fweight, app_var_mult,
                              typefreqs);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseTermExtWeight()
//
//   Compute the weight of a clause as an extension of an arbitrary term
//   weight function. Modifiers are applied, several extensions are
//   supported (standard - sum literal/term weights, subterms - sum
//   weights of all subterms, or take the maximum subterm weight).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseTermExtWeight(Clause_p clause, TermWeightExtension_p twe)
{
   Eqn_p  lit;
   double res = 0;

   for (lit = clause->literals; lit; lit = lit->next)
   {
      res += LiteralTermExtWeight(lit, twe);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: ClauseNonLinearWeight()
//
//   Compute the weight of a clause by counting function symbols and
//   variables and applying various modifiers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseNonLinearWeight(Clause_p clause, double max_term_multiplier,
                             double max_literal_multiplier, double
                             pos_multiplier, long vlweight, long
                             vweight, long fweight, double app_var_mult, bool
                             count_eq_encoding)
{
   Eqn_p  handle;
   double res = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += LiteralNonLinearWeight(handle, max_term_multiplier,
                                    max_literal_multiplier, pos_multiplier,
                                    vlweight, vweight, fweight, app_var_mult,
                                    count_eq_encoding);

   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSymTypeWeight()
//
//   Compute the weight of a clause by counting function, predicate
//   and constant symbols, and variables, and apply various
//   modifiers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseSymTypeWeight(Clause_p clause, double
                           max_term_multiplier, double
                           max_literal_multiplier, double
                           pos_multiplier, long vweight, long
                           fweight, long cweight, long pweight,
                           double app_var_mult)
{
   Eqn_p  handle;
   double res = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += LiteralSymTypeWeight(handle, max_term_multiplier,
                                  max_literal_multiplier, pos_multiplier,
                                  vweight, fweight, cweight, pweight,
                                  app_var_mult);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStandardWeight()
//
//   Compute the standard weight of a clause (Vars = 1, Funs = 2,
//   everything counts equally.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double ClauseStandardWeight(Clause_p clause)
{
   Eqn_p  handle;
   double res = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += EqnStandardWeight(handle);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseOrientWeight()
//
//   Compute the weight of a clause by counting function symbols and
//   variables and applying various other modifiers ;-).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseOrientWeight(Clause_p clause, double
                          unorientable_literal_multiplier,
                          double max_literal_multiplier, double
                          pos_multiplier, long vweight, long fweight,
                          double app_var_mult, bool
                          count_eq_encoding)
{
   Eqn_p  handle;
   double res = 0 ,tmp;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = LiteralWeight(handle, 1, max_literal_multiplier,
                          pos_multiplier, vweight, fweight,
                          app_var_mult,
                          count_eq_encoding);
      if(!EqnIsOriented(handle))
      {
         tmp = tmp*unorientable_literal_multiplier;
      }
      res += tmp;
   }
   return res;


}


/*-----------------------------------------------------------------------
//
// Function: ClauseNotGreaterEqual
//
//   Checks whether the multiset of literals of a clause is neither
//   greater than (wrt. the multiset extension of a specified
//   ordering) nor equal to the multiset of literals of another
//   clause.
//
//   not(A >=>= B)  iff  - not(A = B) and
//                       - for each b in B\A there does not exist an a
//                         in A\B such that a > b
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseNotGreaterEqual(OCB_p ocb,
                           Clause_p clause1, Clause_p clause2)
{
   Eqn_p cl1lits = clause1->literals;
   Eqn_p cl2lits = clause2->literals,cl2litseqs;
   bool foundeq, foundeqlater, foundgtr;

   EqnListDelProp(cl1lits, EPHasEquiv | EPDominates);

   while (cl2lits)
   {
      EqnDelProp(cl2lits, EPHasEquiv | EPIsDominated);
      foundeq = foundgtr = false;

      while (cl1lits && !EqnHasEquiv(cl2lits))
      {
         if (!EqnHasEquiv(cl1lits))
         {
            switch (LiteralCompare(ocb, cl1lits, cl2lits))
            {
            case to_greater:
                  foundeqlater = false;
                  if (!EqnDominates(cl1lits))
                  {
                     cl2litseqs = cl2lits->next;
                     foundeqlater =
                        foundEqLitLater(ocb, cl1lits, cl2litseqs);
                  }
                  if (!foundeqlater)
                  {
                     EqnSetProp(cl1lits, EPDominates);
                     EqnSetProp(cl2lits, EPIsDominated);
                     foundgtr = true;
                  }
                  break;
            case to_equal:
                  EqnSetProp(cl1lits, EPHasEquiv);
                  EqnSetProp(cl2lits, EPHasEquiv);
                  foundeq = true;
                  break;
            case to_lesser:
            case to_uncomparable:
                  break;
            default:
                  assert(false);
                  break;
            }
         }
         cl1lits = cl1lits->next;
      }

      /* early abortion if there is neither a greater nor an equal
         element in clause1 */
      if ((!foundeq) && (!foundgtr))
      {
         /* printf("True\n"); */
         return true;
      }
      cl2lits = cl2lits->next;
   }
   /* printf("False\n"); */
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCompareFun()
//
//   Compare two clauses, induce a total ordering on all clauses. If
//   ClauseCompareFun(clause1, clause2) == 0, then clause1==clause2
//   modulo symmetry of = and AC of the disjunction (In the current
//   implementation, only symmetry is taken into account.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCompareFun(const void *c1, const void* c2)
{
   const Clause_p clause1 = (const Clause_p) c1;
   const Clause_p clause2 = (const Clause_p) c2;
   int res;
   Eqn_p handle1, handle2;

   if((res = clause2->pos_lit_no - clause1->pos_lit_no))
   {
      return res;
   }
   if((res = clause2->neg_lit_no - clause1->neg_lit_no))
   {
      return res;
   }
   handle1 = clause1->literals;
   handle2 = clause2->literals;
   while(handle1)
   {
      assert(handle2);
      res = LiteralCompareFun(handle1, handle2);
      if(res)
      {
         break;
      }
      handle1 = handle1->next;
      handle2 = handle2->next;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCmpById()
//
//   Compare two clauses by identifier.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCmpById(const void* clause1, const void* clause2)
{
   const Clause_p *c1 = (const Clause_p*) clause1;
   const Clause_p *c2 = (const Clause_p*) clause2;

   if((*c1)->ident < (*c2)->ident)
   {
      return -1;
   }
   if((*c1)->ident > (*c2)->ident)
   {
      return 1;
   }
   return 0;
}


#ifdef CLAUSE_PERM_IDENT
/*-----------------------------------------------------------------------
//
// Function: ClauseCmpByPermId()
//
//   Compare two clauses by permanent identifier.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCmpByPermId(const void* clause1, const void* clause2)
{
   const Clause_p *c1 = (const Clause_p*) clause1;
   const Clause_p *c2 = (const Clause_p*) clause2;

   if((*c1)->perm_ident < (*c2)->perm_ident)
   {
      return -1;
   }
   if((*c1)->perm_ident > (*c2)->perm_ident)
   {
      return 1;
   }
   return 0;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseCmpByPermIdR()
//
//   Compare two clauses by reverse permanent identifier.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCmpByPermIdR(const void* clause1, const void* clause2)
{
   const Clause_p *c1 = (const Clause_p*) clause1;
   const Clause_p *c2 = (const Clause_p*) clause2;

   if((*c1)->perm_ident < (*c2)->perm_ident)
   {
      return 1;
   }
   if((*c1)->perm_ident > (*c2)->perm_ident)
   {
      return -1;
   }
   return 0;
}
#endif




/*-----------------------------------------------------------------------
//
// Function: ClauseCmpByStructWeight()
//
//   Compare by a total syntactical order.
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
int ClauseCmpByStructWeight(const void* clause1, const void* clause2)
{
   const Clause_p *c1 = (const Clause_p*) clause1;
   const Clause_p *c2 = (const Clause_p*) clause2;

   return CMP(ClauseStructWeightLexCompare(*c1, *c2), 0);
}

/*-----------------------------------------------------------------------
//
// Function: ClauseCmpByPtr()
//
//   Compare two clauses by address. This is rarely useful outside
//   debugging!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCmpByPtr(const void* clause1, const void* clause2)
{
   const Clause_p *c1 = (const Clause_p*) clause1;
   const Clause_p *c2 = (const Clause_p*) clause2;

   return PCmp(*c1, *c2);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseNormalizeVars()
//
//   Destructively normalize variables in clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseNormalizeVars(Clause_p clause, VarBank_p fresh_vars)
{
   Eqn_p tmplist;
   Subst_p subst;

   assert(!ClauseQueryProp(clause,CPIsDIndexed));

   if(!ClauseIsEmpty(clause))
   {
      subst = SubstAlloc();
      VarBankResetVCounts(fresh_vars);

      NormSubstClause(clause, subst, fresh_vars);

      if(!SubstIsEmpty(subst))
      {
         tmplist = EqnListCopy(clause->literals, clause->literals->bank);
         EqnListFree(clause->literals);
         clause->literals = tmplist;
      }
      SubstDelete(subst);
   }
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCollectSubterms()
//
//   Collect all subterms of clause onto collector. Assumes that
//   TPOpFlag is unset in all subterms. Returns the number of new
//   terms found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCollectSubterms(Clause_p clause, PStack_p collector)
{
   PStackPointer i, start;
   long res;
   Term_p t;

   start = PStackGetSP(collector);
   res = EqnListCollectSubterms(clause->literals, collector);

   for(i=start; i<PStackGetSP(collector); i++)
   {
      t = PStackElementP(collector, i);
      TermCellDelProp(t, TPOpFlag);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseReturnFCodes()
//
//   Push all function symbol codes from clause onto f_codes. Return
//   number of symbols found.
//
// Global Variables: -
//
// Side Effects    : Uses and resets FPOpFlag in the signature
//
/----------------------------------------------------------------------*/

long ClauseReturnFCodes(Clause_p clause, PStack_p f_codes)
{
   long res = 0;
   PStack_p stack;
   Sig_p sig;
   PStackPointer i, start;
   Term_p t;
   FunCode f;

   assert(clause);
   assert(f_codes);

   if(ClauseIsEmpty(clause))
   {
      return res;
   }
   assert(clause->literals);
   sig = clause->literals->bank->sig;
   assert(sig);

   stack = PStackAlloc();
   ClauseCollectSubterms(clause, stack);

   start = PStackGetSP(f_codes);
   for(i=0; i<PStackGetSP(stack);i++)
   {
      t = PStackElementP(stack,i);
      if(!TermIsAnyVar(t))
      {
         if(!SigQueryFuncProp(sig, t->f_code, FPOpFlag))
         {
            SigSetFuncProp(sig, t->f_code, FPOpFlag);
            PStackPushInt(f_codes, t->f_code);
            res++;
         }
      }
   }
   PStackFree(stack);

   /* Reset FPOpFlags */
   for(i=start; i<PStackGetSP(f_codes);i++)
   {
      f =  PStackElementInt(f_codes, i);
      SigDelFuncProp(sig, f, FPOpFlag);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseIsUntyped
//
//   return true iff the clause belongs to the FOF/CNF fragment, ie
//   all its literals are untyped
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/
bool ClauseIsUntyped(Clause_p clause)
{
   Eqn_p lits = clause->literals;

   while (lits)
   {
      if (!EqnIsUntyped(lits))
      {
         return false;
      }
      lits = lits->next;
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseQueryLiteral()
//
//   Return true if there is a literal that satisfies query_fun
//   predicate
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseQueryLiteral(Clause_p clause, bool (*query_fun)(Eqn_p))
{
   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      if(query_fun(lit))
      {
         return true;
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseRecognizeChoice()
//
//   If the clause is of the form ~P X | P (f P) it will recognize
//   that f is a defined choice operatior, store f in choice_symbols
//   map and return true.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define FAIL_ON(cond) if (cond) return false

bool ClauseRecognizeChoice(IntMap_p choice_symbols_map, Clause_p cl)
{
   FAIL_ON(cl->pos_lit_no != 1);
   FAIL_ON(cl->neg_lit_no != 1);

   Eqn_p pos_lit =
      EqnIsPositive(cl->literals) ? cl->literals : cl->literals->next;
   Eqn_p neg_lit =
      EqnIsNegative(cl->literals) ? cl->literals : cl->literals->next;
   assert(EqnIsPositive(pos_lit));
   assert(EqnIsNegative(neg_lit));

   FAIL_ON(EqnIsEquLit(pos_lit));
   FAIL_ON(EqnIsEquLit(neg_lit));

   TB_p bank = pos_lit->bank;
   Term_p neg_term =
      BetaNormalizeDB(bank, LambdaEtaReduceDB(bank, neg_lit->lterm));
   Term_p pos_term =
      BetaNormalizeDB(bank, LambdaEtaReduceDB(bank, pos_lit->lterm));

   FAIL_ON(!TermIsAppliedFreeVar(neg_term));
   FAIL_ON(!TermIsAppliedFreeVar(pos_term));

   FAIL_ON(neg_term->arity != 2);
   FAIL_ON(!TermIsFreeVar(neg_term->args[1]));

   Term_p p_var = neg_term->args[0];
   FAIL_ON(pos_term->arity != 2);
   FAIL_ON(pos_term->args[0] != p_var);

   Term_p fp = pos_term->args[1];
   FAIL_ON(fp->arity != 1);
   FAIL_ON(fp->f_code <= bank->sig->internal_symbols);
   FAIL_ON(fp->args[0] != p_var);
   if(choice_symbols_map)
   {
      FAIL_ON(IntMapGetVal(choice_symbols_map, fp->f_code));
   }

   if(choice_symbols_map)
   {
      neg_lit->lterm = neg_term;
      pos_lit->lterm = pos_term;
      IntMapAssign(choice_symbols_map, fp->f_code, cl);
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCollectGroundTerms()
//
//   Add no-constant ground subterms of the terms in certain literals
//   (positive and/or negative, as per the selection parameters) in
//   the clause to result. If top_only is set, only add maximal (in
//   the subterm relation sense) terms, otherwise add all non-constant
//   ground terms. Returns number of terms newly added.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long ClauseCollectGroundTerms(Clause_p clause, PTree_p *res, bool all_subterms,
                              bool pos_lits, bool neg_lits)
{

   return EqnListCollectGroundTerms(clause->literals, res, all_subterms,
                                    pos_lits, neg_lits);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
