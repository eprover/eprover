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
#include <cte_lambda.h>

typedef struct {
   PObjMap_p* rw_sys;
   TB_p bank;
} local_rw_args;


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

unsigned long RewriteAttempts        = 0;
unsigned long RewriteSuccesses       = 0;
unsigned long RewriteUnboundVarFails = 0;
unsigned long RewriteUncached        = 0;
unsigned long BWRWMatchAttempts      = 0;
unsigned long BWRWMatchSuccesses     = 0;
unsigned long BWRWRwSuccesses        = 0;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static Term_p term_li_normalform(RWDesc_p desc, Term_p term,
                                 bool restricted_rw, bool lambda_demod);

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

   if(TermIsFreeVar(term))
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
//   uncomparable, so a renaming is never a rule. This is tested
//   before free variables of rside are bound to the minimal term
//   (rewrite_strong_rhs_inst). Otherwise an unorientable equation
//   could rewrite a variant of its own side, and the cached rewrite
//   link would later let the equation rewrite (and delete) itself.
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
      if(SubstIsRenaming(subst))
      {
         return false;
      }
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
// Function: rw_record_step()
//
//   Record a rewrite step with demod that produced the subterm res at
//   the current position desc->path as demodulator, result and a copy
//   of the path (NULL if desc->path is NULL). The steps are recorded
//   when they are done, not reconstructed later from the rewrite
//   links: a link can be replaced by a link to another normal form
//   (rewriting with unorientable equations is not confluent), and a
//   chain through the old link then no longer reaches its end.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void rw_record_step(RWDesc_p desc, Clause_p demod, Term_p res)
{
   assert(desc->steps);
   assert(demod);

   PStackPushP(desc->steps, demod);
   PStackPushP(desc->steps, res);
   PStackPushP(desc->steps, desc->path?PStackCopy(desc->path):NULL);
}


/*-----------------------------------------------------------------------
//
// Function: rw_steps_reset()
//
//   Discard the recorded steps in desc.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void rw_steps_reset(RWDesc_p desc)
{
   PStackPointer i;
   PStack_p path;

   for(i=2; i<PStackGetSP(desc->steps); i+=3)
   {
      path = PStackElementP(desc->steps, i);
      if(path)
      {
         PStackFree(path);
      }
   }
   PStackReset(desc->steps);
}


/*-----------------------------------------------------------------------
//
// Function: term_follow_top_RW_chain()
//
//   Return the last term in an existing rewrite link chain, following
//   only top rewrite links. If one of those is induced by a SoS
//   clause, set desc->sos. If desc->steps is set, record each link
//   followed as a step, see rw_record_step().
//
// Global Variables: -
//
// Side Effects    : May change desc->sos_rewritten and desc->steps.
//
/----------------------------------------------------------------------*/

/* static */ Term_p term_follow_top_RW_chain(Term_p term, RWDesc_p desc,
                                             bool restricted_rw)
{
   assert(term);

   //printf("### Start\n");
   while(TermIsTopRewritten(term)&&(!restricted_rw||TermIsRRewritten(term)))
   {
      //printf("### Term: %p: ", term);
      //TermPrintSExpr(stdout, term, term->owner_bank->sig);
      //printf("\n");

      assert(term);
      if(TermCellQueryProp(term, TPIsSOSRewritten))
      {
         desc->sos_rewritten = true;
      }

      // assert(TOGreater(desc->ocb, term, TermRWReplaceField(term),
      //                   DEREF_NEVER, DEREF_NEVER));
      if(desc->steps)
      {
         rw_record_step(desc, TermRWDemod(term), TermRWReplaceField(term));
      }
      term = TermRWReplaceField(term);
      assert(term);
   }
   //printf("### Final: %p: ", term);
   //TermPrintSExpr(stdout, term, term->owner_bank->sig);
   //printf("\n");
   return term;
}


/*-----------------------------------------------------------------------
//
// Function: replace_term()
//
//   Replace all subterms stored in the rw_sys by their respective
//   associated partners.
//
// Global Variables: -
//
// Side Effects    : Instantiates (Really? StS)
//
/----------------------------------------------------------------------*/

Term_p replace_term(PObjMap_p* rw_sys, TB_p bank, Term_p t)
{
   Term_p s = PObjMapFind(rw_sys, t, PCmpFun);

   if(s && s!=t)
   {
      s = replace_term(rw_sys, bank, s);
   }
   else
   {
      bool changed = false;
      s = TermTopCopyWithoutArgs(t);
      for(int i=0; i < t->arity; i++)
      {
         s->args[i] = replace_term(rw_sys, bank, t->args[i]);
         assert(TermIsShared(s->args[i]) && s->args[i]->type == t->args[i]->type);
         changed = changed || (s->args[i] != t->args[i]);
      }

      if(changed)
      {
         s = TBTermTopInsert(bank, s);
      }
      else
      {
         TermTopFree(s);
         s = t;
      }
   }
   return s;
}

//helper function to adhere to TermMapper interface
Term_p local_rw_term(void* p_args, Term_p t)
{
   local_rw_args* args = p_args;
   return replace_term(args->rw_sys, args->bank, t);
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

ClausePos_p indexed_find_demodulator(OCB_p ocb, Term_p term,
                                    SysDate date,
                                    ClauseSet_p demodulators,
                                    Subst_p subst,
                                    bool prefer_general,
                                    bool restricted_rw)
{
   Eqn_p       eqn;
   ClausePos_p pos, res = NULL;

   assert(term);
   assert(demodulators);
   assert(demodulators->demod_index);
   assert(term->weight ==
            TermWeight(term, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(!TermIsTopRewritten(term));

   RewriteAttempts++;

   PDTreeSearchInit(demodulators->demod_index, term, date, prefer_general);

   while((pos = PDTreeFindNextDemodulator(demodulators->demod_index, subst)))
   {
      eqn = pos->literal;

      if(((EqnIsOriented(eqn)&&
          !SysDateIsEarlier(TermNFDate(term,RewriteAdr(RuleRewrite)),
                            pos->clause->date))
         ||
         (!EqnIsOriented(eqn)&&
          !SysDateIsEarlier(TermNFDate(term,RewriteAdr(FullRewrite)),
                            pos->clause->date))))
      {
         continue;
      }
      switch(pos->side)
      {
      case LeftSide:
            if((EqnIsOriented(eqn)
                || instance_is_rule(ocb, EqnGetBank(eqn), eqn->lterm, eqn->rterm, subst))
               &&
               (!restricted_rw ||
                !SubstIsRenaming(subst)))
            {
               res = pos;
            }
            break;
      case RightSide:
            assert(!EqnIsOriented(eqn));
            if(instance_is_rule(ocb, EqnGetBank(eqn), eqn->rterm, eqn->lterm, subst)
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
   }
   PDTreeSearchExit(demodulators->demod_index);

#ifndef NDEBUG
   if(res
      && problemType != PROBLEM_HO
      && !TermStructPrefixEqual(ClausePosGetSide(res), term, DEREF_ONCE, DEREF_NEVER,
                                0, ocb->sig))
   {
      fprintf(stderr, "Term ");
      TermPrintDbg(stderr, ClausePosGetSide(res), ocb->sig, DEREF_NEVER);
      fprintf(stderr, " derefed { ");
      TermPrintDbg(stderr, ClausePosGetSide(res), ocb->sig, DEREF_ONCE);
      fprintf(stderr, " } should match ");
      TermPrintDbg(stderr, term, ocb->sig, DEREF_NEVER);
      fprintf(stderr, ", substitution is : ");
      SubstPrint(stderr, subst, ocb->sig, DEREF_NEVER);
      fprintf(stderr, ".\n");

      assert(false);
   }
#endif
   return res;
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
   ClausePos_p pos;
   Term_p      repl;

   assert(demodulators->demod_index);
   assert(term);
   assert(!TermIsFreeVar(term));
   assert(!TermIsTopRewritten(term));

   pos = indexed_find_demodulator(ocb, term, date, demodulators,
                                  subst, prefer_general, restricted_rw);
   if(pos)
   {
      RewriteSuccesses++;

      repl = TBInsertInstantiated(bank, ClausePosGetOtherSide(pos));

      if(problemType == PROBLEM_HO)
      {
         repl = MakeRewrittenTerm(term, repl, 0, bank);
      }

      if(repl == term)
      {
         SubstDelete(subst);
         return term;
      }

      assert(pos->clause->ident);
      TermAddRWLink(term, repl, pos->clause, ClauseIsSOS(pos->clause),
                    restricted_rw?RWAlwaysRewritable:RWLimitedRewritable);
      RewriteUncached++;
      // assert(TOGreater(ocb, term, repl, DEREF_NEVER, DEREF_NEVER));
      // The assertion is logically true, but in practice LPO fails on
      // too deep terms, so l > r, but sigma(l) !> sigma(r) w.r.t. the
      // implementation

      term = repl;
   }
   SubstDelete(subst);

   return term;
}



/*-----------------------------------------------------------------------
//
// Function: rewrite_with_clause_set_list()
//
//   Rewrite a term at top level with the sets of
//   demodulators. Returns new term.
//
// Global Variables:
//
// Side Effects    : Only by rewrite_with_clause_set.
//
/----------------------------------------------------------------------*/

static Term_p rewrite_with_clause_set_list(OCB_p ocb, TB_p bank, Term_p term,
                                          ClauseSet_p* demodulators,
                                          RewriteLevel level, bool
                                          prefer_general,
                                          bool restricted_rw)
{
   unsigned int  i;
   Term_p res = term;

   assert(level);
   assert(!TermIsFreeVar(term));
   assert(!TermIsTopRewritten(term));

   for(i=0; i<level; i++)
   {
      assert(demodulators[i]);

      if(TermIsDBClosed(term) &&
         SysDateIsEarlier(TermNFDate(term,level-1), demodulators[i]->date))
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

static bool term_subterm_rewrite(RWDesc_p desc, Term_p *term, bool lambda_demod)
{
   bool modified = false;
   Term_p new_term = TermTopCopyWithoutArgs(*term);
   int  i;

   if(!lambda_demod && TermIsLambda(*term))
   {
      TermTopFree(new_term);
      return false;
   }

   for(i=0; i<(*term)->arity; i++)
   {
      if(desc->path)
      {
         PStackPushInt(desc->path, i);
      }
      new_term->args[i] = term_li_normalform(desc, (*term)->args[i], false, lambda_demod);
      if(desc->path)
      {
         PStackDiscardTop(desc->path);
      }
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
                                 bool restricted_rw,
                                 bool lambda_demod)
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
   if(TermIsFreeVar(term))
   {
      assert(!TermIsRewritten(term));
      return term;
   }
   while(modified)
   {
      modified = term_subterm_rewrite(desc, &term, lambda_demod);

      if(!TermIsFreeVar(term))
      {
         if(TermIsTopRewritten(term))
         {
            new_term = term_follow_top_RW_chain(term, desc, restricted_rw&&(!modified));
         }
         else
         {
            rewrite_with_clause_set_list(desc->ocb, desc->bank,
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
// Function: record_rw_steps()
//
//   Set the designated side of the literal at pos to its normal
//   form nf and record the rewrite steps that led there (collected in
//   desc->steps while nf was computed) in the derivation of the
//   clause. For a full proof object, each step becomes a separate
//   inference, see ClauseArchiveSteps().
//
// Global Variables: -
//
// Side Effects    : Changes the literal and the derivation
//
/----------------------------------------------------------------------*/

static void record_rw_steps(RWDesc_p desc, ClausePos_p pos, Term_p nf)
{
   Clause_p clause = pos->clause;
   Eqn_p eqn = pos->literal;
   Term_p *side = pos->side == LeftSide?&(eqn->lterm):&(eqn->rterm);

   assert(PStackGetSP(desc->steps));
   desc->rw_steps += PStackGetSP(desc->steps)/3;
   ClauseArchiveSteps(desc->step_archive, desc->step_sides, clause,
                      side, nf, desc->steps, DCRewrite);
   PStackReset(desc->steps);
}


/*-----------------------------------------------------------------------
//
// Function: eqn_li_normalform()
//
//   Compute the normal form of maximal, minimal or both terms in an
//   equation. Return rewritten sides (truth value is true if any side
//   was rewritten).
//
// Global Variables: -
//
// Side Effects    : Changes terms and term bank
//
/----------------------------------------------------------------------*/

EqnSide eqn_li_normalform(RWDesc_p desc, ClausePos_p pos,
                          bool interred_rw, bool lambda_demod)
{
   Eqn_p  eqn = pos->literal;
   Term_p l_old = eqn->lterm, r_old = eqn->rterm, nf;
   bool   restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) &&
      EqnIsOriented(eqn) && interred_rw;
   EqnSide res = NoSide;

   assert(desc->steps);
   rw_steps_reset(desc);
   nf = term_li_normalform(desc, eqn->lterm, restricted_rw, lambda_demod);
   if(l_old!=nf)
   {
      EqnDelProp(eqn, EPMaxIsUpToDate);
      res = MaxSide;
      pos->side = LeftSide;
      record_rw_steps(desc, pos, nf);
      if(OutputLevel>=4)
      {
         DocClauseRewriteDefault(pos, l_old);
      }
   }
   rw_steps_reset(desc);
   nf = term_li_normalform(desc, eqn->rterm, false, lambda_demod);
   if(r_old!=nf)
   {
      pos->side = RightSide;
      record_rw_steps(desc, pos, nf);
      if(EqnQueryProp(eqn, EPIsEquLiteral) && eqn->rterm == EqnGetBank(eqn)->true_term)
      {
         EqnDelProp(eqn, EPIsEquLiteral);
      }
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
      if(OutputLevel>=4)
      {
         DocClauseRewriteDefault(pos, r_old);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: rw_desc_cell_alloc()
//
//   Allocate an initialized RWDescCell. A full proof object keeps
//   the clause before each step in step_archive, and step_sides
//   records the sides of these copies. Both may be NULL.
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
                                              prefer_general,
                                              ClauseSet_p step_archive,
                                              PStack_p step_sides)
{
   RWDesc_p desc = RWDescCellAlloc();

   desc->ocb            = ocb;
   desc->bank           = bank;
   desc->demods         = demodulators;
   desc->demod_date     = ClauseSetListGetMaxDate(demodulators, level);
   desc->level          = level;
   desc->prefer_general = prefer_general;
   desc->step_archive   = step_archive;
   desc->step_sides     = step_sides;
   desc->steps          = NULL;
   desc->path           = NULL;
   desc->rw_steps       = 0;
   desc->sos_rewritten  = false;

   return desc;
}


/*-----------------------------------------------------------------------
//
// Function: rw_desc_cell_free()
//
//   Free an RWDescCell and its step stacks.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void rw_desc_cell_free(RWDesc_p desc)
{
   if(desc->steps)
   {
      rw_steps_reset(desc);
      PStackFree(desc->steps);
   }
   if(desc->path)
   {
      PStackFree(desc->path);
   }
   RWDescCellFree(desc);
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

long clause_tree_push(PStack_p stack, OrdSet_p clausetree)
{
   OrdSetIter_p iterstack;
   OrdSet_p  cell;
   Clause_p clause;
   long     res = 0;

   iterstack = OrdSetTraverseInit(clausetree);

   while((cell = OrdSetTraverseNext(iterstack)))
   {
      clause = cell->key;
      if(!ClauseQueryProp(clause, CPRWDetected))
      {
         ClauseSetProp(clause, CPRWDetected);
         PStackPushP(stack, clause);
         res++;
      }
   }
   OrdSetTraverseExit(iterstack);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: term_bw_rw_check()
//
//   Check if term is rewritable with the demodulator demod, used in
//   the direction lterm->rterm (oriented tells if this direction is
//   known to be ordered). Return RWAlwaysRewritable if it is,
//   RWLimitedRewritable if it is only rewritable if not at the top
//   of a maximal side of a positive oriented literal (the step would
//   be a renaming), RWNotRewritable otherwise.
//
// Global Variables: -
//
// Side Effects    : Sets properties and rewrite links in term.
//
/----------------------------------------------------------------------*/

static RWResultType term_bw_rw_check(Clause_p demod,
                                     OCB_p ocb,
                                     Term_p term,
                                     Term_p lterm,
                                     Term_p rterm,
                                     bool oriented)
{
   Eqn_p eqn = demod->literals;
   RWResultType rwres = RWNotRewritable;
   Subst_p subst = SubstAlloc();

   assert(!TermIsFreeVar(term));

   BWRWMatchAttempts++;
   int remains = 0;
   if(SubstMatchComplete(lterm, term, subst))
   {
      BWRWMatchSuccesses++;
      if(oriented
         || instance_is_rule(ocb, EqnGetBank(eqn), lterm, rterm, subst))
      {
         if(!oriented || /* Only a performance hack */
            !SubstIsRenaming(subst))
         {
            TermCellSetProp(term, TPIsRRewritable|TPIsRewritable);
            rwres = RWAlwaysRewritable;
         }
         else
         {
            TermCellSetProp(term, TPIsRewritable);
            rwres = RWLimitedRewritable;
         }
         if(!TermIsRewritten(term) || (rwres == RWAlwaysRewritable))
         {
            Term_p repl = TBInsertInstantiated(EqnGetBank(eqn), rterm);
            if(problemType == PROBLEM_HO)
            {
               repl = MakeRewrittenTerm(term, repl, remains, EqnGetBank(eqn));
               if(remains)
               {
                  repl = TBTermTopInsert(EqnGetBank(eqn), repl);
               }
            }
            if(repl == term)
            {
               TermCellDelProp(term, TPIsRRewritable|TPIsRewritable);
               rwres = RWNotRewritable;
            }
            else
            {
               TermAddRWLink(term, repl, demod, ClauseIsSOS(demod), rwres);
               RewriteUncached++;
               //TermDeleteRWLink(term);
            }
         }
      }
      /* We cannot set the NF date here, since we have no indication
         of the state of the subterms. */
   }
   SubstDelete(subst);
   return rwres;
}


/*-----------------------------------------------------------------------
//
// Function: push_matchable_occs()
//
//   Push the subterm occurrences of all terms in index that may be
//   instances of pattern onto occs.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void push_matchable_occs(SubtermIndex_p index, Term_p pattern,
                                PStack_p occs)
{
   PStack_p      termtrees = PStackAlloc();
   PStack_p      iterstack;
   SubtermTree_p cell;

   FPIndexFindMatchable(index, pattern, termtrees);
   while(!PStackEmpty(termtrees))
   {
      iterstack = PTreeTraverseInit(PStackPopP(termtrees));
      while((cell = PTreeTraverseNext(iterstack)))
      {
         PStackPushP(occs, cell->key);
      }
      PTreeTraverseExit(iterstack);
   }
   PStackFree(termtrees);
}


/*-----------------------------------------------------------------------
//
// Function: term_bw_rw_status()
//
//   Return the backward-rewriting status of term with respect to
//   demod (see term_bw_rw_check()), trying both directions of an
//   unoriented demodulator.
//
// Global Variables: -
//
// Side Effects    : Via term_bw_rw_check()
//
/----------------------------------------------------------------------*/

static RWResultType term_bw_rw_status(Clause_p demod, OCB_p ocb,
                                      Term_p term)
{
   Eqn_p        eqn = demod->literals;
   RWResultType res, tmp;

   res = term_bw_rw_check(demod, ocb, term, eqn->lterm, eqn->rterm,
                          EqnIsOriented(eqn));
   if(!EqnIsOriented(eqn))
   {
      tmp = term_bw_rw_check(demod, ocb, term, eqn->rterm, eqn->lterm,
                             false);
      res = MAX(res, tmp);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: find_rewritable_clauses_indexed()
//
//   Push all clauses in index that are rewritable with demod onto
//   stack (in both directions if demod is unoriented). Return the
//   number of clauses pushed.
//
//   The candidate terms are checked in order of their entry numbers,
//   each in both directions, exactly as FindRewritableClauses() does.
//   Since term_bw_rw_check() creates the replacement terms, this
//   makes the creation order (and hence term entry numbers) the same
//   for all index types.
//
// Global Variables: -
//
// Side Effects    : Sets properties and links in affected terms and
//                   clauses.
//
/----------------------------------------------------------------------*/

static long find_rewritable_clauses_indexed(Clause_p demod,
                                            OCB_p ocb,
                                            SubtermIndex_p index,
                                            PStack_p stack)
{
   long          res = 0;
   Eqn_p         eqn = demod->literals;
   PStack_p      occs = PStackAlloc();
   PStackPointer i;
   SubtermOcc_p  occ, prev = NULL;

   push_matchable_occs(index, eqn->lterm, occs);
   if(!EqnIsOriented(eqn))
   {
      push_matchable_occs(index, eqn->rterm, occs);
   }
   PStackSort(occs, CmpSubtermOccRefs);

   for(i=0; i<PStackGetSP(occs); i++)
   {
      occ = PStackElementP(occs, i);
      if(occ == prev)
      {
         continue; /* Candidate for both directions */
      }
      prev = occ;
      switch(term_bw_rw_status(demod, ocb, occ->term))
      {
      case RWAlwaysRewritable:
            res += clause_tree_push(stack, occ->pl.occs.rw_full);
            res += clause_tree_push(stack, occ->pl.occs.rw_rest);
            break;
      case RWLimitedRewritable:
            res += clause_tree_push(stack, occ->pl.occs.rw_full);
            break;
      default:
            break;
      }
   }
   PStackFree(occs);
   return res;
}


/* Encoding of the per-term cache entries of FindRewritableClauses():
   0 is "not yet seen", otherwise BW_VISITED | status. */

#define BW_VISITED 4
#define BW_STATUS  3

/*-----------------------------------------------------------------------
//
// Function: collect_new_bw_candidates()
//
//   Push all terms in set that are not yet in cache onto terms, and
//   mark them in cache (indexed by entry number).
//
// Global Variables: -
//
// Side Effects    : Extends cache
//
/----------------------------------------------------------------------*/

static void collect_new_bw_candidates(OrdSet_p set, PDArray_p cache,
                                      PStack_p terms)
{
   OrdSetIter_p iter = OrdSetTraverseInit(set);
   OrdSet_p     cell;
   Term_p       term;

   while((cell = OrdSetTraverseNext(iter)))
   {
      term = cell->key;
      if(!PDArrayElementInt(cache, term->entry_no))
      {
         PDArrayAssignInt(cache, term->entry_no, BW_VISITED);
         PStackPushP(terms, term);
      }
   }
   OrdSetTraverseExit(iter);
}


/*-----------------------------------------------------------------------
//
// Function: cmp_term_entry()
//
//   Compare two PStack elements holding terms by entry number (for
//   PStackSort()).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int cmp_term_entry(const void *e1, const void *e2)
{
   Term_p t1 = ((const IntOrP*)e1)->p_val;
   Term_p t2 = ((const IntOrP*)e2)->p_val;

   return CMP(t1->entry_no, t2->entry_no);
}


/*-----------------------------------------------------------------------
//
// Function: set_has_bw_status()
//
//   Return true if a term in set has a rewriting status (from cache)
//   of at least min_status.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool set_has_bw_status(OrdSet_p set, PDArray_p cache,
                              RWResultType min_status)
{
   OrdSetIter_p iter = OrdSetTraverseInit(set);
   OrdSet_p     cell;
   bool         res = false;

   while((cell = OrdSetTraverseNext(iter)))
   {
      if((PDArrayElementInt(cache, ((Term_p)cell->key)->entry_no)
          & BW_STATUS) >= min_status)
      {
         res = true;
         break;
      }
   }
   OrdSetTraverseExit(iter);
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
                               bool restricted_rw, bool lambda_demod)
{
   Term_p res;
   RWDesc_p desc = rw_desc_cell_alloc(ocb, bank, demodulators, level,
                                      prefer_general, NULL, NULL);

   res = term_li_normalform(desc, term, restricted_rw, lambda_demod);
   rw_desc_cell_free(desc);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseComputeLINormalform()
//
//   Compute the normal form of terms in a clause. Return number of
//   rewrite steps performed. A full proof object keeps the clause
//   before each step in step_archive and its sides in step_sides.
//
// Global Variables: -
//
// Side Effects    : Changes terms and term bank
//
/----------------------------------------------------------------------*/

long ClauseComputeLINormalform(OCB_p ocb, TB_p bank, Clause_p clause,
                               ClauseSet_p *demodulators, RewriteLevel level,
                               bool prefer_general, bool lambda_demod,
                               ClauseSet_p step_archive, PStack_p step_sides)
{
   Eqn_p handle;
   EqnSide tmp = NoSide;
   long res = 0;
   RWDesc_p desc = rw_desc_cell_alloc(ocb, bank, demodulators, level,
                                      prefer_general, step_archive,
                                      step_sides);
   ClausePosCell pos;
   bool done = false;

   desc->steps = PStackAlloc();
   if(step_archive && PrintProofObject >= POSingleStepDerivation)
   {
      desc->path = PStackAlloc();
   }

   assert(demodulators);
   assert(desc->demods);
   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));

   /* printf(COMCHAR" ClauseComputeLINormalform(%ld)...\n",clause->ident); */
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
         tmp = eqn_li_normalform(desc, &pos,
                                 ClauseQueryProp(clause,CPLimitedRW),
                                 lambda_demod);
         if((tmp&MaxSide)
            && EqnIsPositive(handle)
            && EqnIsMaximal(handle)
            && ClauseQueryProp(clause,CPLimitedRW))
         {
            ClauseDelProp(clause,CPLimitedRW);
            /* We need to try everything again...*/
            done = false;
         }
      }
   }
   if(desc->sos_rewritten)
   {
      ClauseSetProp(clause, CPIsSOS);
   }
   res = desc->rw_steps;

   if(res)
   {
      ClauseDelProp(clause, CPInitial);
   }
   rw_desc_cell_free(desc);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetComputeLINormalform()
//
//   Compute a normal form for terms in all clauses in set with
//   respect to clauses in demodulators up to level. Returns number of
//   rewrite steps. Updates weights of rewritten clauses. A full proof
//   object keeps each clause before each step in step_archive and its
//   sides in step_sides.
//
// Global Variables: -
//
// Side Effects    : Changes clauses
//
/----------------------------------------------------------------------*/

long ClauseSetComputeLINormalform(OCB_p ocb, TB_p bank, ClauseSet_p
                                  set, ClauseSet_p *demodulators,
                                  RewriteLevel level, bool
                                  prefer_general, bool lambda_demod,
                                  ClauseSet_p step_archive,
                                  PStack_p step_sides)
{
   Clause_p handle;
   long     tmp;
   long     res = 0;

   assert(demodulators);

   for(handle=set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      tmp = ClauseComputeLINormalform(ocb, bank,
                                      handle,
                                      demodulators,
                                      level,
                                      prefer_general,
                                      lambda_demod,
                                      step_archive,
                                      step_sides);

      if(tmp)
      {
         handle->weight = ClauseStandardWeight(handle);
      }
      res+=tmp;
      /* assert(handle->weight == ClauseStandardWeight(handle)); */
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: FindRewritableClauses()
//
//   Find all clauses in the NULL-terminated array sets that are
//   rewritable with new_demod and push them onto results. Return
//   true if there is at least one. This scans all clauses, but checks
//   exactly the subterms the backward-rewriting index would hold
//   (ClauseCollectIdxSubterms()), in the same order and with the same
//   side effects on the terms as FindRewritableClausesIndexed(), so
//   that the search does not depend on whether an index is used.
//
// Global Variables: -
//
// Side Effects    : Sets properties and rewrite links in terms.
//
/----------------------------------------------------------------------*/

bool FindRewritableClauses(OCB_p ocb, ClauseSet_p *sets,
                           PStack_p results, Clause_p new_demod,
                           bool lambda_demod, bool legacy_depth)
{
   Clause_p      handle;
   PDArray_p     cache = PDArrayAlloc(1024, 0);
   PStack_p      terms = PStackAlloc();
   PStack_p      csets = PStackAlloc();
   ClauseSet_p   *set;
   PStackPointer i;
   Term_p        term;
   OrdSet_p      rest, full;
   bool          res = false;

   assert(ClauseIsDemodulator(new_demod));

   for(set = sets; *set; set++)
   {
      for(handle = (*set)->anchor->succ; handle != (*set)->anchor;
          handle = handle->succ)
      {
         rest = NULL;
         full = NULL;
         ClauseCollectIdxSubterms(handle, &rest, &full, lambda_demod,
                                  legacy_depth);
         collect_new_bw_candidates(rest, cache, terms);
         collect_new_bw_candidates(full, cache, terms);
         PStackPushP(csets, rest);
         PStackPushP(csets, full);
      }
   }
   /* Check in the same order as find_rewritable_clauses_indexed() */
   PStackSort(terms, cmp_term_entry);
   for(i=0; i<PStackGetSP(terms); i++)
   {
      term = PStackElementP(terms, i);
      PDArrayAssignInt(cache, term->entry_no,
                       BW_VISITED|term_bw_rw_status(new_demod, ocb, term));
   }
   PStackFree(terms);

   i = 0;
   for(set = sets; *set; set++)
   {
      for(handle = (*set)->anchor->succ; handle != (*set)->anchor;
          handle = handle->succ)
      {
         rest = PStackElementP(csets, i++);
         full = PStackElementP(csets, i++);
         /* As in the index: restricted terms need a full rewrite,
            all others any rewrite. */
         if(set_has_bw_status(rest, cache, RWAlwaysRewritable) ||
            set_has_bw_status(full, cache, RWLimitedRewritable))
         {
            PStackPushP(results, handle);
            res = true;
         }
         OrdSetFree(rest);
         OrdSetFree(full);
      }
   }
   PStackFree(csets);
   PDArrayFree(cache);
   return res;
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
   assert(ClauseIsDemodulator(new_demod));

   return find_rewritable_clauses_indexed(new_demod, ocb, index, stack);
}

/*-----------------------------------------------------------------------
//
// Function: ClauseLocalRW()
//
//   Find negative literals s != t such that s > t and replace all
//   occurrences of s with t in the clause. A full proof object keeps
//   the clause before the step in step_archive.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms, may add rewrite
//                   links.
//
/----------------------------------------------------------------------*/

bool ClauseLocalRW(OCB_p ocb, Clause_p clause, ClauseSet_p step_archive)
{
   PObjMap_p rw_sys = NULL;

   ClauseOrientLiterals(ocb, clause);
   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      if(EqnIsNegative(lit) && EqnIsOriented(lit))
      {
         PObjMapStore(&rw_sys, lit->lterm, lit->rterm, PCmpFun);
      }
      else if(!EqnIsEquLit(lit) && EqnIsPositive(lit))
      {
         assert(lit->rterm == EqnGetBank(lit)->true_term);
         PObjMapStore(&rw_sys, lit->lterm, EqnGetBank(lit)->false_term, PCmpFun);
      }
   }
   /* if(rw_sys) */
   /* { */
   /*    printf("Clause: "); */
   /*    ClausePrint(stdout, clause, true); */
   /*    printf("\n"); */
   /* } */
   bool modified = false;
   Clause_p before = ClauseArchiveStepBegin(step_archive, clause);
   for(Eqn_p lit = clause->literals; lit; lit = lit->next)
   {
      if(!(EqnIsNegative(lit) && EqnIsOriented(lit))
          && !(!EqnIsEquLit(lit) && EqnIsPositive(lit)))
      {
         Term_p lterm = lit->lterm, rterm = lit->rterm;
         local_rw_args arg = {.rw_sys = &rw_sys, .bank = EqnGetBank(lit) };
         EqnMap(lit, local_rw_term, &arg);
         if(lterm != lit->lterm || rterm != lit->rterm)
         {
            modified = true;
         }
      }
   }

   ClauseArchiveStepEnd(step_archive, clause, before, modified);
   if(modified)
   {
      ClauseRecomputeLitCounts(clause);
      ClauseRemoveSuperfluousLiterals(clause, step_archive);
      ClauseDelProp(clause, CPIsOriented);
      ClausePushDerivation(clause, DCLocalRewrite);
   }


   PObjMapFree(rw_sys);
   return modified;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
