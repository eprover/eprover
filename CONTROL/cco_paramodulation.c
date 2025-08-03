/*-----------------------------------------------------------------------

File  : cco_paramodulation.c

Author: Stephan Schulz

Contents

  Controling paramodulation inferences.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 18:26:14 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "cco_paramodulation.h"
#include <cte_ho_csu.h>



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
// Function: sim_paramod_q()
//
//   Given frompos (instantiated) and pm_type, determine wether to use
//   normal, simultaneous or super-simultaneois paramodulation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static ParamodulationType sim_paramod_q(OCB_p ocb, ClausePos_p frompos,
                                        ParamodulationType pm_type)
{
   ParamodulationType res = ParamodPlain;
   Term_p max_side, rep_side;

   switch(pm_type)
   {
   case ParamodPlain:
         break;
   case ParamodSim:
   case ParamodSuperSim:
         res = pm_type;
         break;
   case ParamodOrientedSim:
         res = EqnIsOriented(frompos->literal)?ParamodSim:ParamodPlain;
         break;
   case ParamodOrientedSuperSim:
         res = EqnIsOriented(frompos->literal)?ParamodSuperSim:ParamodPlain;
         break;
   case ParamodDecreasingSim:
         max_side = ClausePosGetSide(frompos);
         rep_side = ClausePosGetOtherSide(frompos);
         res = TOGreater(ocb, max_side, rep_side, DEREF_ALWAYS, DEREF_ALWAYS)?
            ParamodSim:ParamodPlain;
         break;
   case ParamodSizeDecreasingSim:
         max_side = ClausePosGetSide(frompos);
         rep_side = ClausePosGetOtherSide(frompos);
         /* This should probably be instantiated weight... */
         res = (TermStandardWeight(max_side)>TermStandardWeight(rep_side))?
            ParamodSim:ParamodPlain;
         break;
   default:
         assert(false && "Unknown paramodulation type");
         break;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: variable_paramod()
//
//   Perform paramodulation or simulated paramodulation as
//   requested. Return result (if any)
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static Clause_p variable_paramod(TB_p bank, OCB_p ocb, ClausePos_p from,
                          ClausePos_p into, VarBank_p freshvars,
                          ParamodulationType pm_type, InfType *inf)
{
   Clause_p paramod = NULL;

   switch(sim_paramod_q(ocb, from, pm_type))
   {
   case ParamodSim:
         paramod = ClauseOrderedSimParamod(bank, ocb, from, into,
                                           freshvars);
         *inf = inf_sim_paramod;
         break;
   case ParamodSuperSim:
         paramod = ClauseOrderedSuperSimParamod(bank, ocb, from, into,
                                                freshvars);
         *inf = inf_sim_paramod;
         break;
   case ParamodPlain:
         paramod = ClauseOrderedParamod(bank, ocb, from, into,
                                        freshvars);
         *inf = inf_paramod;
         break;
   default:
         assert(false && "Unexpected paramodulation type");
         break;

   }
   return paramod;
}


/*-----------------------------------------------------------------------
//
// Function: update_clause_info()
//
//   Given a (newly generated) paramodulant and the two "real"
//   parents, update meta-information.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void update_clause_info(Clause_p child,
                               Clause_p parent1, Clause_p parent2)
{
   child->proof_size  = parent1->proof_size+parent2->proof_size+1;
   child->proof_depth = MAX(parent1->proof_depth, parent2->proof_depth)+1;
   ClauseSetTPTPType(child, ClauseQueryTPTPType(parent1));

   ClauseSetProp(child,
                 ClauseGiveProps(parent1, CPIsSOS)|
                 ClauseGiveProps(parent2, CPIsSOS));

   if(parent1!=parent2)
   {
      ClauseSetTPTPType(child,
                        TPTPTypesCombine(
                           ClauseQueryTPTPType(parent1),
                           ClauseQueryTPTPType(parent2)));
   }
}



/*---------------------------------------------------------------------*/
/*                    Local "into-paramod functions                    */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: compute_into_pm_pos_clause()
//
//   Compute all paramodulations from pminfo->from* into the clause
//   and positions described by into_clause_pos. Return number of such
//   clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static long compute_into_pm_pos_clause(ParamodInfo_p pminfo,
                                       ClauseTPos_p into_clause_pos,
                                       ClauseSet_p store,
                                       ParamodulationType pm_type)
{
   long res = 0;
   PStack_p iterstack;
   NumTree_p cell;
   Term_p    lside, rside;
   Clause_p  clause;

   pminfo->into = into_clause_pos->clause;

   iterstack = NumTreeTraverseInit(into_clause_pos->pos);
   while ((cell = NumTreeTraverseNext(iterstack)))
   {
      clause = NULL;
      pminfo->into_cpos = cell->key;
#ifdef NEVER_DEFINED
      if((pminfo->new_orig == pminfo->into)
         &&(pminfo->into_cpos == pminfo->from_cpos))
      {
         /* Optimization for the case that from and into are the same
            - consider only those where into_pos != from_pos ("=" results
            in a tautology. Note that for
            the _from_ case, we can discard the c/c case completely
            (or can we?) */
         break;
      } - this is wrong for the case of unbound variables! */
#endif
      pminfo->into_pos  = UnpackClausePos(cell->key, pminfo->into);
      lside = ClausePosGetSide(pminfo->into_pos);
      rside = ClausePosGetOtherSide(pminfo->into_pos);

      if((EqnIsOriented(pminfo->into_pos->literal)
          ||!TOGreater(pminfo->ocb, rside, lside, DEREF_ALWAYS, DEREF_ALWAYS))
         &&
         ((EqnIsPositive(pminfo->into_pos->literal)&&
           EqnListEqnIsStrictlyMaximal(pminfo->ocb,
                                       pminfo->into->literals,
                                       pminfo->into_pos->literal))
     ||
          (EqnIsNegative(pminfo->into_pos->literal)
           &&
           EqnListEqnIsMaximal(pminfo->ocb,
                               pminfo->into->literals,
                               pminfo->into_pos->literal))))
      {
         /* printf(COMCHAR" compute_into_pm_pos_clause\n");  */
         clause = ClauseParamodConstruct(pminfo, pm_type);
         if(clause)
         {
            ClauseSetInsert(store, clause);
            res++;
            update_clause_info(clause, pminfo->into, pminfo->new_orig);
            DocClauseCreationDefault(clause,
                                     pm_type==ParamodPlain?inf_paramod:inf_sim_paramod,
                                     pminfo->into,
                                     pminfo->new_orig);
            DerivationCode dc = pm_type==ParamodPlain?DCParamod:DCSimParamod;
            if(pminfo->subst_is_ho)
            {
               dc = DPSetIsHO(dc);
            }
            ClausePushDerivation(clause, dc,
                                 pminfo->into, pminfo->new_orig);
         }
      }
      ClausePosFree(pminfo->into_pos);
      if(clause && pm_type!=ParamodPlain)
      {
         break;
      }
   }
   NumTreeTraverseExit(iterstack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: compute_into_pm_pos_term()
//
//   Compute all paramodulations from clause with clause|pos = term,
//   term is the LHS for the overlap, into clauses in into_clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

long compute_pos_into_pm_term(ParamodInfo_p pminfo,
                              ParamodulationType type,
                              Term_p olterm,
                              SubtermOcc_p into_clauses,
                              ClauseSet_p store)
{
   long               res = 0;
   PStack_p           iterstack;
   PObjTree_p         cell;
   Subst_p            subst = SubstAlloc();
   Term_p             max_side, rep_side;
   ParamodulationType sim_pm;

   /*printf("\n@i %ld\n", DebugCount); */
   CSUIterator_p unif_iter =
      CSUIterInit(olterm, into_clauses->term, subst, pminfo->bank);
   while(NextCSUElement(unif_iter))
   {
      max_side = ClausePosGetSide(pminfo->from_pos);
      rep_side = ClausePosGetOtherSide(pminfo->from_pos);
      pminfo->subst_is_ho = SubstHasHOBinding(subst);

      if((EqnIsOriented(pminfo->from_pos->literal) ||
          !TOGreater(pminfo->ocb, rep_side, max_side, DEREF_ALWAYS,
                     DEREF_ALWAYS))
         &&
         EqnListEqnIsStrictlyMaximal(pminfo->ocb,
                 pminfo->from->literals,
                 pminfo->from_pos->literal))
      {
         /* printf("compute_pos_into_pm_term() oc ok\n"); */
         sim_pm = sim_paramod_q(pminfo->ocb, pminfo->from_pos, type);
         /* Iterate over all the into-clauses   */
         iterstack = PTreeTraverseInit(into_clauses->pl.pos.clauses);
         while ((cell = PTreeTraverseNext(iterstack)))
         {
            res += compute_into_pm_pos_clause(pminfo, cell->key,
                                              store, sim_pm);
         }
         PTreeTraverseExit(iterstack);
      }
   }
   CSUIterDestroy(unif_iter);
   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: compute_pos_into_termtree()
//
//   Compute all paramodulations from clause with clause|pos = term,
//   term is the LHS for the overlap, into clauses in into_tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

static long compute_pos_into_pm_termtree(ParamodInfo_p pminfo,
                                         ParamodulationType type,
                                         Term_p olterm,
                                         SubtermTree_p into_tree,
                                         ClauseSet_p store)
{
   long          res = 0;
   PStack_p      iterstack;
   PObjTree_p    cell;

   iterstack = PTreeTraverseInit(into_tree);
   while ((cell = PTreeTraverseNext(iterstack)))
   {
      res += compute_pos_into_pm_term(pminfo, type,
                                      olterm, cell->key, store);
   }
   PTreeTraverseExit(iterstack);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: compute_pos_into_pm()
//
//   Compute all paramodulations from clause with clause|pos = term,
//   term is the LHS for the overlap, into clauses in into_index.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

static long compute_pos_into_pm(ParamodInfo_p pminfo,
                                ParamodulationType type,
                                Term_p olterm,
                                OverlapIndex_p into_index,
                                ClauseSet_p store)
{
   long          res = 0;
   SubtermTree_p termtree;
   PStack_p      candidates = PStackAlloc();

   FPIndexFindUnifiable(into_index, olterm, candidates);

   while(!PStackEmpty(candidates))
   {
      termtree = PStackPopP(candidates);
      res += compute_pos_into_pm_termtree(pminfo, type,
                                          olterm, termtree, store);
   }

   PStackFree(candidates);
   return res;
}



/*---------------------------------------------------------------------*/
/*                    Local "from-paramod functions                    */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: compute_from_pm_pos_clause()
//
//   Compute all paramodulations into pminfo->into* from the clause
//   and positions described by from_clause_pos. Return number of such
//   clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static long compute_from_pm_pos_clause(ParamodInfo_p pminfo,
                                       ParamodulationType type,
                                       ClauseTPos_p from_clause_pos,
                                       ClauseSet_p store)
{
   long res = 0;
   PStack_p iterstack;
   NumTree_p cell;
   Term_p    lside, rside;
   Clause_p  clause;
   ParamodulationType pm_type;

   pminfo->from = from_clause_pos->clause;

   iterstack = NumTreeTraverseInit(from_clause_pos->pos);
   while ((cell = NumTreeTraverseNext(iterstack)))
   {
      clause = NULL;
      pminfo->from_cpos = cell->key;
      if(pminfo->new_orig == pminfo->from)
      {
         /* Optimization for the case that from and into are the same
          * - these are already handled in the "into" case */
         break;
      }
      pminfo->from_pos  = UnpackClausePos(cell->key, pminfo->from);
      pm_type = sim_paramod_q(pminfo->ocb, pminfo->from_pos, type);
      lside = ClausePosGetSide(pminfo->from_pos);
      rside = ClausePosGetOtherSide(pminfo->from_pos);

      if((EqnIsOriented(pminfo->from_pos->literal)
          ||!TOGreater(pminfo->ocb, rside, lside, DEREF_ALWAYS, DEREF_ALWAYS))
         &&
         (EqnListEqnIsStrictlyMaximal(pminfo->ocb,
                                      pminfo->from->literals,
                                      pminfo->from_pos->literal)))
      {
         /* printf(COMCHAR" compute_from_pm_pos_clause\n");  */
         clause = ClauseParamodConstruct(pminfo, pm_type);
         if(clause)
         {
            ClauseSetInsert(store, clause);
            res++;
            update_clause_info(clause, pminfo->from, pminfo->new_orig);
            DocClauseCreationDefault(clause,
                                     pm_type?inf_sim_paramod:inf_paramod,
                                     pminfo->new_orig,
                                     pminfo->from);
            DerivationCode dc = pm_type?DCSimParamod:DCParamod;
            if(pminfo->subst_is_ho)
            {
               dc = DPSetIsHO(dc);
            }
            ClausePushDerivation(clause, dc,
                                 pminfo->new_orig, pminfo->from);
         }
      }
      ClausePosFree(pminfo->from_pos);
      /* Unfortunately, this optimization is wrong here - we iterate
         over positions in the from-clause!
         if(clause && sim_pm)
         {
         break;
         }*/
   }
   NumTreeTraverseExit(iterstack);

   return res;
}




/*-----------------------------------------------------------------------
//
// Function: compute_from_pm_pos_term()
//
//   Compute all paramodulations into clause with clause|pos = term,
//   term is the LHS for the overlap, from clauses in from_clauses.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

long compute_pos_from_pm_term(ParamodInfo_p pminfo,
                              ParamodulationType type,
                              Term_p olterm,
                              SubtermOcc_p from_clauses,
                              ClauseSet_p store)
{
   long             res = 0;
   PStack_p         iterstack;
   PObjTree_p       cell;
   Subst_p          subst = SubstAlloc();
   Term_p           max_side, min_side;

   /*printf("\n@f %ld\n", DebugCount); */
   CSUIterator_p unif_iter =
      CSUIterInit(olterm, from_clauses->term, subst, pminfo->bank);
   while(NextCSUElement(unif_iter))
   {
      max_side = ClausePosGetSide(pminfo->into_pos);
      min_side = ClausePosGetOtherSide(pminfo->into_pos);
      pminfo->subst_is_ho = SubstHasHOBinding(subst);

      if((EqnIsOriented(pminfo->into_pos->literal) ||
          !TOGreater(pminfo->ocb, min_side, max_side, DEREF_ALWAYS,
                     DEREF_ALWAYS))
         &&
         ((EqnIsPositive(pminfo->into_pos->literal)&&
           EqnListEqnIsStrictlyMaximal(pminfo->ocb,
                                       pminfo->into->literals,
                                       pminfo->into_pos->literal))
          ||
          (EqnIsNegative(pminfo->into_pos->literal) &&
           EqnListEqnIsMaximal(pminfo->ocb,
                               pminfo->into->literals,
                               pminfo->into_pos->literal))))
      {
         /* printf("compute_pos_from_pm_term() oc ok\n"); */
         /* Iterate over all the into-clauses   */
         iterstack = PTreeTraverseInit(from_clauses->pl.pos.clauses);
         while ((cell = PTreeTraverseNext(iterstack)))
         {
            res += compute_from_pm_pos_clause(pminfo, type, cell->key, store);
         }
         PTreeTraverseExit(iterstack);
      }
   }
   CSUIterDestroy(unif_iter);
   SubstDelete(subst);
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: compute_pos_from_termtree()
//
//   Compute all paramodulations into clause with pminfo->into|pos = term,
//   term is the LHS for the overlap, from clauses in from_tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

static long compute_pos_from_pm_termtree(ParamodInfo_p pminfo,
                                         ParamodulationType type,
                                         Term_p olterm,
                                         SubtermTree_p from_tree,
                                         ClauseSet_p store)
{
   long          res = 0;
   PStack_p      iterstack;
   PObjTree_p    cell;

   iterstack = PTreeTraverseInit(from_tree);
   while ((cell = PTreeTraverseNext(iterstack)))
   {
      res += compute_pos_from_pm_term(pminfo, type,
                                      olterm, cell->key, store);
   }
   PTreeTraverseExit(iterstack);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: compute_pos_from_pm()
//
//   Compute all paramodulations into clause with pminfo->into|pos =
//   term, term is the LHS for the overlap, from clauses in
//   from_index.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

static long compute_pos_from_pm(ParamodInfo_p pminfo,
                                ParamodulationType type,
                                Term_p olterm,
                                OverlapIndex_p from_index,
                                ClauseSet_p store)
{
   long          res = 0;
   SubtermTree_p termtree;
   PStack_p      candidates = PStackAlloc();

   FPIndexFindUnifiable(from_index, olterm, candidates);

   while(!PStackEmpty(candidates))
   {
      termtree = PStackPopP(candidates);
      res += compute_pos_from_pm_termtree(pminfo, type,
                                           olterm, termtree, store);
   }

   PStackFree(candidates);
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ComputeClauseClauseParamodulants()
//
//   Compute all (simultaneous) paramodulants between clause and with,
//   with terms from bank, and put them into store. Returns number of
//   paramodulants.
//
// Global Variables: -
//
// Side Effects    : As above...
//
/----------------------------------------------------------------------*/

long ComputeClauseClauseParamodulants(TB_p bank, OCB_p ocb, Clause_p
                                      clause, Clause_p parent_alias,
                                      Clause_p with, ClauseSet_p
                                      store, VarBank_p freshvars,
                                      ParamodulationType pm_type)
{
   Clause_p    paramod;
   Term_p      test;
   ClausePos_p pos1, pos2;
   long        paramod_count = 0;
   InfType     inf_type = inf_noinf;


   assert(EqnListQueryPropNumber(clause->literals, EPIsMaximal));
   assert(EqnListQueryPropNumber(clause->literals, EPIsMaximal)
          ==
          EqnListQueryPropNumber(parent_alias->literals, EPIsMaximal));
   assert(EqnListQueryPropNumber(with->literals, EPIsMaximal));

   if(ClauseQueryProp(clause, CPNoGeneration)||
      ClauseQueryProp(with, CPNoGeneration))
   {
      return 0;
   }

   pos1 = ClausePosAlloc();
   pos2 = ClausePosAlloc();

   test = ClausePosFirstParamodPair(clause, pos1, with, pos2,
                false, pm_type != ParamodPlain);
   while(test)
   {
      assert(TermPosIsTopPos(pos1->pos));

      /* printf("\n@i %ld\n", DebugCount); */
      paramod = variable_paramod(bank, ocb, pos1, pos2,
                                 freshvars, pm_type, &inf_type);

      if(paramod)
      {
    paramod_count++;
    paramod->proof_size  =
       parent_alias->proof_size+with->proof_size+1;
    paramod->proof_depth = MAX(parent_alias->proof_depth,
                with->proof_depth)+1;
    ClauseSetTPTPType(paramod,
            ClauseQueryTPTPType(parent_alias));
    ClauseSetProp(paramod,
             ClauseGiveProps(parent_alias, CPIsSOS)|
             ClauseGiveProps(with, CPIsSOS));
    if(parent_alias!=with)
    {
       ClauseSetTPTPType(paramod,
               TPTPTypesCombine(
             ClauseQueryTPTPType(paramod),
             ClauseQueryTPTPType(with)));
    }
    DocClauseCreationDefault(paramod, inf_type, with,
              parent_alias);
         ClausePushDerivation(clause,
                              inf_type==inf_sim_paramod?DCSimParamod:DCParamod,
                              with, parent_alias);
    ClauseSetInsert(store, paramod);
      }
      test = ClausePosNextParamodPair(pos1, pos2, false, pm_type != ParamodPlain);
   }
   /* Paramod clause into with - no top positions this time ;-) */

   if(parent_alias==with)
   {
      /* Both clauses are identical, i.e. both cases are
    symmetric. Ergo do nothing... */
   }
   else
   {
      test = ClausePosFirstParamodPair(with, pos1, clause, pos2, true,
                                       pm_type != ParamodPlain);

      while(test)
      {
    assert(TermPosIsTopPos(pos1->pos));
         /*printf("\n@f %ld\n", DebugCount);*/
         paramod = variable_paramod(bank, ocb, pos1, pos2,
                                    freshvars, pm_type, &inf_type);
    if(paramod)
    {
       paramod_count++;
       paramod->proof_size  =
          parent_alias->proof_size+with->proof_size+1;
       paramod->proof_depth = MAX(parent_alias->proof_depth,
                   with->proof_depth)+1;
       ClauseSetTPTPType(paramod,
               ClauseQueryTPTPType(with));
       ClauseSetProp(paramod,
           ClauseGiveProps(parent_alias, CPIsSOS)|
           ClauseGiveProps(with, CPIsSOS));
       if(parent_alias!=with)
       {
          ClauseSetTPTPType(paramod,
             TPTPTypesCombine(
                ClauseQueryTPTPType(paramod),
                ClauseQueryTPTPType(parent_alias)));
       }
       DocClauseCreationDefault(paramod, inf_type, parent_alias, with);
            ClausePushDerivation(clause,
                                 inf_type==inf_sim_paramod?DCSimParamod:DCParamod,
                                 parent_alias, with);
       ClauseSetInsert(store, paramod);
    }
    test = ClausePosNextParamodPair(pos1, pos2, true, pm_type != ParamodPlain);
      }
   }
   ClausePosFree(pos1);
   ClausePosFree(pos2);

   return paramod_count;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeAllParamodulants()
//
//   Compute all paramodulants between clause and with_set, put them
//   into store.
//
// Global Variables: -
//
// Side Effects    : By called functions, memory manangement.
//
/----------------------------------------------------------------------*/

long ComputeAllParamodulants(TB_p bank, OCB_p ocb, Clause_p clause,
              Clause_p parent_alias, ClauseSet_p
              with_set, ClauseSet_p store, VarBank_p
              freshvars, ParamodulationType pm_type)
{
   Clause_p handle;
   long     paramod_count = 0;

   for(handle = with_set->anchor->succ; handle != with_set->anchor;
       handle = handle->succ)
   {
      paramod_count +=
    ComputeClauseClauseParamodulants(bank, ocb, clause,
                                          parent_alias, handle,
                                          store, freshvars, pm_type);
   }
   return paramod_count;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeIntoParamodulants()
//
//   Compute all paramodulants from clause into clauses in
//   into_index.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/


long ComputeIntoParamodulants(ParamodInfo_p pminfo,
                              ParamodulationType type,
                              Clause_p clause,
                              OverlapIndex_p into_index,
                              OverlapIndex_p negp_index,
                              ClauseSet_p store)
{
   long          res = 0;
   PStack_p      pos_stack = PStackAlloc();
   Term_p        olterm;
   CompactPos    pos;

   ClauseCollectFromTermsPos(clause, pos_stack);
   pminfo->from = clause;

   while(!PStackEmpty(pos_stack))
   {
      pos    = PStackPopInt(pos_stack);
      olterm = PStackPopP(pos_stack);
      pminfo->from_cpos  = pos;
      pminfo->from_pos   = UnpackClausePos(pos, clause);
      res += compute_pos_into_pm(pminfo, type, olterm, negp_index, store);
      if(EqnIsEquLit(pminfo->from_pos->literal))
      {
         res += compute_pos_into_pm(pminfo, type, olterm, into_index, store);
      }
      ClausePosFree(pminfo->from_pos);
   }
   PStackFree(pos_stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeFromParamodulants()
//
//   Compute all paramodulants from clauses in from_index into
//   clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ComputeFromParamodulants(ParamodInfo_p pminfo,
                              ParamodulationType type,
                              Clause_p clause,
                              OverlapIndex_p from_index,
                              ClauseSet_p store)
{
   long res = 0;
   PStack_p      pos_stack = PStackAlloc();
   Term_p        olterm;
   CompactPos    pos;

   ClauseCollectIntoTermsPos(clause, pos_stack);
   pminfo->into = clause;

   while(!PStackEmpty(pos_stack))
   {
      pos    = PStackPopInt(pos_stack);
      olterm = PStackPopP(pos_stack);
      pminfo->into_cpos  = pos;
      pminfo->into_pos   = UnpackClausePos(pos, clause);

      /* Positive/positive top level has already been done in the
         into-case.*/
      if(EqnIsNegative(pminfo->into_pos->literal)||
         !ClausePosIsTop(pminfo->into_pos))
      {
         res += compute_pos_from_pm(pminfo, type, olterm, from_index, store);
      }
      ClausePosFree(pminfo->into_pos);
   }
   PStackFree(pos_stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeFromSimParamodulants()
//
//   Compute all simultaneouss paramodulants from clauses in
//   from_index into clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ComputeFromSimParamodulants(ParamodInfo_p pminfo,
                                 ParamodulationType type,
                                 Clause_p clause,
                                 OverlapIndex_p from_index,
                                 ClauseSet_p store)
{
   long res = 0;
   PStack_p      pos_stack = PStackAlloc();
   Term_p        olterm;
   CompactPos    pos;

   /*
   // Here: Collect terms and term positions in a double tree (or
   // tree/stack construct)
   */
   ClauseCollectIntoTermsPos(clause, pos_stack);

   pminfo->into = clause;

   /*
   // Here: Iterate over the terms, and inside the terms over the
   // positions.
   */
   while(!PStackEmpty(pos_stack))
   {
      pos    = PStackPopInt(pos_stack);
      olterm = PStackPopP(pos_stack);
      pminfo->into_cpos  = pos;
      pminfo->into_pos   = UnpackClausePos(pos, clause);

      /* Positive/positive top level has already been done in the
         into-case.*/
      if(EqnIsNegative(pminfo->into_pos->literal)||
         !ClausePosIsTop(pminfo->into_pos))
      {
         res += compute_pos_from_pm(pminfo, type, olterm, from_index, store);
      }
      ClausePosFree(pminfo->into_pos);
   }
   PStackFree(pos_stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeAllParamodulantsIndexed()
//
//   Compute all paramodulants (of the right pm_type) between clause
//   and clauses in the indices. Put them into store. Return number of
//   clauses generated.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ComputeAllParamodulantsIndexed(TB_p bank, OCB_p ocb,
                                    VarBank_p freshvars,
                                    Clause_p clause,
                                    Clause_p parent_alias,
                                    OverlapIndex_p into_index,
                                    OverlapIndex_p negp_index,
                                    OverlapIndex_p from_index,
                                    ClauseSet_p store,
                                    ParamodulationType pm_type)
{
   long res = 0;
   ParamodInfoCell pminfo;

   pminfo.bank      = bank;
   pminfo.freshvars = freshvars;
   pminfo.ocb       = ocb;
   pminfo.new_orig  = parent_alias;

   res += ComputeIntoParamodulants(&pminfo,
                                   pm_type,
                                   clause,
                                   into_index,
                                   negp_index,
                                   store);

   res += ComputeFromParamodulants(&pminfo,
                                   pm_type,
                                   clause,
                                   from_index,
                                   store);

   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
