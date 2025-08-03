/*-----------------------------------------------------------------------

  File  : ccl_paramod.c

  Author: Stephan Schulz

  Contents

  Paramodulation and stuff...

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Mon May 25 23:53:05 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_paramod.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/* If true, do overlap non-equational literals as well - default for
   proof mode, but (probably) unecessary for SETHEO preprocessing in
   METOP */

bool ParamodOverlapNonEqLiterals = true;
bool ParamodOverlapIntoNegativeLiterals = true;

char* pm_names[] =
{
   "ParamodPlain",
   "ParamodSim",
   "ParamodOrientedSim",
   "ParamodSuperSim",
   "ParamodOrientedSuperSim",
   "ParamodDecreasingSim",
   "ParamodSizeDecreasingSim",
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
// Function: check_paramod_ordering_constraint()
//
//   Given two clause positions and an OCB, return true if the clause
//   resulting from the described paramod-inference shall be
//   kept for further processing. Formally, if sigma(from->clause) >
//   sigma(into->clause), the paramodulant can be discarded. However,
//   this check ist pretty expensive, and does not always improve
//   performance. This function discards some of the unnecessary
//   clauses, trying to capture only the bad cases and to minimize
//   real ordering comparisons.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#ifdef NEVER_DEFINED

#define PARAMOD_FROM_LENGTH_LIMIT 2 /* This should come from
                   ProofControl as soon as
                   implemented */

static bool check_paramod_ordering_constraint(OCB_p ocb, ClausePos_p
                     from_pos, ClausePos_p
                     into_pos)
{
   bool res = false;
   int from_length = ClauseLiteralNumber(from_pos->clause);

   /* Accept inferences with short clauses */

   if(from_length>=PARAMOD_FROM_LENGTH_LIMIT)
   {
      res = true;
   }
   /* In some cases we _know_ that from_pos is smaller */
   else if(from_length == 1 && !TermPosIsTopPos(into_pos->pos)&&
      EqnIsOriented(from_pos->literal))
   {
      res = true;
   }
   else
   {
      res = ClauseNotGreaterEqual(ocb, from_pos->clause,
                into_pos->clause);
   }
   return res;
}

#endif

/*-----------------------------------------------------------------------
//
// Function: clause_pos_find_first_neg_max_lside()
//
//   Find the first maximal negative side in the list at pos->literal.
//
// Global Variables: -
//
// Side Effects    : Changes pos->literal, pos->side
//
/----------------------------------------------------------------------*/

static Term_p clause_pos_find_first_neg_max_lside(ClausePos_p pos)
{
   Eqn_p handle;
   Term_p res = NULL;

   for(handle = pos->literal; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle) &&
    EqnIsNegative(handle))
      {
         break;
      }
   }
   pos->literal = handle;
   if(handle)
   {
      pos->side = LeftSide;
      PStackReset(pos->pos);
      res = ClausePosGetSide(pos);
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ParamodStr()
//
//   Return a string representing the paramodulation type.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

char* ParamodStr(ParamodulationType pm_type)
{
   assert(pm_type <= ParamodSizeDecreasingSim);
   return pm_names[pm_type];
}


/*-----------------------------------------------------------------------
//
// Function: ParamodType()
//
//    Given a string encoding, return paramodulation type (or -1 if
//    none).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ParamodulationType ParamodType(char *pm_str)
{
   return StringIndex(pm_str, pm_names);
}



/*-----------------------------------------------------------------------
//
// Function: ParamodInfoPrint()
//
//   Print a paramodulation descriptor (for debugging).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ParamodInfoPrint(FILE* out, ParamodInfo_p info)
{
   fprintf(out, COMCHAR" From: %6ld |%6ld\n"COMCHAR" ", info->from->ident, info->from_cpos);
   ClausePrint(out, info->from, true);
   fprintf(out, "\n"COMCHAR"Into: %6ld |%6ld\n"COMCHAR"", info->into->ident, info->into_cpos);
   ClausePrint(out, info->into, true);
   fprintf(out, "\n"COMCHAR"Orig: %6ld\n"COMCHAR, info->new_orig->ident);
   ClausePrint(out, info->new_orig, true);
   fprintf(out, "\n");
}


/*-----------------------------------------------------------------------
//
// Function: ClausePlainParamodConstruct()
//
//   Construct a clause via plain paramodulation according to
//   the data in ol_desc. Return the clause, unless it's trivial
//   tautological (then return NULL).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Clause_p ClausePlainParamodConstruct(ParamodInfo_p ol_desc)
{
   Clause_p  res=NULL;
   Term_p    from_rhs, into_rhs, new_rhs, new_lhs;
   Eqn_p     into_copy, from_copy, pm_lit;
   Subst_p   subst = SubstAlloc();


   assert(TermStructPrefixEqual(ClausePosGetSubterm(ol_desc->from_pos),
                               ClausePosGetSubterm(ol_desc->into_pos),
                               DEREF_ALWAYS,
                               DEREF_ALWAYS,
                               0, ol_desc->bank->sig));
   assert(EqnIsPositive(ol_desc->from_pos->literal));
   assert(PStackEmpty(ol_desc->from_pos->pos));


   VarBankResetVCounts(ol_desc->freshvars);
   NormSubstEqnList(ol_desc->into->literals,
                    subst, ol_desc->freshvars);
   NormSubstEqnList(ol_desc->from->literals,
                    subst, ol_desc->freshvars);

   from_rhs = ClausePosGetOtherSide(ol_desc->from_pos);
   into_rhs = ClausePosGetOtherSide(ol_desc->into_pos);
   new_lhs = TBTermPosReplace(ol_desc->bank, from_rhs,
                              ol_desc->into_pos->pos,
                              DEREF_ALWAYS, 0,
                              ClausePosGetSubterm(ol_desc->into_pos));

   new_rhs = TBInsertOpt(ol_desc->bank,
                         into_rhs,
                         DEREF_ALWAYS);

   if(!EqnIsPositive(ol_desc->into_pos->literal)||(new_lhs != new_rhs))
   {
      into_copy = EqnListCopyOptExcept(ol_desc->into->literals,
                                       ol_desc->into_pos->literal);
      if(EqnListFindTrue(into_copy))
      {
         EqnListFree(into_copy);
      }
      else
      {
         from_copy = EqnListCopyOptExcept(ol_desc->from->literals,
                                          ol_desc->from_pos->literal);

         if(EqnListFindTrue(from_copy))
         {
            EqnListFree(into_copy);
            EqnListFree(from_copy);
         }
         else
         {
            into_copy = EqnListAppend(&into_copy, from_copy);

            pm_lit = EqnAlloc(new_lhs, new_rhs, ol_desc->bank,
                              EqnIsPositive(ol_desc->into_pos->literal));
            pm_lit =  EqnListAppend(&pm_lit, into_copy);
            EqnListLambdaNormalize(pm_lit);
            EqnListRemoveResolved(&pm_lit);
            EqnListRemoveDuplicates(pm_lit);
            res = ClauseAlloc(pm_lit);
         }
      }
   }
   SubstDelete(subst);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSimParamodConstruct()
//
//   Construct a clause via simultaneous paramodulation according to
//   the data in ol_desc. Return the clause, unless it's trivial
//   tautological (then return NULL).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSimParamodConstruct(ParamodInfo_p ol_desc)
{
   Clause_p  res=NULL;
   Term_p    rhs_instance, into_term;
   Eqn_p     into_copy, from_copy;
   Subst_p   subst = SubstAlloc();

   VarBankResetVCounts(ol_desc->freshvars);
   into_term = ClausePosGetSubterm(ol_desc->into_pos);

   /* All the checks are assumed to have been done and succeeded, we
      just build the clause.. */

   NormSubstEqnListExcept(ol_desc->into->literals, NULL,
                          subst, ol_desc->freshvars);
   NormSubstEqnListExcept(ol_desc->from->literals, NULL,
                          subst, ol_desc->freshvars);
   assert(ClausePosGetSide(ol_desc->from_pos)->type
            == ClausePosGetOtherSide(ol_desc->from_pos)->type);

   Term_p tmp_rhs =
      MakeRewrittenTerm(TermDerefAlways(into_term),
                        TermDerefAlways(ClausePosGetOtherSide(ol_desc->from_pos)),
                        0,
                        ol_desc->bank);

   rhs_instance = TBInsertNoProps(ol_desc->bank, tmp_rhs, DEREF_ALWAYS);

   into_copy = EqnListCopyRepl(ol_desc->into->literals,
                               ol_desc->bank, into_term, rhs_instance);
   if(EqnListFindTrue(into_copy))
   {
      EqnListFree(into_copy);
   }
   else
   {
      from_copy = EqnListCopyOptExcept(ol_desc->from->literals,
                                       ol_desc->from_pos->literal);
      if(EqnListFindTrue(from_copy))
      {
         EqnListFree(into_copy);
         EqnListFree(from_copy);
      }
      else
      {
         EqnListDelProp(into_copy, EPFromClauseLit);
         EqnListSetProp(from_copy, EPFromClauseLit);

         into_copy = EqnListAppend(&into_copy, from_copy);

         EqnListLambdaNormalize(into_copy);
         EqnListRemoveResolved(&into_copy);
         EqnListRemoveDuplicates(into_copy);
         res = ClauseAlloc(into_copy);
      }
   }
   SubstDelete(subst);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseSuperSimParamodConstruct()
//
//   Construct a clause via simultaneous paramodulation according to
//   the data in ol_desc. Return the clause, unless it's trivial
//   tautological (then return NULL).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSuperSimParamodConstruct(ParamodInfo_p ol_desc)
{
   Clause_p  res=NULL;
   Term_p    rhs_instance, into_term;
   Eqn_p     into_copy, from_copy, tmp_copy;
   Subst_p   subst = SubstAlloc();

   assert(TermStructPrefixEqual(ClausePosGetSubterm(ol_desc->from_pos),
                                ClausePosGetSubterm(ol_desc->into_pos),
                                DEREF_ALWAYS,
                                DEREF_ALWAYS,
                                0, ol_desc->bank->sig));

   VarBankResetVCounts(ol_desc->freshvars);
   into_term = ClausePosGetSubterm(ol_desc->into_pos);

   /* All the checks are assumed to have been done and succeeded, we
      just build the clause.. */

   NormSubstEqnListExcept(ol_desc->into->literals, NULL,
                          subst, ol_desc->freshvars);
   NormSubstEqnListExcept(ol_desc->from->literals, NULL,
                          subst, ol_desc->freshvars);
   assert(ClausePosGetSide(ol_desc->from_pos)->type
          == ClausePosGetOtherSide(ol_desc->from_pos)->type);


   Term_p tmp_lhs = TBInsert(ol_desc->bank, into_term, DEREF_ALWAYS);
   Term_p tmp_rhs =
      MakeRewrittenTerm(TermDerefAlways(into_term),
                        TermDerefAlways(ClausePosGetOtherSide(ol_desc->from_pos)),
                        0,
                        ol_desc->bank);

   rhs_instance = TBInsertNoProps(ol_desc->bank, tmp_rhs, DEREF_ALWAYS);

   tmp_copy = EqnListCopyOpt(ol_desc->into->literals);

   into_copy = EqnListCopyReplPlain(tmp_copy,
                                    ol_desc->bank, tmp_lhs, rhs_instance);
   EqnListFree(tmp_copy);

   if(EqnListFindTrue(into_copy))
   {
      EqnListFree(into_copy);
   }
   else
   {
      from_copy = EqnListCopyOptExcept(ol_desc->from->literals,
                                       ol_desc->from_pos->literal);
      if(EqnListFindTrue(from_copy))
      {
         EqnListFree(into_copy);
         EqnListFree(from_copy);
      }
      else
      {
         EqnListDelProp(into_copy, EPFromClauseLit);
         EqnListSetProp(from_copy, EPFromClauseLit);

         into_copy = EqnListAppend(&into_copy, from_copy);

         EqnListLambdaNormalize(into_copy);
         EqnListRemoveResolved(&into_copy);
         EqnListRemoveDuplicates(into_copy);
         res = ClauseAlloc(into_copy);
      }
   }
   SubstDelete(subst);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseParamodConstruct()
//
//   Construct the clause from the overlap described (and checked!) in
//   ol_desc, either by paramodulation or simulataneous
//   paramodulation. Return the clause.
//
//   This has the implicit precondition that all variables involved
//   are already instantiated with the mgu of ol_desc->from|from_cpos
//   and ol_desc->into|into_pos.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

Clause_p ClauseParamodConstruct(ParamodInfo_p ol_desc,
                                ParamodulationType pm_type)
{
   Clause_p res = NULL;
   /* ParamodInfoPrint(stdout, ol_desc); */

   assert(PackClausePos(ol_desc->from_pos) == ol_desc->from_cpos);
   assert(PackClausePos(ol_desc->into_pos) == ol_desc->into_cpos);

   switch(pm_type)
   {
   case ParamodSim:
         res = ClauseSimParamodConstruct(ol_desc);
         break;
   case ParamodSuperSim:
         res = ClauseSuperSimParamodConstruct(ol_desc);
         break;
   case ParamodPlain:
         res = ClausePlainParamodConstruct(ol_desc);
         break;
   default:
         assert(false && "Unexpected paramodulation type");
         break;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ComputeOverlap()
//
//   Given an equation and a term position, overlap the designated
//   side of the equation into the subterm, i.e. given s[t], u=v,
//   return sigma(s[v]) if sigma = mgu(t,u) and sigma(u) !< sigma(v).
//
//   If the operation is successful, subst will contain the mgu, and
//   the pointer to the new term, inserted into bank, will be
//   returned. Otherwise, subst will be unchanged and NULL will be
//   returned.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes substitution, may
                     change term bank.
//
/----------------------------------------------------------------------*/

Term_p ComputeOverlap(TB_p bank, OCB_p ocb, ClausePos_p from, Term_p
            into, TermPos_p pos,  Subst_p subst, VarBank_p
            freshvars)
{
   Term_p        new_rside = NULL, sub_into, max_side, rep_side;
   PStackPointer oldstate;
   UnificationResult  unify_res;

   assert(from->side == LeftSide || !EqnIsOriented(from->literal));
   assert(EqnIsPositive(from->literal));
   assert(TermPosIsTopPos(from->pos));

   sub_into = TermPosGetSubterm(into, pos);

   assert(!TermIsFreeVar(sub_into));

   max_side = ClausePosGetSide(from);
   rep_side = ClausePosGetOtherSide(from);

   oldstate = PStackGetSP(subst);

   unify_res = SubstMguComplete(max_side, sub_into, subst);

   /* If unification succeeded and potentially prefix of into term has been unified */
   if(!UnifFailed(unify_res)
         && CheckHOUnificationConstraints(unify_res, RightTerm, max_side, sub_into))
   {
      if(!EqnIsOriented(from->literal)
          && TOGreater(ocb, rep_side, max_side, DEREF_ALWAYS,
                  DEREF_ALWAYS))
      {
         SubstBacktrackToPos(subst, oldstate);
      }
      else
      {
         /* We need to get consistent variables _before_ inserting the
            newly generated term into the term bank ! */
         SubstNormTerm(into, subst, freshvars, bank->sig);
         SubstNormTerm(rep_side, subst, freshvars, bank->sig);
         new_rside = TBTermPosReplace(bank, rep_side, pos,
                       DEREF_ALWAYS, 0, sub_into);
      }
   }
   return new_rside;
}


/*-----------------------------------------------------------------------
//
// Function: EqnOrderedParamod()
//
//   Overlap the equation described by into from the one described by
//   into and compute the critical pair, if one exists. Return a
//   pointer to a critical pair, if it exists, NULL othewise. If a cp
//   exists, subst will contain the substitution.
//
// Global Variables: -
//
// Side Effects    : As for ComputeOverlap()
//
/----------------------------------------------------------------------*/

Eqn_p  EqnOrderedParamod(TB_p bank, OCB_p ocb, ClausePos_p from,
          ClausePos_p into, Subst_p subst, VarBank_p
          freshvars)
{
   Term_p        new_lside, new_rside, lside, rside;
   PStackPointer oldstate;
   Eqn_p         new_cp = NULL;

   assert(from->side == LeftSide || !EqnIsOriented(from->literal));
   assert(EqnIsPositive(from->literal));
   assert(TermPosIsTopPos(from->pos));
   assert(into->side == LeftSide || !EqnIsOriented(into->literal));

   lside = ClausePosGetSide(into);
   rside = ClausePosGetOtherSide(into);
   oldstate = PStackGetSP(subst);

   new_lside = ComputeOverlap(bank, ocb, from, lside, into->pos,
               subst, freshvars);
   if(new_lside)
   {
      if((!EqnIsOriented(into->literal))
    && TOGreater(ocb, rside, lside, DEREF_ALWAYS, DEREF_ALWAYS))
      {
    SubstBacktrackToPos(subst, oldstate);
      }
      else
      {
    SubstNormTerm(rside, subst, freshvars, bank->sig);
    new_rside = TBInsert(bank, rside, DEREF_ALWAYS);

    /* Optimize away trivial paramods... */
    if(!EqnIsPositive(into->literal) || (new_lside != new_rside))
    {
       new_cp = EqnAlloc(new_lside, new_rside, bank,
               EqnIsPositive(into->literal));
       EqnSetProp(new_cp, EPIsPMIntoLit);
    }
      }
   }
   return new_cp;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseOrderedParamod()
//
//   Given two clauses, try to perform an ordered paramodulation
//   step. Return the clause if it works, NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : May create new clause and change bank
//
/----------------------------------------------------------------------*/

Clause_p ClauseOrderedParamod(TB_p bank, OCB_p ocb, ClausePos_p from,
               ClausePos_p into, VarBank_p freshvars)
{
   Clause_p  new_clause = NULL;
   Eqn_p     new_literals, into_copy, from_copy;
   Subst_p   subst;

   assert(EqnIsMaximal(from->literal));
   assert(!EqnIsOriented(from->literal)||(from->side==LeftSide));
   assert(!TermIsFreeVar(ClausePosGetSide(from))||
     EqnIsEquLit(into->literal)||!TermPosIsTopPos(into->pos));

   /*{
      ClausePos_p tmp1, tmp2;
      CompactPos  ctmp1, ctmp2;

      printf("\nO:");
      ClausePosPrint(stdout, from);
      printf("(%ld)->(%ld)", PackClausePos(from),PackClausePos(into));
      ClausePosPrint(stdout, into);
      printf("\n");

      ctmp1 = PackClausePos(from);
      ctmp2 = PackClausePos(into);
      tmp1 = UnpackClausePos(ctmp1, from->clause);
      tmp2 = UnpackClausePos(ctmp2, into->clause);

      printf("T:");
      ClausePosPrint(stdout, tmp1);
      printf("(%ld)->(%ld)", ctmp1, ctmp2);
      ClausePosPrint(stdout, tmp2);
      printf("\n");

      ClausePosFree(tmp1);
      ClausePosFree(tmp2);
      }*/

   subst = SubstAlloc();
   VarBankResetVCounts(freshvars);
   new_literals = EqnOrderedParamod(bank, ocb, from, into, subst,
                freshvars);
   if(new_literals)
   {
      if(((EqnIsPositive(into->literal)&&
      EqnListEqnIsStrictlyMaximal(ocb,
                   into->clause->literals,
                   into->literal))
     ||
     (EqnIsNegative(into->literal)/* &&
      EqnListEqnIsMaximal(ocb,
           into->clause->literals,
           into->literal)*/))
    &&
    EqnListEqnIsStrictlyMaximal(ocb,
                 from->clause->literals,
                 from->literal)
    /* &&
            check_paramod_ordering_constraint(ocb, from, into)*/)
      {
    NormSubstEqnListExcept(into->clause->literals, into->literal,
            subst, freshvars);
    NormSubstEqnListExcept(from->clause->literals, from->literal,
            subst, freshvars);
    new_literals->next = NULL;

    into_copy = EqnListCopyOptExcept(into->clause->literals,
                   into->literal);
    from_copy = EqnListCopyOptExcept(from->clause->literals,
                   from->literal);

    EqnListDelProp(into_copy, EPFromClauseLit);
    EqnListSetProp(from_copy, EPFromClauseLit);
         EqnSetProp(new_literals, EPFromClauseLit);

    into_copy = EqnListAppend(&into_copy, from_copy);
    EqnListDelProp(into_copy, EPIsPMIntoLit);
    new_literals =  EqnListAppend(&new_literals, into_copy);

    EqnListRemoveResolved(&new_literals);
    EqnListRemoveDuplicates(new_literals);
    new_clause = ClauseAlloc(new_literals);
      }
      else
      {
    EqnFree(new_literals);
      }
   }
   SubstDelete(subst);

   return new_clause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseOrderedSimParamod()
//
//   Perform a simultaneous ordered simultaneous paramod step (if
//   necessary).
//
// Global Variables: - (but reads term property TPPotentialParamod)
//
// Side Effects    : Memory operations, changes fresh variable counter
//
/----------------------------------------------------------------------*/

Clause_p ClauseOrderedSimParamod(TB_p bank, OCB_p ocb, ClausePos_p
                                 from, ClausePos_p into, VarBank_p
                                 freshvars)
{
   Clause_p  new_clause = NULL;
   Term_p    rhs_instance, from_term, into_term;
   Eqn_p     into_copy, from_copy;
   Subst_p   subst;
   UnificationResult unify_res;

   assert(EqnIsMaximal(from->literal));
   assert(!EqnIsOriented(from->literal)||(from->side==LeftSide));
   // In HO case variables might paramodulate into predicate positions
   assert(!TermIsFreeVar(ClausePosGetSide(from))||problemType==PROBLEM_HO||
     EqnIsEquLit(into->literal)||!TermPosIsTopPos(into->pos));

   into_term = ClausePosGetSubterm(into);

   if(!TermCellQueryProp(into_term, TPPotentialParamod))
   {
      return NULL;
   }
   from_term = ClausePosGetSide(from);
   subst = SubstAlloc();
   VarBankResetVCounts(freshvars);
   unify_res = SubstMguComplete(from_term, into_term, subst);

   if((UnifFailed(unify_res) ||
       !CheckHOUnificationConstraints(unify_res, RightTerm, from_term, into_term)) ||
      (!EqnIsOriented(from->literal) &&
       TOGreater(ocb, ClausePosGetOtherSide(from), from_term,
                 DEREF_ALWAYS, DEREF_ALWAYS)))
   {
      /* Fail because of into-position invariant property of into-term
       * - either we don't unify, or the intantiated from-term is no
       longer maximal in its literal */
      TermCellDelProp(into_term, TPPotentialParamod);
   }
   else if(!EqnIsOriented(into->literal) &&
           TOGreater(ocb, ClausePosGetOtherSide(into), ClausePosGetSide(into),
                     DEREF_ALWAYS, DEREF_ALWAYS))
   {
      /* Do nothing - we fail because of an into-property that is not
         invariant over positions (the instantiated into-position is no
         longer in a maximal term in the literal) */
   }
   else if(!EqnListEqnIsStrictlyMaximal(ocb,
                                        from->clause->literals,
                                        from->literal))
   {
      /* Fail because of into-position invariant property of into-term
       * - the unifier causes the from-literal to be no longer
       strictly maximal */
      TermCellDelProp(into_term, TPPotentialParamod);
   }
   else if(!((EqnIsPositive(into->literal)&&
              EqnListEqnIsStrictlyMaximal(ocb,
                                          into->clause->literals,
                                          into->literal))
             ||
             (EqnIsNegative(into->literal) &&
              EqnListEqnIsMaximal(ocb,
                                  into->clause->literals,
                                  into->literal)))
      )
   {
      /* Do nothing - we fail because of an into-property that is not
         invariant over positions - the instantiated into-literal is
         no longer (strictly) maximal */
   }
   else
   { /* Now we build the new clause! */
      /* _all_ instances of into_term are handled */
      TermCellDelProp(into_term, TPPotentialParamod);

      NormSubstEqnListExcept(into->clause->literals, NULL, subst, freshvars);
      NormSubstEqnListExcept(from->clause->literals, NULL, subst, freshvars);

      Term_p tmp_rhs = MakeRewrittenTerm(TermDerefAlways(into_term),
                                         TermDerefAlways(ClausePosGetOtherSide(from)),
                                         0,
                                         bank);
      rhs_instance = TBInsertNoProps(bank, tmp_rhs, DEREF_ALWAYS);

      into_copy = EqnListCopyRepl(into->clause->literals,
                                  bank, into_term, rhs_instance);
      if(EqnListFindTrue(into_copy))
      {
         EqnListFree(into_copy);
      }
      else
      {
         from_copy = EqnListCopyOptExcept(from->clause->literals,
                                          from->literal);
         if(EqnListFindTrue(from_copy))
         {
            EqnListFree(into_copy);
            EqnListFree(from_copy);
         }
         else
         {
            EqnListDelProp(into_copy, EPFromClauseLit);
            EqnListSetProp(from_copy, EPFromClauseLit);

            into_copy = EqnListAppend(&into_copy, from_copy);

            EqnListRemoveResolved(&into_copy);
            EqnListRemoveDuplicates(into_copy);
            new_clause = ClauseAlloc(into_copy);
         }
      }
   }

   SubstDelete(subst);
   return new_clause;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseOrderedSuperSimParamod()
//
//   Perform a simultaneous ordered simultaneous paramod step (if
//   necessary).
//
// Global Variables: - (but reads term property TPPotentialParamod)
//
// Side Effects    : Memory operations, changes fresh variable counter
//
/----------------------------------------------------------------------*/

Clause_p ClauseOrderedSuperSimParamod(TB_p bank, OCB_p ocb, ClausePos_p
                                      from, ClausePos_p into, VarBank_p
                                      freshvars)
{
   Clause_p  new_clause = NULL;
   Term_p    rhs_instance, from_term, into_term;
   Eqn_p     into_copy, from_copy, tmp_copy;
   Subst_p   subst;
   UnificationResult unify_res;

   assert(EqnIsMaximal(from->literal));
   assert(!EqnIsOriented(from->literal)||(from->side==LeftSide));
   // In HO case variables might paramodulate into predicate positions
   assert(!TermIsFreeVar(ClausePosGetSide(from))||problemType==PROBLEM_HO||
     EqnIsEquLit(into->literal)||!TermPosIsTopPos(into->pos));

   into_term = ClausePosGetSubterm(into);

   if(!TermCellQueryProp(into_term, TPPotentialParamod))
   {
      return NULL;
   }
   from_term = ClausePosGetSide(from);
   subst = SubstAlloc();
   VarBankResetVCounts(freshvars);
   unify_res = SubstMguComplete(from_term, into_term, subst);

   if((UnifFailed(unify_res) ||
       !CheckHOUnificationConstraints(unify_res, RightTerm, from_term, into_term)) ||
      (!EqnIsOriented(from->literal) &&
       TOGreater(ocb, ClausePosGetOtherSide(from), from_term,
                 DEREF_ALWAYS, DEREF_ALWAYS)))
   {
      /* Fail because of into-position invariant property of into-term
       * - either we don't unify, or the intantiated from-term is no
       longer maximal in its literal */
      TermCellDelProp(into_term, TPPotentialParamod);
   }
   else if(!EqnIsOriented(into->literal) &&
           TOGreater(ocb, ClausePosGetOtherSide(into), ClausePosGetSide(into),
                     DEREF_ALWAYS, DEREF_ALWAYS))
   {
      /* Do nothing - we fail because of an into-property that is not
         invariant over positions (the instantiated into-position is no
         longer in a maximal term in the literal) */
   }
   else if(!EqnListEqnIsStrictlyMaximal(ocb,
                                        from->clause->literals,
                                        from->literal))
   {
      /* Fail because of into-position invariant property of into-term
       * - the unifier causes the from-literal to be no longer
       strictly maximal */
      TermCellDelProp(into_term, TPPotentialParamod);
   }
   else if(!((EqnIsPositive(into->literal)&&
              EqnListEqnIsStrictlyMaximal(ocb,
                                          into->clause->literals,
                                          into->literal))
             ||
             (EqnIsNegative(into->literal) &&
              EqnListEqnIsMaximal(ocb,
                                  into->clause->literals,
                                  into->literal)))
      )
   {
      /* Do nothing - we fail because of an into-property that is not
         invariant over positions - the instantiated into-literal is
         no longer (strictly) maximal */
   }
   else
   { /* Now we build the new clause! */
      /* _all_ instances of into_term are handled */
      TermCellDelProp(into_term, TPPotentialParamod);

      NormSubstEqnListExcept(into->clause->literals, NULL, subst, freshvars);
      NormSubstEqnListExcept(from->clause->literals, NULL, subst, freshvars);

      Term_p tmp_lhs = TBInsert(bank, into_term, DEREF_ALWAYS);
      Term_p tmp_rhs = MakeRewrittenTerm(TermDerefAlways(into_term),
                                         TermDerefAlways(ClausePosGetOtherSide(from)),
                                         0,
                                         bank);
      rhs_instance = TBInsertNoProps(bank, tmp_rhs, DEREF_ALWAYS);


      tmp_copy = EqnListCopy(into->clause->literals, bank);

      into_copy = EqnListCopyRepl(tmp_copy, bank, tmp_lhs, rhs_instance);

      EqnListFree(tmp_copy);

      if(EqnListFindTrue(into_copy))
      {
         EqnListFree(into_copy);
      }
      else
      {
         from_copy = EqnListCopyOptExcept(from->clause->literals,
                                          from->literal);
         if(EqnListFindTrue(from_copy))
         {
            EqnListFree(into_copy);
            EqnListFree(from_copy);
         }
         else
         {
            EqnListDelProp(into_copy, EPFromClauseLit);
            EqnListSetProp(from_copy, EPFromClauseLit);

            into_copy = EqnListAppend(&into_copy, from_copy);

            EqnListRemoveResolved(&into_copy);
            EqnListRemoveDuplicates(into_copy);
            new_clause = ClauseAlloc(into_copy);
         }
      }
   }
   SubstDelete(subst);
   return new_clause;
}



/*-----------------------------------------------------------------------
//
// MACRO: IS_NO_PARAMOD_POS
//
//   Check wether a position is a valid paramod-position. Ugly!
//
// Global Variables: Reads local scope variable pos, no_top amd res.
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define IS_NO_PARAMOD_POS \
   (TermIsFreeVar(res)|| /* No paramod into variables */ \
  /* Only overlap positive root positions once */\
  (EqnIsPositive(pos->literal) && no_top && TermPosIsTopPos(pos->pos))||\
  /* Don't overlap variable into predicate position */\
    (TermIsFreeVar(ClausePosGetSide(from_pos)) &&\
     problemType == PROBLEM_FO && !EqnIsEquLit(pos->literal) &&\
     TermPosIsTopPos(pos->pos)))

/*-----------------------------------------------------------------------
//
// Function: ClausePosFirstParamodInto()
//
//   Find the first potential paramod-position in clause. If no_top is
//   true, do not select top positions of terms. Returns the term at
//   the selected position, or NULL if no position exists. If
//   successful and simu_paramod is true, also resets
//   TPPotentialParamod in this and potentially following positions.
//
// Global Variables: ParamodOverlapIntoNegativeLiterals,
//                   ParamodOverlapNonEqLiterals
//
// Side Effects    : Changes pos
//
/----------------------------------------------------------------------*/

Term_p ClausePosFirstParamodInto(Clause_p clause, ClausePos_p pos,
                                 ClausePos_p from_pos, bool no_top,
                                 ParamodulationType pm_type)
{
   Term_p res;

   pos->clause = clause;
   pos->literal = clause->literals;

   if(EqnIsEquLit(from_pos->literal) || problemType == PROBLEM_HO)
   {
      res = ClausePosFindFirstMaximalSubterm(pos);
   }
   else
   {  /* We don't need to try subterms, they won't unify anyways. We
         also don't need positive literals, because at best we would
         get a tautology. */
      res = clause_pos_find_first_neg_max_lside(pos);
   }
   if(res)
   {
      if(IS_NO_PARAMOD_POS)
      {
         res = ClausePosNextParamodInto(pos, from_pos, no_top);
      }
   }
   if(pm_type!=ParamodPlain && res)
   {
      /* Hack: We only need to reset this from here on! Previous
       * position will not be tested anyways */
      EqnListTermSetProp(pos->literal, TPPotentialParamod);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosNextParamodInto()
//
//   Given a position, find the next potential paramod-position. Avoid
//   top-positions if no_top is true. Returns the term at
//   the selected position, or NULL if no position exists.
//
// Global Variables: ParamodOverlapIntoNegativeLiterals,
//                   ParamodOverlapNonEqLiterals
//
// Side Effects    : Changes pos
//
/----------------------------------------------------------------------*/

Term_p ClausePosNextParamodInto(ClausePos_p pos, ClausePos_p from_pos,
                                bool no_top)
{
   Term_p res;

   if(EqnIsEquLit(from_pos->literal) || problemType == PROBLEM_HO)
   {
      res = ClausePosFindNextMaximalSubterm(pos);
   }
   else
   {  /* We don't need to try subterms, they won't unify anyways -- not true in HO case. We
         also don't need positive literals, because at best we would
         get a tautology. */
      pos->literal = pos->literal->next;
      res = clause_pos_find_first_neg_max_lside(pos);
   }
   while(res && (IS_NO_PARAMOD_POS))
   {
      if(EqnIsEquLit(from_pos->literal) || problemType == PROBLEM_HO)
      {
         res = ClausePosFindNextMaximalSubterm(pos);
      }
      else
      {  /* See above - same reasoning -- also not true in HO  */
         pos->literal = pos->literal->next;
         res = clause_pos_find_first_neg_max_lside(pos);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosFirstParamodFromSide()
//
//   Given a clause and a position, set the position to the first
//   side that can be used for paramodulation. Does not check strategy
//   for efficiency reasons ClausePos*ParamodPair() should ensure that
//   this is only called in cases were it makes sense.
//
// Global Variables: ParamodOverlapIntoNegativeLiterals,
//                   ParamodOverlapNonEqLiterals
//
// Side Effects    : Changes pos
//
/----------------------------------------------------------------------*/

Term_p ClausePosFirstParamodFromSide(Clause_p from,
                                     ClausePos_p from_pos)
{
   Term_p res = NULL;

   from_pos->clause = from;
   from_pos->literal = from->literals;
   res = ClausePosFindFirstMaximalSide(from_pos, true);
   while(res  &&
         ((!(ParamodOverlapNonEqLiterals ||
             EqnIsEquLit(from_pos->literal)))
          ||
          EqnIsSelected(from_pos->literal)))
   {
      res = ClausePosFindNextMaximalSide(from_pos, true);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosNextParamodFromSide()
//
//   Given a position, set the position to the next side that can be
//   used for paramodulation. Does not check strategy for efficiency
//   reasons. ClausePos*ParamodPair() should ensure that this is only
//   called in cases were it makes sense.
//
// Global Variables: ParamodOverlapIntoNegativeLiterals,
//                   ParamodOverlapNonEqLiterals
//
// Side Effects    : Changes pos
//
/----------------------------------------------------------------------*/

Term_p ClausePosNextParamodFromSide(ClausePos_p from_pos)
{
   Term_p res;

   res = ClausePosFindNextMaximalSide(from_pos, true);
   while(res  &&
         ((!(ParamodOverlapNonEqLiterals ||
             EqnIsEquLit(from_pos->literal)))
          ||
          EqnIsSelected(from_pos->literal)))
   {
      res = ClausePosFindNextMaximalSide(from_pos, true);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function:  ClausePosFirstParamodPair()
//
//   Given two clauses, create the first possible paramod-position
//   from a literal in from into a literal in into. Return term
//   paramodualated into, or NULL if no position exists.
//
// Global Variables: -
//
// Side Effects    : Changes from_pos, into_pos.
//
/----------------------------------------------------------------------*/

Term_p ClausePosFirstParamodPair(Clause_p from, ClausePos_p from_pos,
                                 Clause_p into, ClausePos_p into_pos,
                                 bool no_top, ParamodulationType pm_type)
{
   Term_p res;

   res = ClausePosFirstParamodFromSide(from, from_pos);
   assert(TermPosIsTopPos(from_pos->pos));

   while(res)
   {
      res = ClausePosFirstParamodInto(into, into_pos,
                                      from_pos, no_top,
                                      pm_type);
      if(res)
      {
         break;
      }
      res = ClausePosNextParamodFromSide(from_pos);
      assert(TermPosIsTopPos(from_pos->pos));
   }
   assert(TermPosIsTopPos(from_pos->pos));

   return res;
}

/*-----------------------------------------------------------------------
//
// Function:  ClausePosNextParamodPair()
//
//   Given two clause positions, compute the next possible
//   paramod-position from a literal in from into a literal in
//   into. Return term paramodualated into, or NULL if no position
//   exists.
//
// Global Variables: -
//
// Side Effects    : Changes from_pos, into_pos.
//
/----------------------------------------------------------------------*/

Term_p ClausePosNextParamodPair(ClausePos_p from_pos, ClausePos_p
                                into_pos, bool no_top, ParamodulationType pm_type)
{
   Term_p res;

   res = ClausePosNextParamodInto(into_pos, from_pos, no_top);
   if(!res)
   {
      res = ClausePosNextParamodFromSide(from_pos);
      assert(TermPosIsTopPos(from_pos->pos));
      while(res)
      {
         res = ClausePosFirstParamodInto(into_pos->clause, into_pos,
                                         from_pos, no_top, pm_type);
         if(res)
         {
            break;
         }
         res = ClausePosNextParamodFromSide(from_pos);
         assert(TermPosIsTopPos(from_pos->pos));
      }
   }
   assert(TermPosIsTopPos(from_pos->pos));
   return res;
}


#ifdef ENABLE_LFHO
/*-----------------------------------------------------------------------
//
// Function:  CheckHOUnificationConstraints()
//
//   Checks whether arguments are trailing on the right side of the
//   equation (into term) and whether we are not paramodulating into
//   the variable head of applied variable term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool CheckHOUnificationConstraints(UnificationResult res, UnifTermSide exp_side, Term_p from, Term_p to)
{
   return true;
}
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
