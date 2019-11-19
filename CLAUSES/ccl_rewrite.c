/*-----------------------------------------------------------------------

  File  : ccl_rewrite.c

  Author: Stephan Schulz

  Contents

  Rewriting.

  Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Creates: Tue May 26 19:47:52 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_rewrite.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long RewriteAttempts    = 0;
long RewriteSuccesses   = 0;
long RewriteUnboundVarFails = 0;
long BWRWMatchAttempts  = 0;
long BWRWMatchSuccesses = 0;
long BWRWRwSuccesses = 0;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static Term_p term_li_normalform(RWDesc_p desc, Term_p term,
                                 bool restricted_rw);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: subst_complete_min_instance()
//
//   Complete the substitution by binding any unbound variable to the
//   minimum term of the appropriate type.
//
// Global Variables: -
//
// Side Effects    : May create minimal terms and constants
//
/----------------------------------------------------------------------*/

static void subst_complete_min_instance(OCB_p ocb, TB_p bank,
                                        Subst_p subst, Term_p term)
{
   int i;

   if(TermIsVar(term))
   {
      if(!(term->binding))
      {
         SubstAddBinding(subst, term,
                         OCBDesignatedMinTerm(ocb, bank, term->type));
      }
   }
   else
   {
      for(i=0;i<term->arity;i++)
      {
         subst_complete_min_instance(ocb, bank, subst, term->args[i]);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: instance_is_rule()
//
//   Return true if lside->rside is a rule, i.e. lside>rside (for the
//   instantiated terms) and rside contains no unbound
//   variables. Assumes that uninstantated terms lside and rside are
//   uncomparable!
//
// Global Variables: -
//
// Side Effects    : TOGreater() can manipulate TPOpFlag of terms.
//
/----------------------------------------------------------------------*/

static bool instance_is_rule(OCB_p ocb, TB_p bank,
                             Term_p lside, Term_p rside, Subst_p subst)

{
   if(ocb->rewrite_strong_rhs_inst)
   {
      subst_complete_min_instance(ocb, bank, subst, rside);
   }
   else if(TermHasUnboundVariables(rside))
   {
      RewriteUnboundVarFails++;
      return false;
   }
   if(SubstIsRenaming(subst)) /* Save comparisons */
   {
      return false;
   }
   return TOGreater(ocb, lside, rside, DEREF_ONCE, DEREF_ONCE);
}


/*-----------------------------------------------------------------------
//
// Function: term_follow_top_RW_chain()
//
//   Return the last term in an existing rewrite link chain, following
//   only top rewrite links. If one of those is induced by a SoS
//   clause, set desc->sos.
//
// Global Variables: -
//
// Side Effects    : May change desc->sos_rewritten.
//
/----------------------------------------------------------------------*/

/* static */ Term_p term_follow_top_RW_chain(Term_p term, RWDesc_p desc,
                                             bool restricted_rw)
{
   assert(term);

   while(TermIsTopRewritten(term)&&(!restricted_rw||TermIsRRewritten(term)))
   {
      assert(term);
      if(TermCellQueryProp(term, TPIsSOSRewritten))
      {
         desc->sos_rewritten = true;
      }

      assert(TOGreater(desc->ocb, term, TermRWReplaceField(term),
                        DEREF_NEVER, DEREF_NEVER));
      term = TermRWReplaceField(term);
      assert(term);
   }
   return term;
}


/*-----------------------------------------------------------------------
//
// Function: term_is_top_rewritable()
//
//   Return true if the term is rewritable with new_demod at the top
//   position, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Temporary variable bindings (backtracked)
//
/----------------------------------------------------------------------*/

static RWResultType term_is_top_rewritable(TB_p bank, OCB_p ocb,
                                           Term_p term, Clause_p
                                           new_demod, bool restricted_rw)
{
   Subst_p      subst = SubstAlloc();
   Eqn_p        eqn;
   RWResultType res   = RWNotRewritable;
   Term_p       rterm;

   assert(new_demod->pos_lit_no == 1);
   assert(new_demod->neg_lit_no == 0);
   assert(!TermIsVar(term));

   eqn = new_demod->literals;

   /* printf("Checking term: ");
      TBPrintTermFull(stdout, eqn->bank, term);
      printf("\n");
      printf("with demod clause %ld: ", new_demod->ident);
      ClausePrint(stdout, new_demod, true);
      printf("\n");*/
   BWRWMatchAttempts++;
   int remains =  MATCH_FAILED;
   if((remains = SubstMatchPossiblyPartial(eqn->lterm, term, subst)) != MATCH_FAILED)
   {
      BWRWMatchSuccesses++;
      assert(problemType == PROBLEM_FO || !remains);
      if((EqnIsOriented(eqn)
          || instance_is_rule(ocb, eqn->bank, eqn->lterm, eqn->rterm, subst)))
      {
         if(!EqnIsOriented(eqn) || /* Only a performance hack */
            !SubstIsRenaming(subst))
         {
            res = RWAlwaysRewritable;
            TermCellSetProp(term, TPIsRRewritable|TPIsRewritable);
         }
         else
         {
            res = RWLimitedRewritable;
            TermCellSetProp(term, TPIsRewritable);
         }
         if(!TermIsRewritten(term) || (res == RWAlwaysRewritable))
         {
            Term_p tmp_rewritten = MakeRewrittenTerm(term, eqn->rterm, remains, bank);
            rterm = TBInsertInstantiated(bank, tmp_rewritten);

            if(remains)
            {
               TermTopFree(tmp_rewritten);
               tmp_rewritten = NULL;
            }

            TermAddRWLink(term, rterm, new_demod, ClauseIsSOS(new_demod), res);
         }
      }
      SubstBacktrack(subst);
   }
   if(!((res == RWAlwaysRewritable)||
        (!restricted_rw&&res==RWLimitedRewritable))
      &&
      !EqnIsOriented(eqn))
   {
      BWRWMatchAttempts++;
      if((remains = SubstMatchPossiblyPartial(eqn->rterm, term, subst)) != MATCH_FAILED)
      {
         BWRWMatchSuccesses++;
         if(instance_is_rule(ocb, eqn->bank, eqn->rterm, eqn->lterm, subst))
          /* If instance is rule -> subst is no renaming! */
         {
            assert(!SubstIsRenaming(subst));
            TermCellSetProp(term, TPIsRRewritable|TPIsRewritable);
            res = RWAlwaysRewritable;

            Term_p tmp_rewritten = MakeRewrittenTerm(term, eqn->lterm, remains, bank);

            rterm = TBInsertInstantiated(bank, tmp_rewritten);

            if(remains)
            {
               TermTopFree(tmp_rewritten);
            }

            TermAddRWLink(term, rterm, new_demod, ClauseIsSOS(new_demod), res);
         }
      }
   }
   SubstDelete(subst);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: term_is_rewritable()
//
//   Return true if the term is rewritable with new_demod, false
//   otherwise. Set nf_date[0,1] on non-rewritable terms to nf_date
//   (i.e. assumes thate term is in normal for with respect to earlier
//   systems).
//
//   I keep this like it is for the moment despite the new
//   rewriting. We may loose a few cycles by not immediately adding a
//   rewrite link if we detected a possible rewrite step, but on the
//   other hand we may have fewer intermediate term cells this way,
//   since the actual rewriting is performed later. The most elegant
//   option would probably be to add the rewrite link in
//   term_is_top_rewritable() only, but to avoid propagating it up!
//   --> TODO!
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms, may set TPIsRewritable
//                   flag.
//
/----------------------------------------------------------------------*/

static bool term_is_rewritable(TB_p bank, OCB_p ocb, Term_p term, Clause_p
                               new_demod, SysDate nf_date,
                               bool restricted_rw)
{
   int i;
   bool res = false;
   RWResultType topres;

   /*printf("term_is_rewritable(");
     TermPrint(stdout, term, ocb->sig, DEREF_NEVER);
     printf(")...\n"); */

   if(TermIsVar(term))
   {
      return false;
   }
   if(TermCellQueryProp(term, TPIsRRewritable) ||
      (!restricted_rw &&
       TermCellQueryProp(term, TPIsRewritable)))
   {
      return true;
   }

   /* assert(!TermIsRewritten(term));*/

   if(SysDateEqual(term->rw_data.nf_date[RewriteAdr(FullRewrite)], nf_date))
   {
      return false;
   }
   for(i=0; i<term->arity; i++)
   {
      if(term_is_rewritable(bank, ocb, term->args[i], new_demod, nf_date, false))
      {
         res = true;
         break;
      }
   }
   if(res)
   {
      TermCellSetProp(term, TPIsRewritable|TPIsRRewritable);
      return true;
   }
   topres = term_is_top_rewritable(bank, ocb, term, new_demod, restricted_rw);
   /* Properties set in term_is_top_rewritable! */
   switch(topres)
   {
   case RWLimitedRewritable:
         return !restricted_rw;
   case RWAlwaysRewritable:
         return true;
   default:
         /* Nothing to do, see below. */
         break;
   }
   if(!TermCellIsAnyPropSet(term,
                            TPIsRewritable|TPIsRRewritable|
                            TPIsRewritten|TPIsRRewritten)
      &&!restricted_rw)
   {
      term->rw_data.nf_date[RewriteAdr(RuleRewrite)] =
         term->rw_data.nf_date[RewriteAdr(FullRewrite)] = nf_date;
   }
   /* printf("...term_is_rewritable() = false (no match)\n");*/
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: eqn_has_rw_side()
//
//   Return NoSide, MaxSide, MinSide depending on wether
//   eqn does or doesn't have a rewritable (maximal) side.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms.
//
/----------------------------------------------------------------------*/

static EqnSide eqn_has_rw_side(OCB_p ocb, Eqn_p eqn, Clause_p
                               new_demod, SysDate nf_date)
{
   bool resl, resr;
   bool restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) && EqnIsOriented(eqn);

   /* printf("restricted_rw: %d\n", restricted_rw); */
   resl = term_is_rewritable(eqn->bank, ocb, eqn->lterm, new_demod, nf_date,
                             restricted_rw);
   resr = term_is_rewritable(eqn->bank, ocb, eqn->rterm, new_demod, nf_date,
                             false);

   if(resl)
   {
      return MaxSide;
   }
   else if(resr)
   {
      /* printf("resr %d\n",EqnIsOriented(eqn)); */
      return EqnIsOriented(eqn)?MinSide:MaxSide;
   }
   return NoSide;
}


/*-----------------------------------------------------------------------
//
// Function: clause_is_rewritable()
//
//   Return true if clause is rewriteable with new_demod.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms.
//
/----------------------------------------------------------------------*/

static bool clause_is_rewritable(OCB_p ocb, Clause_p clause,
                                 Clause_p new_demod, SysDate
                                 nf_date)
{
   Eqn_p handle;
   EqnSide tmp;
   bool res = false;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = eqn_has_rw_side(ocb, handle, new_demod, nf_date);
      if(tmp != NoSide)
      {
         res = true;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: find_rewritable_clauses()
//
//   A non-index-using implementation of
//   FindRewritableClause(). Returns true if any clause
//   is rewritable
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms.
//
/----------------------------------------------------------------------*/

static bool find_rewritable_clauses(OCB_p ocb, ClauseSet_p set,
                                    PStack_p results, Clause_p
                                    new_demod, SysDate nf_date)
{
   Clause_p handle;
   bool     res = false, tmp;

   assert(new_demod->pos_lit_no == 1);
   assert(new_demod->neg_lit_no == 0);

   for(handle = set->anchor->succ; handle != set->anchor; handle =
          handle->succ)
   {
      tmp = clause_is_rewritable(ocb, handle, new_demod, nf_date);
      if(tmp)
      {
         PStackPushP(results, handle);
         res = true;
      }
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: indexed_find_demodulator()
//
//   Find a demodulator via demodulators->demod_index.
//
// Global Variables: -
//
// Side Effects    : Instantiates
//
/----------------------------------------------------------------------*/

MatchRes_p indexed_find_demodulator(OCB_p ocb, Term_p term,
                                    SysDate date,
                                    ClauseSet_p demodulators,
                                    Subst_p subst,
                                    bool prefer_general,
                                    bool restricted_rw)
{
   Eqn_p       eqn;
   ClausePos_p pos, res = NULL;
   MatchRes_p match_info;

   assert(term);
   assert(demodulators);
   assert(demodulators->clauseset_indexes->demod_index);
   assert(term->weight ==
            TermWeight(term, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(!TermIsTopRewritten(term));

   RewriteAttempts++;

   PDTreeSearchInit(demodulators->clauseset_indexes->demod_index, term, date, prefer_general);

   while((match_info = PDTreeFindNextDemodulator(demodulators->clauseset_indexes->demod_index, subst)))
   {
      pos = match_info->pos;
      eqn = pos->literal;

      if((EqnIsOriented(eqn)&&
          !SysDateIsEarlier(TermNFDate(term,RewriteAdr(RuleRewrite)),
                            pos->clause->date))
         ||
         (!EqnIsOriented(eqn)&&
          !SysDateIsEarlier(TermNFDate(term,RewriteAdr(FullRewrite)),
                            pos->clause->date)))
      {
         MatchResFree(match_info); // avoid memory leak -- it was alloc'd in PDTreeFindNextDemodulator
         continue;
      }
      switch(pos->side)
      {
      case LeftSide:
            if((EqnIsOriented(eqn)
                || instance_is_rule(ocb, eqn->bank, eqn->lterm, eqn->rterm, subst))
               &&
               (!restricted_rw ||
                !SubstIsRenaming(subst)))
            {
               res = pos;
            }
            break;
      case RightSide:
            assert(!EqnIsOriented(eqn));
            if(instance_is_rule(ocb, eqn->bank, eqn->rterm, eqn->lterm, subst)
               /* &&
                  !restricted_rw */)
               /* Case SubstIsRenaming(subst) already eliminated in
                  instance_is_rule! */
               /* The prevous condition seems wrong! If subst is a
                  real substitution, we can alwayws rewrite! TODO! */
            {
               res = pos;
            }
            break;
      default:
            assert(false);
            break;
      }
      if(res)
      {
         break;
      }
      MatchResFree(match_info);
   }
   PDTreeSearchExit(demodulators->clauseset_indexes->demod_index);

#ifndef NDEBUG
   if(match_info
      && !TermStructPrefixEqual(ClausePosGetSide(match_info->pos), term, DEREF_ONCE, DEREF_NEVER,
                                match_info->remaining_args, ocb->sig))
   {
      fprintf(stderr, "Term ");
      TermPrint(stderr, ClausePosGetSide(match_info->pos), ocb->sig, DEREF_NEVER);
      fprintf(stderr, " derefed { ");
      TermPrint(stderr, ClausePosGetSide(match_info->pos), ocb->sig, DEREF_ONCE);
      fprintf(stderr, " } should match ");
      TermPrint(stderr, term, ocb->sig, DEREF_NEVER);
      fprintf(stderr, ", substitution is : ");
      SubstPrint(stderr, subst, ocb->sig, DEREF_NEVER);
      fprintf(stderr, ", trailing %d.\n", match_info->remaining_args);

      assert(false);
   }
#endif
   return match_info;
}


/*-----------------------------------------------------------------------
//
// Function: rewrite_with_clauseset()
//
//   Rewrite the given term at root position with the first matching,
//   orientable rule from demodulators. Return new term.
//
// Global Variables: -
//
// Side Effects    : May add rewrite link to term.
//
/----------------------------------------------------------------------*/

static Term_p rewrite_with_clause_set(OCB_p ocb, TB_p bank, Term_p term,
                                      SysDate date, ClauseSet_p
                                      demodulators, bool prefer_general,
                                      bool restricted_rw)
{
   Subst_p     subst = SubstAlloc();
   MatchRes_p mi;
   Term_p      repl;

   assert(demodulators->clauseset_indexes->demod_index);
   assert(term);
   assert(!TermIsVar(term));
   assert(!TermIsTopRewritten(term));

   mi = indexed_find_demodulator(ocb, term, date, demodulators,
                                 subst, prefer_general, restricted_rw);
   if(mi)
   {
      RewriteSuccesses++;
      assert(problemType == PROBLEM_HO || mi->remaining_args == 0);

      repl = TBInsertInstantiated(bank, ClausePosGetOtherSide(mi->pos));

      if(problemType == PROBLEM_HO)
      {
         repl = MakeRewrittenTerm(term, repl, mi->remaining_args, bank);
         if(mi->remaining_args)
         {
            repl = TBTermTopInsert(bank, repl);
         }
      }

      assert(mi->pos->clause->ident);
      TermAddRWLink(term, repl, mi->pos->clause, ClauseIsSOS(mi->pos->clause),
                    restricted_rw?RWAlwaysRewritable:RWLimitedRewritable);
      assert(TOGreater(ocb, term, repl, DEREF_NEVER, DEREF_NEVER));

      term = repl;
      MatchResFree(mi);
   }
   SubstDelete(subst);

   return term;
}



/*-----------------------------------------------------------------------
//
// Function: rewrite_with_clause_setlist()
//
//   Rewrite a term at top level with the sets of
//   demodulators. Returns new term.
//
// Global Variables:
//
// Side Effects    : Only by rewrite_with_clause_set.
//
/----------------------------------------------------------------------*/

static Term_p rewrite_with_clause_setlist(OCB_p ocb, TB_p bank, Term_p term,
                                          ClauseSet_p* demodulators,
                                          RewriteLevel level, bool
                                          prefer_general,
                                          bool restricted_rw)
{
   unsigned int  i;
   Term_p res = term;

   assert(level);
   assert(!TermIsVar(term));
   assert(!TermIsTopRewritten(term));

   for(i=0; i<level; i++)
   {
      assert(demodulators[i]);

      if(SysDateIsEarlier(TermNFDate(term,level-1), demodulators[i]->date))
      {
         res = rewrite_with_clause_set(ocb, bank, term,
                                       TermNFDate(term,level-1),
                                       demodulators[i],
                                       prefer_general,
                                       restricted_rw);
         if(res!=term)
         {
            break;
         }
      }
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: term_subterm_rewrite()
//
//   Normalize the subterms of the given term and propagate the result
//   to term. Returns modification, result per *term.
//
// Global Variables: -
//
// Side Effects    : May change termbank, introduce rw-links.
//
/----------------------------------------------------------------------*/

static bool term_subterm_rewrite(RWDesc_p desc, Term_p *term)
{
   bool modified = false;
   Term_p new_term = TermTopCopyWithoutArgs(*term);
   int  i;

   for(i=0; i<(*term)->arity; i++)
   {
      new_term->args[i] = term_li_normalform(desc, (*term)->args[i], false);
      modified = modified || (new_term->args[i]!= (*term)->args[i]);
   }
   if(modified)
   {
      new_term = TBTermTopInsert(desc->bank, new_term);
      assert(new_term!=*term);
      TermAddRWLink(*term, new_term, REWRITE_AT_SUBTERM, false,
                    RWAlwaysRewritable);
      *term = new_term;
   }
   else
   {
      TermTopFree(new_term);
   }
   return modified;
}

/*-----------------------------------------------------------------------
//
// Function: term_li_normalform()
//
//   Compute a leftmost-innermost normal form of term. This uses
//   dates to minimize rewrite-attempts: If the normal form of the
//   term is younger than the clause sets, no further rewrite-attempt
//   on this term is made.
//
// Global Variables: -
//
// Side Effects    : Changes terms, updates nf_date.
//
/----------------------------------------------------------------------*/

static Term_p term_li_normalform(RWDesc_p desc, Term_p term,
                                 bool restricted_rw)
{
   bool    modified = true;
   Term_p new_term;

   if(desc->level == NoRewrite)
   {
      return term;
   }
   term = term_follow_top_RW_chain(term, desc, restricted_rw);
   assert(!TermIsTopRewritten(term)||restricted_rw);

   if(!TermIsRewritten(term)&&
      !SysDateIsEarlier(term->rw_data.nf_date[desc->level-1],desc->demod_date))
   {
      return term;
   }
   if(TermIsVar(term))
   {
      assert(!TermIsRewritten(term));
      return term;
   }
   while(modified)
   {
      modified = term_subterm_rewrite(desc, &term);

      if(!TermIsVar(term))
      {
         if(TermIsTopRewritten(term))
         {
            new_term = term_follow_top_RW_chain(term, desc, restricted_rw&&(!modified));
         }
         else
         {
            rewrite_with_clause_setlist(desc->ocb, desc->bank,
                                        term, desc->demods,
                                        desc->level,
                                        desc->prefer_general,
                                        restricted_rw&&(!modified));
            new_term = term_follow_top_RW_chain(term, desc, restricted_rw&&(!modified));
         }
         if(term != new_term)
         {
            modified = true;
            term = new_term;
         }
      }
   }
   /* This is tricky! The term may be sub-top-level rewritten by a
      rule that had been eliminated by an equation, or by a rule that
      modified its right hand side (new loop!). So we may not find
      the full normal form here - the rule is gone, and we do not test
      for equations. Thus, we are not necessarily in full normal
      form. Also, if restricted_rw is enabled, non-rewritability may
      be due to the extra constraint and does not carry over. */
   if(!TermIsRewritten(term)&&!restricted_rw)
   {
      term->rw_data.nf_date[RewriteAdr(RuleRewrite)] = desc->demod_date;
      if(desc->level == FullRewrite)
      {
         term->rw_data.nf_date[RewriteAdr(FullRewrite)] = desc->demod_date;
      }
   }
   return term;
}

/*-----------------------------------------------------------------------
//
// Function: eqn_li_normalform()
//
//   Compute the normal form of maximal, minimal or both terms in an
//   equation. Return true if a term was rewritten.
//
// Global Variables: -
//
// Side Effects    : Changes terms and term bank
//
/----------------------------------------------------------------------*/

EqnSide eqn_li_normalform(RWDesc_p desc, ClausePos_p pos, bool interred_rw)
{
   Eqn_p  eqn = pos->literal;
   Term_p l_old = eqn->lterm, r_old = eqn->rterm;
   bool   restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) &&
      EqnIsOriented(eqn) && interred_rw;
   EqnSide res = NoSide;

   eqn->lterm =  term_li_normalform(desc, eqn->lterm, restricted_rw);
   if(l_old!=eqn->lterm)
   {
      EqnDelProp(eqn, EPMaxIsUpToDate);
      res = MaxSide;
      pos->side = LeftSide;
      if(OutputLevel>=4)
      {
         DocClauseRewriteDefault(pos, l_old);
      }
      CLAUSE_ENSURE_DERIVATION(pos->clause);
      TermComputeRWSequence(pos->clause->derivation,
                            l_old, ClausePosGetSide(pos), DCRewrite);
   }
   eqn->rterm = term_li_normalform(desc, eqn->rterm, false);
   if(r_old!=eqn->rterm)
   {
      if(EqnIsOriented(eqn))
      {
         res = res|MinSide;
      }
      else
      {
         res = res|MaxSide;
      }
      if(!EqnIsOriented(eqn))
      {
         EqnDelProp(eqn, EPMaxIsUpToDate);
      }
      pos->side = RightSide;
      if(OutputLevel>=4)
      {
         DocClauseRewriteDefault(pos, r_old);
      }
      CLAUSE_ENSURE_DERIVATION(pos->clause);
      TermComputeRWSequence(pos->clause->derivation,
                            r_old, ClausePosGetSide(pos), DCRewrite);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: rw_desc_cell_alloc()
//
//   Allocate an initialized RWDescCell.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

static __inline__ RWDesc_p rw_desc_cell_alloc(OCB_p ocb, TB_p bank,
                                              ClauseSet_p
                                              *demodulators,
                                              RewriteLevel level, bool
                                              prefer_general)
{
   RWDesc_p desc = RWDescCellAlloc();

   desc->ocb            = ocb;
   desc->bank           = bank;
   desc->demods         = demodulators;
   desc->demod_date     = ClauseSetListGetMaxDate(demodulators, level);
   desc->level          = level;
   desc->prefer_general = prefer_general;
   desc->sos_rewritten  = false;

   return desc;
}


/*-----------------------------------------------------------------------
//
// Function: clause_tree_push()
//
//   Push all clauses in the tree onto stack (unless already there,
//   indicated by CPRWDetected).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long clause_tree_push(PStack_p stack, PTree_p clausetree)
{
   PStack_p iterstack;
   PTree_p  cell;
   Clause_p clause;
   long     res = 0;

   iterstack = PTreeTraverseInit(clausetree);

   while((cell = PTreeTraverseNext(iterstack)))
   {
      clause = cell->key;
      if(!ClauseQueryProp(clause, CPRWDetected))
      {
         ClauseSetProp(clause, CPRWDetected);
         PStackPushP(stack, clause);
         res++;
      }
   }
   PTreeTraverseExit(iterstack);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: term_find_rw_clauses()
//
//   Push all clauses stored at termocc that are rewritable with
//   lterm->rterm onto stack. Return number if there is at least one.
//
// Global Variables: -
//
// Side Effects    : Sets properties and links in affected terms and
//                   clauses. May set the normal-form date even if
//                   this single traverse does not establish
//                   non-rewritability, i.e. assumes that always all
//                   possible directions will be used.
//
/----------------------------------------------------------------------*/

static long term_find_rw_clauses(Clause_p demod,
                                 OCB_p ocb,
                                 SubtermOcc_p termocc,
                                 PStack_p stack,
                                 Term_p lterm,
                                 Term_p rterm,
                                 bool oriented,
                                 SysDate nf_date)
{
   Eqn_p eqn = demod->literals;
   RWResultType rwres = RWNotRewritable;
   long    res = 0;
   Subst_p subst = SubstAlloc();
   Term_p  term = termocc->term;

   assert(!TermIsVar(term));

   BWRWMatchAttempts++;
   int remains = MATCH_FAILED;
   if((remains = SubstMatchPossiblyPartial(lterm, term, subst)) != MATCH_FAILED)
   {
      BWRWMatchSuccesses++;
      if(oriented
         || instance_is_rule(ocb, eqn->bank, lterm, rterm, subst))
      {
         if(!oriented || /* Only a performance hack */
            !SubstIsRenaming(subst))
         {
            TermCellSetProp(term, TPIsRRewritable|TPIsRewritable);
            rwres = RWAlwaysRewritable;
            res += clause_tree_push(stack, termocc->pl.occs.rw_full);
            res += clause_tree_push(stack, termocc->pl.occs.rw_rest);
         }
         else
         {
            TermCellSetProp(term, TPIsRewritable);
            rwres = RWLimitedRewritable;
            res += clause_tree_push(stack, termocc->pl.occs.rw_full);
         }
         if(!TermIsRewritten(term) || (rwres == RWAlwaysRewritable))
         {
            Term_p tmp_rewritten = MakeRewrittenTerm(term, rterm, remains, eqn->bank);
            rterm = TBInsertInstantiated(eqn->bank, tmp_rewritten);

            if(remains)
            {
               TermTopFree(tmp_rewritten);
            }
            TermAddRWLink(term, rterm, demod, ClauseIsSOS(demod), rwres);
            //TermDeleteRWLink(term);
         }
      }
      /* We cannot set the NF date here, since we have no indication
         of the state of the subterms. */
   }
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: tree_find_rw_clauses()
//
//   Push all clauses in termtree that are rewritable with
//   lterm->rterm onto stack. Return number of clauses.
//
// Global Variables: -
//
// Side Effects    : Sets properties and links in affected terms and
//                   clauses. May set the normal-form date even if
//                   this single traverse does not establish
//                   non-rewritability, i.e. assumes that always all
//                   possible directions will be used.
//
/----------------------------------------------------------------------*/

static long tree_find_rw_clauses(Clause_p demod,
                                 OCB_p ocb,
                                 SubtermTree_p termtree,
                                 PStack_p stack,
                                 Term_p lterm,
                                 Term_p rterm,
                                 bool oriented,
                                 SysDate nf_date)
{
   bool          res = 0;
   PStack_p      iterstack;
   SubtermTree_p cell;

   iterstack = PTreeTraverseInit(termtree);

   while((cell = PTreeTraverseNext(iterstack)))
   {
      res += term_find_rw_clauses(demod, ocb, cell->key, stack,
                                  lterm, rterm, oriented, nf_date);
   }
   PTreeTraverseExit(iterstack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: find_rewritable_clauses_indexed()
//
//   Push all clauses in index that are rewritable with lterm->rterm
//   onto stack. Return true if there is at least one.
//
// Global Variables: -
//
// Side Effects    : Sets properties and links in affected terms and
//                   clauses. May set the normal-form date even if
//                   this single traverse does not establish
//                   non-rewritability, i.e. assumes that always all
//                   possible directions will be used.
//
/----------------------------------------------------------------------*/

static long find_rewritable_clauses_indexed(Clause_p demod,
                                            OCB_p ocb,
                                            SubtermIndex_p index,
                                            PStack_p stack,
                                            Term_p lterm,
                                            Term_p rterm,
                                            bool oriented,
                                            SysDate nf_date)
{
   long          res = 0;
   PStack_p      termtrees = PStackAlloc();
   SubtermTree_p tree;

   FPIndexFindMatchable(index, lterm, termtrees);

   while(!PStackEmpty(termtrees))
   {
      tree = PStackPopP(termtrees);
      res += tree_find_rw_clauses(demod, ocb, tree, stack,
                                  lterm, rterm, oriented,
                                  nf_date);
   }
   PStackFree(termtrees);
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TermComputeLINormalform()
//
//   Compute a leftmost-innermost normal form of term and return
//   it. This uses dates to minimize rewrite-attempts: If the normal
//   form of the term is younger than the clause sets, no further
//   rewrite-attempt on this term is made.
//
// Global Variables: -
//
// Side Effects    : Changes terms, updates nf_date.
//
/----------------------------------------------------------------------*/

Term_p TermComputeLINormalform(OCB_p ocb, TB_p bank, Term_p term,
                               ClauseSet_p *demodulators, RewriteLevel
                               level, bool prefer_general,
                               bool restricted_rw)
{
   Term_p res;
   RWDesc_p desc = rw_desc_cell_alloc(ocb, bank, demodulators, level,
                                      prefer_general);

   res = term_li_normalform(desc, term, restricted_rw);
   RWDescCellFree(desc);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseComputeLINormalform()
//
//   Compute the normal form of terms in a clause. Return true if a
//   term was rewritten.
//
// Global Variables: -
//
// Side Effects    : Changes terms and term bank
//
/----------------------------------------------------------------------*/

bool ClauseComputeLINormalform(OCB_p ocb, TB_p bank, Clause_p clause,
                               ClauseSet_p *demodulators,
                               RewriteLevel level, bool prefer_general)
{
   Eqn_p handle;
   EqnSide tmp = NoSide;
   bool res=false;
   RWDesc_p desc = rw_desc_cell_alloc(ocb, bank, demodulators, level,
                                      prefer_general);
   ClausePosCell pos;
   bool done = false;

   assert(demodulators);
   assert(desc->demods);
   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));

   /* printf("# ClauseComputeLINormalform(%ld)...\n",clause->ident); */
   /* if(prefer_general!=0)
      {
      printf("ClauseComputeLINormalform(level=%d prefer_general=%d)\n",
      level, prefer_general);
      } */

   pos.clause = clause;

   while(!done)
   {
      done = true;
      for(handle = clause->literals; handle; handle=handle->next)
      {
         pos.literal = handle;
         tmp = eqn_li_normalform(desc, &pos, ClauseQueryProp(clause,CPLimitedRW));
         if((tmp&MaxSide)
            && EqnIsPositive(handle)
            &&EqnIsMaximal(handle)
            &&ClauseQueryProp(clause,CPLimitedRW))
         {
            ClauseDelProp(clause,CPLimitedRW);
            /* We need to try everything again...*/
            done = false;
         }
         res = res || tmp;
      }
   }
   if(desc->sos_rewritten)
   {
      ClauseSetProp(clause, CPIsSOS);
   }
   if(res)
   {
      ClauseDelProp(clause, CPInitial);
   }
   RWDescCellFree(desc);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetComputeLINormalform()
//
//   Compute a normal form for terms in all clauses in set with
//   respect to clauses in demodulators up to level. Returns number of
//   clauses rewritten. Updates weights of rewritten clauses.
//
// Global Variables: -
//
// Side Effects    : Changes clauses
//
/----------------------------------------------------------------------*/

long ClauseSetComputeLINormalform(OCB_p ocb, TB_p bank, ClauseSet_p
                                  set, ClauseSet_p *demodulators,
                                  RewriteLevel level, bool
                                  prefer_general)
{
   Clause_p handle;
   bool     tmp;
   long     res = 0;

   assert(demodulators);

   for(handle=set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      tmp = ClauseComputeLINormalform(ocb, bank,
                                      handle,
                                      demodulators,
                                      level,
                                      prefer_general);

      if(tmp)
      {
         handle->weight = ClauseStandardWeight(handle);
         res++;
      }
      /* assert(handle->weight == ClauseStandardWeight(handle)); */
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: FindRewritableClauses()
//
//   New version - find all clauses that are rewritable with
//   new_demod.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms, may add rewrite
//                   links.
//
/----------------------------------------------------------------------*/

bool FindRewritableClauses(OCB_p ocb, ClauseSet_p set,
                           PStack_p results, Clause_p
                           new_demod, SysDate nf_date)
{
   return find_rewritable_clauses(ocb, set, results, new_demod,
                                  nf_date);
}


/*-----------------------------------------------------------------------
//
// Function: FindRewritableClausesIndexed()
//
//   New version - find all clauses that are rewritable with
//   new_demod using the subterm index. Returns true if any rewritable
//   clause was found.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms, may add rewrite
//                   links.
//
/----------------------------------------------------------------------*/

long FindRewritableClausesIndexed(OCB_p ocb, SubtermIndex_p index,
                                  PStack_p stack, Clause_p new_demod,
                                  SysDate nf_date)
{
   long res;
   Eqn_p eqn = new_demod->literals;

   assert(ClauseIsDemodulator(new_demod));

   res = find_rewritable_clauses_indexed(new_demod,
                                         ocb, index,
                                         stack,
                                         eqn->lterm,
                                         eqn->rterm,
                                         EqnIsOriented(eqn),
                                         nf_date);
   if(!EqnIsOriented(eqn))
   {
      res += find_rewritable_clauses_indexed(new_demod,
                                             ocb, index,
                                             stack,
                                             eqn->rterm,
                                             eqn->lterm,
                                             false,
                                             nf_date);
   }
   /*printf("Found %ld rewritable clauses\n", res);
     {
     PStackPointer i;

     for(i=0; i< PStackGetSP(stack); i++)
     {
     ClausePrint(stdout, PStackElementP(stack, i), true);
     printf("\n");
     }
     printf("---\n");
     }*/
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
