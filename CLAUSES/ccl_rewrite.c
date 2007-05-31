/*-----------------------------------------------------------------------

File  : ccl_rewrite.c

Author: Stephan Schulz

Contents
 
  Rewriting. 

  Copyright 1998-2002 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue May 26 19:47:52 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "ccl_rewrite.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long RewriteAttempts = 0;
long RewriteSucesses = 0;
long RewriteUnboundVarFails = 0;
bool RewriteStrongRHSInst = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



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

static bool instance_is_rule(OCB_p ocb, Eqn_p demod, 
                             Term_p lside, Term_p rside, Subst_p subst)

{   
   if(RewriteStrongRHSInst)
   {
      SubstCompleteInstance(subst, rside, 
                            OCBDesignatedMinTerm(ocb, demod->bank)); 
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

/* static */ Term_p term_follow_top_RW_chain(Term_p term, RWDesc_p desc)
{
   assert(term);

   /* printf("Starting chain\n"); */
   while(TermIsTopRewritten(term))
   {
      assert(term);
      if(TermCellQueryProp(term, TPIsSOSRewritten))
      {
	 desc->sos_rewritten = true;
      }
      term = TermRWReplaceField(term);
      /* printf("Following chain\n"); */
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

   if(SubstComputeMatch(eqn->lterm, term, subst, TBTermEqual))
   {      
      if((EqnIsOriented(eqn) 
	  || instance_is_rule(ocb, eqn, eqn->lterm, eqn->rterm, subst)))
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
            rterm = TBInsertInstantiated(bank, eqn->rterm);
            TermAddRWLink(term, rterm, new_demod->ident, ClauseIsSOS(new_demod), res);
         }
      }
      SubstBacktrack(subst);
   }
   if(res!=RWSuccess && !EqnIsOriented(eqn))
   {
      if(SubstComputeMatch(eqn->rterm, term, subst, TBTermEqual))
      {
	 if(instance_is_rule(ocb, eqn, eqn->rterm, eqn->lterm, subst))
	    /* If instance is rule -> subst is no renaming! */
	 {
            assert(!SubstIsRenaming(subst));
            TermCellSetProp(term, TPIsRRewritable|TPIsRewritable);
	    res = RWAlwaysRewritable;
            
            rterm = TBInsertInstantiated(bank, eqn->lterm);
            TermAddRWLink(term, rterm, new_demod->ident, ClauseIsSOS(new_demod), res);
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
//   rewriting. We may loose a few cycle by not immediately adding a
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
   
   /* printf("term_is_rewritable()...\n"); */

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

   if(SysDateCompare(term->rw_data.nf_date[RewriteAdr(FullRewrite)], nf_date)
      == DateEqual)
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
   if(topres != RWNoRewritable)
   {
      /* Properties set in term_is_top_rewritable! */
      return true;
   }
   if(!TermCellIsAnyPropSet(term, 
                            TPIsRewritable|TPIsRRewritable|
                            TPIsRewritten|TermIsRRewritten))
   {
      assert(topres!=RWNoProperInstance);
      term->rw_data.nf_date[RewriteAdr(RuleRewrite)] =
         term->rw_data.nf_date[RewriteAdr(FullRewrite)] = nf_date;
   }
   /* printf("...term_is_rewritable() = false (no match)\n"); */
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

   printf("Checking clause %ld: ", clause->ident);
   ClausePrint(stdout, clause, true);
   printf("\n");
   printf("with demod clause %ld: ", new_demod->ident);
   ClausePrint(stdout, new_demod, true);
   printf("\n");

   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = eqn_has_rw_side(ocb, handle, new_demod, nf_date);
      if(tmp != NoSide)
      {
	 res = true;
      }
   }
   /* printf("Res: %d\n", res);  */
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

static ClausePos_p indexed_find_demodulator(OCB_p ocb, Term_p term,
					    SysDate date,
					    ClauseSet_p demodulators,
					    Subst_p subst,
					    bool prefer_general)
{
   Eqn_p       eqn;   
   ClausePos_p pos, res = NULL;
   bool restricted_rw = false /* TermCellQueryProp(term, TPRestricted) */;
   
   assert(!restricted_rw);
   assert(term);
   assert(demodulators);
   assert(demodulators->demod_index);
   /* assert(term->weight == TermWeight(term, DEFAULT_VWEIGHT, 
      DEFAULT_FWEIGHT));*/
   assert(!TermIsTopRewritten(term));

   RewriteAttempts++;   
   
   PDTreeSearchInit(demodulators->demod_index, term, date, prefer_general);
   
   while((pos = PDTreeFindNextDemodulator(demodulators->demod_index, subst)))
   {
      eqn = pos->literal;
      
      if((EqnIsOriented(eqn)&&
	  SysDateCompare(TermNFDate(term,RewriteAdr(FullRewrite)),
			 pos->clause->date)!=DateEarlier)
	 ||
	 (!EqnIsOriented(eqn)&&
	  SysDateCompare(TermNFDate(term,RewriteAdr(FullRewrite)),
			 pos->clause->date)!=DateEarlier))
      {
	 continue;
      }

      switch(pos->side)
      {
      case LeftSide:
	    if((EqnIsOriented(eqn) 
		|| instance_is_rule(ocb, eqn, eqn->lterm, eqn->rterm, subst))
	       &&
	       (!restricted_rw ||
		!SubstIsRenaming(subst)))
	    {
	       res = pos;
	    }
	    break;
      case RightSide:
	    assert(!EqnIsOriented(eqn));
	    if(instance_is_rule(ocb, eqn, eqn->rterm, eqn->lterm, subst)
	       /* &&
	        !restricted_rw */) 
	       /* Case SubstIsRenaming(subst) already eleminated in
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
				      demodulators, bool prefer_general)
{
   Subst_p     subst = SubstAlloc();
   ClausePos_p pos;
   Term_p      repl;
   
   assert(demodulators->demod_index);
   assert(term);
   assert(!TermIsVar(term));
   assert(!TermIsTopRewritten(term));

   pos = indexed_find_demodulator(ocb, term, date, demodulators,
				  subst, prefer_general);
   if(pos)
   {
      RewriteSucesses++;

      repl = ClausePosGetOtherSide(pos);      
      repl = TBInsertInstantiated(bank, repl);
      
      assert(pos->clause->ident);
      TermAddRWLink(term, repl, pos->clause->ident, ClauseIsSOS(pos->clause));
      term = repl;
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
					  prefer_general)
{
   unsigned int  i;
   Term_p res = term;

   assert(level);
   assert(!TermIsVar(term));
   assert(!TermIsTopRewritten(term));
   
   for(i=0; i<level; i++)
   {
      assert(demodulators[i]);      

      if(SysDateCompare(TermNFDate(term,level-1), demodulators[i]->date) ==
	 DateEarlier)
      {
	 res = rewrite_with_clause_set(ocb, bank, term,
				       TermNFDate(term,level-1),
				       demodulators[i],
				       prefer_general);
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
// Function: term_li_normalform()
//
//   Compute a leftmost-innermost normal form of term. This is more
//   tricky than expected, as supertems on the stack may change. I'll
//   start with a recursive version and see how it works. This uses
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

   /* printf("Term: ");
   TermPrint(stdout, term, desc->ocb->sig, DEREF_NEVER);
   printf("\n"); */
   
   if(desc->level == NoRewrite)
   {
      return term;
   }
   term = term_follow_top_RW_chain(term, desc);
   assert(!TermIsTopRewritten(term));
   
   /* printf("Term after top chain: ");
   TermPrint(stdout, term, desc->ocb->sig, DEREF_NEVER);
   printf("\n"); */
   
   if(!TermIsRewritten(term)&&
      SysDateCompare(term->rw_data.nf_date[desc->level-1],
		     desc->demod_date)!=DateEarlier)
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
      Term_p new_term = TermTopCopy(term);
      int  i;
      
      modified = false;      
      for(i=0; i<term->arity; i++)
      {	 
	 new_term->args[i] = term_li_normalform(desc, term->args[i]);
	 modified = modified || (new_term->args[i]!= term->args[i]);
      }
      if(modified)
      {
	 new_term = TBTermTopInsert(desc->bank, new_term);
	 assert(new_term!=term);
	 TermAddRWLink(term, new_term, REWRITE_AT_SUBTERM, false);	 
	 term = new_term;
      }
      else
      {
	 TermTopFree(new_term);
      }
      /* printf("After subterm rewriting: ");
      TermPrint(stdout, term, desc->ocb->sig, DEREF_NEVER);
      printf("\n"); */
      
      if(!TermIsVar(term))
      {
	 if(TermIsTopRewritten(term))
	 {
	    new_term = term_follow_top_RW_chain(term, desc);
	 }
	 else
	 {
	    rewrite_with_clause_setlist(desc->ocb, desc->bank,
					term, desc->demods,
					desc->level,
					desc->prefer_general);
	    new_term = term_follow_top_RW_chain(term, desc);
	 }
	 modified = (term!=new_term);
	 term = new_term;
	 /* printf("After top level step: ");
	 TermPrint(stdout, term, desc->ocb->sig, DEREF_NEVER);
	 printf("\n"); */
      }
   }
   /* printf("Assumed normal form: ");
   TermPrint(stdout, term, desc->ocb->sig, DEREF_NEVER);
   printf("\n"); */

   /* This is tricky! The term may be sub-top-level rewritten by a
      rule that had been eliminated by an equation, or by a rule that
      modified its right hand side (new loop!). So we may not find
      the full normal form here - the rule is gone, and we do not test
      for equations. Thus, we are not necessarily in full normal
      form. */
   if(!TermIsRewritten(term))
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

bool eqn_li_normalform(RWDesc_p desc, ClausePos_p pos)
{   
   Eqn_p  eqn = pos->literal;
   Term_p l_old = eqn->lterm, r_old = eqn->rterm;
   bool   restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) && EqnIsOriented(eqn);

   /* printf("Rewriting: ");
   TermPrint(stdout, eqn->lterm, eqn->bank->sig, DEREF_NEVER);
   printf("\n"); */
   eqn->lterm = term_li_normalform(desc, eqn->lterm, restricted_rw);
   if(l_old!=eqn->lterm)
   {
      EqnDelProp(eqn, EPMaxIsUpToDate);
      if(OutputLevel>=4)
      {
	 pos->side = LeftSide;
	 DocClauseRewriteDefault(pos, l_old);
      }
   }
   /* printf("Rewriting: ");
   TermPrint(stdout, eqn->rterm, eqn->bank->sig, DEREF_NEVER);
   printf("\n"); */
   eqn->rterm = term_li_normalform(desc, eqn->rterm, false);
   if(r_old!=eqn->rterm)
   {
      if(!EqnIsOriented(eqn))
      {
	 EqnDelProp(eqn, EPMaxIsUpToDate);
      }
      if(OutputLevel>=4)
      {
	 pos->side = RightSide;
	 DocClauseRewriteDefault(pos, r_old);
      }
   }
   return (l_old!=eqn->lterm)||(r_old!=eqn->rterm);
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
			     level, bool prefer_general)
{
   Term_p res;
   RWDesc_p desc = rw_desc_cell_alloc(ocb, bank, demodulators, level,
				      prefer_general);

   res = term_li_normalform(desc, term);
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
   bool tmp, res=false;
   RWDesc_p desc = rw_desc_cell_alloc(ocb, bank, demodulators, level,
				      prefer_general);
   ClausePosCell pos;
   
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
   for(handle = clause->literals; handle; handle=handle->next)
   {
      pos.literal = handle;
      tmp = eqn_li_normalform(desc, &pos);
      res = res || tmp;
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
      tmp = ClauseComputeLINormalform(ocb, bank, handle,
				      demodulators, level,
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
   /* Later: Use the index if it exists */
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


