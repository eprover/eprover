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
/* long RewriteTermDepth = 0; */

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

static bool instance_is_rule(OCB_p ocb, Term_p lside, Term_p rside,
			     Subst_p subst)

{   
   if(TermHasUnboundVariables(rside))
   {
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

static Term_p term_follow_top_RW_chain(Term_p term, RWDesc_p desc)
{
   assert(term);

   /* printf("Starting chain\n"); */
   while(TermIsTopRewritten(term))
   {
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

static bool term_is_top_rewritable(OCB_p ocb, Term_p term, Clause_p
				   new_demod)
{
   Subst_p subst = SubstAlloc();
   Eqn_p   eqn;
   bool    res = false;
   
   assert(new_demod->pos_lit_no == 1);
   assert(new_demod->neg_lit_no == 0);
   assert(!TermIsVar(term));
   
   eqn = new_demod->literals;

   DEBUGMARK(RW_MATCH_WATCH, "Matching ");
   DEBUG(RW_MATCH_WATCH, 
	 ClausePrint(stdout, new_demod, true););
   DEBUGOUT(RW_MATCH_WATCH, " (l) onto ");
   DEBUG(RW_MATCH_WATCH, 
	 TermPrint(stdout, term, ocb->sig, DEREF_NEVER););
   DEBUGOUT(RW_MATCH_WATCH, "\nInstantiated lside: ");
   DEBUG(RW_MATCH_WATCH,
	 TermPrint(stdout, eqn->lterm, ocb->sig, DEREF_ONCE););
   DEBUGOUT(RW_MATCH_WATCH, "\n");
   if(SubstComputeMatch(eqn->lterm, term, subst, TBTermEqual))
   {
      if((EqnIsOriented(eqn) 
	  || instance_is_rule(ocb, eqn->lterm, eqn->rterm, subst))
	 &&
	 (!TermCellQueryProp(term, TPRestricted) ||
	  !SubstIsRenaming(subst)))
      {
	 res = true;
      }
      SubstBacktrack(subst);
   }
   if(!res && !EqnIsOriented(eqn))
   {
      if(SubstComputeMatch(eqn->rterm, term, subst, TBTermEqual))
      {
	 if(instance_is_rule(ocb, eqn->rterm, eqn->lterm, subst))
	    /* If instance is rule -> subst is no renaming! */
	 {
	    res = true;
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

static bool term_is_rewritable(OCB_p ocb, Term_p term, Clause_p
                               new_demod, SysDate nf_date)
{
   int i;
   bool res = false;

   /* printf("term_is_rewritable()...\n"); */

   if(TermIsVar(term))
   {
      return false;
   }
   if(TermCellQueryProp(term, TPIsRewritable)||TermIsRewritten(term))
   {
      return true;
   }

   assert(!TermIsRewritten(term));

   if(SysDateCompare(term->rw_data.nf_date[RewriteAdr(FullRewrite)], nf_date) == DateEqual)
   {
      return false;
   }
   for(i=0; i<term->arity; i++)
   {
      if(term_is_rewritable(ocb, term->args[i], new_demod, nf_date))
      {
         res = true;
         break;
      }
   }
   if(res)
   {
      TermCellSetProp(term, TPIsRewritable);
      return true;
   }
   if(term_is_top_rewritable(ocb, term, new_demod))
   {
      TermCellSetProp(term, TPIsRewritable);
      return true;
   }
   term->rw_data.nf_date[RewriteAdr(RuleRewrite)] =
      term->rw_data.nf_date[RewriteAdr(FullRewrite)] = nf_date;
   
   /* printf("...term_is_rewritable() = false (no match)\n"); */
   return false;
}



/*-----------------------------------------------------------------------
//
// Function: eqn_has_rw_side()
//
//   Return NoSide, MaxSide, MinSide depending on wether
//   eqn has or has not a rewritable (maximal) side.
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

   resl = term_is_rewritable(ocb, eqn->lterm, new_demod, nf_date);
   resr = term_is_rewritable(ocb, eqn->rterm, new_demod, nf_date);

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

   /* printf("Checking clause %ld: ", clause->ident);
      ClausePrint(stdout, clause, false);
   printf("\n"); */
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
// Function: clause_has_rw_max_side()
//
//   Return MaxSide, if the clause has a rewritable  maximal literal that
//   may become non-maximal, MinSide if this is not the case, but it
//   has another rewritable, NoSide otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms.
//
/----------------------------------------------------------------------*/

static EqnSide clause_has_rw_max_side(OCB_p ocb, Clause_p clause,
				   Clause_p new_demod, SysDate
				   nf_date)
{
   Eqn_p handle;
   EqnSide tmp;
   bool maxres = false, minres = false;

   /* printf("Checking clause %ld: ", clause->ident);
      ClausePrint(stdout, clause, false);
   printf("\n"); */
   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = eqn_has_rw_side(ocb, handle, new_demod, nf_date);
      if(tmp != NoSide)
      {
	 if(EqnIsMaximal(handle))
	 {	
	    if((tmp == MaxSide)||
	       (EqnIsPositive(handle) && (clause->pos_lit_no >=2))||
	       (EqnIsNegative(handle) && (clause->neg_lit_no >=2)))
	    {	  
	       maxres = true;
	    }
	    else
	    {
	       minres = true;
	    }
	 }
	 else
	 {
	    minres = true;
	 }
      }	 
   }
   if(maxres)
   {
      return MaxSide;
   }
   if(minres)
   {
      return MinSide;
   }
   return NoSide;
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
// Function: find_clauses_with_rw_max_sides()
//
//   A non-index-using implementation of
//   FindClausesWithRewritableMaxSides(). Return true if any clause
//   has a rewritable non-maximal side.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms.
//
/----------------------------------------------------------------------*/

static bool find_clauses_with_rw_max_sides(OCB_p ocb, ClauseSet_p set,
					   PStack_p results, Clause_p
					   new_demod, SysDate nf_date)
{
   Clause_p handle;
   EqnSide  tmp;
   bool     res = false;
   
   assert(new_demod->pos_lit_no == 1);
   assert(new_demod->neg_lit_no == 0);
   
   for(handle = set->anchor->succ; handle != set->anchor; handle =
	  handle->succ)
   {
      tmp = clause_has_rw_max_side(ocb, handle, new_demod, nf_date);
      assert(tmp!=BothSides);
      if(tmp==MaxSide)
      /* if(tmp!=NoSide) */
      {
	 PStackPushP(results, handle);
      }
      res = res || (tmp==MinSide);
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
   
   DEBUGMARK(RW_INTERFACE_WATCH2,
	     "TermIndexedFindDemodulator()...\n");
   
   assert(term);
   assert(demodulators);
   assert(demodulators->demod_index);
   /* assert(term->weight == TermWeight(term, DEFAULT_VWEIGHT, 
      DEFAULT_FWEIGHT));*/
   assert(!TermIsTopRewritten(term));

   RewriteAttempts++;   
   
   DEBUGMARK(RW_MATCH_WATCH, "\nSearching match for: ");
   DEBUG(RW_MATCH_WATCH, TermPrint(stdout, term, ocb->sig,
				   DEREF_NEVER);
	 printf("\n"););
   
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
	    DEBUGMARK(RW_MATCH_WATCH, "Found: ");
	    DEBUG(RW_MATCH_WATCH, 
		  ClausePrint(stdout, pos->clause, true);
		  printf(" (l), instantiated: ");
		  TermPrint(stdout, pos->clause->literals->lterm,
			    ocb->sig, DEREF_ONCE);
		  printf(" <=> ");
		  TermPrint(stdout, pos->clause->literals->rterm,
			    ocb->sig, DEREF_ONCE);
		  printf("\n");
	       );
	    if((EqnIsOriented(eqn) 
		|| instance_is_rule(ocb, eqn->lterm, eqn->rterm, subst))
	       &&
	       (!TermCellQueryProp(term, TPRestricted) ||
		!SubstIsRenaming(subst)))
	    {
	       res = pos;
	    }
	    break;
      case RightSide:
	    DEBUGMARK(RW_MATCH_WATCH, "Found: ");
	    DEBUG(RW_MATCH_WATCH, 
		  ClausePrint(stdout, pos->clause, true);
		  printf(" (r), instantiated: ");
		  TermPrint(stdout, pos->clause->literals->lterm,
			    ocb->sig, DEREF_ONCE);
		  printf(" <=> ");
		  TermPrint(stdout, pos->clause->literals->rterm,
			    ocb->sig, DEREF_ONCE);
		  printf("\n");
	       );
	    assert(!EqnIsOriented(eqn));
	    if(instance_is_rule(ocb, eqn->rterm, eqn->lterm, subst)
	       &&
	       !TermCellQueryProp(term, TPRestricted)) 
	       /* Case SubstIsRenaming(subst) already eleminated in
		  instance_is_rule! */
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

   DEBUGMARK(RW_INTERFACE_WATCH2,
	     "...TermIndexedFindDemodulator()\n");
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
// Function: term_LI_normalform()
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

static Term_p term_li_normalform(RWDesc_p desc, Term_p term)
{
   bool    modified = true;

   DEBUGMARK(RW_INTERFACE_WATCH2, "TermComputeLINormalform()...\n");   
   /* printf("Term: ");
   TermPrint(stdout, term, desc->ocb->sig, DEREF_NEVER);
   printf("\n"); */
   
   if(desc->level == NoRewrite)
   {
      DEBUGMARK(RW_INTERFACE_WATCH2, "...TermComputeLINormalform() (RW-Level = 0)\n");  
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
      DEBUGMARK(RW_INTERFACE_WATCH2, "...TermComputeLINormalform() (Used dates)\n");  
      return term;
   }      
   if(TermIsVar(term))
   {
      DEBUGMARK(RW_INTERFACE_WATCH2, "...TermComputeLINormalform() (Variable)\n");        
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
	 new_term = TBTermtopInsert(desc->bank, new_term);
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
   DEBUGMARK(RW_INTERFACE_WATCH2, "...TermComputeLINormalform()\n");   
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
   Eqn_p eqn = pos->literal;
   Term_p l_old=eqn->lterm, r_old=eqn->rterm;

   /* printf("Rewriting: ");
   TermPrint(stdout, eqn->lterm, eqn->bank->sig, DEREF_NEVER);
   printf("\n"); */
   eqn->lterm = term_li_normalform(desc, eqn->lterm);
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
   eqn->rterm = term_li_normalform(desc, eqn->rterm);
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

   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));
   
   /* printf("# ClauseComputeLINormalform(%ld)...\n",clause->ident); */
   if(prefer_general!=0)
   {
      printf("ClauseComputeLINormalform(level=%d prefer_general=%d\n",
             level, prefer_general);
   }

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
   DEBUG(RW_INTERFACE_WATCH1, printf("# Res :");
	 ClausePrint(stdout, clause, true);printf("\n"););
   DEBUGMARK(RW_INTERFACE_WATCH1, "...ClauseComputeLINormalform()\n");      
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

   DEBUGMARK(RW_INTERFACE_WATCH1,
	     "ClauseSetComputeLINormalform()...\n");

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
   DEBUGMARK(RW_INTERFACE_WATCH1, "...ClauseSetComputeLINormalform()\n");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FindClausesWithRewritableMaxSides()
//
//   Add pointers to all clauses in set for which (potentially)
//   maximal sides can be rewritten with new_demod to the results
//   tree. This uses two optimizations: 
//
//   1) If a terms nf_date is equal to nf_date, the term (and all
//      subterms) are considered to be irreducible with respect to
//      new_demod (hence nf_date should be bigger than any nf_date of
//      terms in clauses). If a term and all its subterms are
//      irreducible by demod, it's date will be set to nf_date.
//   2) If a terms nf_date is equal to SysDateCreationTime(), the term
//      is considered to be reducible with respect to new_demod. If a
//      term is rewritable, it's nf_date will be set to
//      SysDateCreationTime(). 
//
//   Returns true if any rewritable non-maximal sides are found.
//
// Global Variables: -
//
// Side Effects    : Changes nf_dates of terms
//
/----------------------------------------------------------------------*/

bool FindClausesWithRewritableMaxSides(OCB_p ocb, ClauseSet_p set,
				       PStack_p results, Clause_p
				       new_demod, SysDate nf_date)
{   
   return find_clauses_with_rw_max_sides(ocb, set, results, new_demod,
					 nf_date);
   /* Later: Use the index if it exists */
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


