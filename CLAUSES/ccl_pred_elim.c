/*-----------------------------------------------------------------------

File  : clb_pred_elim.c

Author: Petar Vukmirovic

Contents

  Implements (defined) predicate elimination as described in 
  SAT-inspired eliminations for superposition
  (https://ieeexplore.ieee.org/document/9617710).

Copyright 1998-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> vr  7 jan 2022 13:34:39 CET
-----------------------------------------------------------------------*/

#include "ccl_pred_elim.h"
#include <clb_min_heap.h>
#include <ccl_satinterface.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct 
{
   long card; // querying size in O(1)
   PTree_p set; // querying membership in O(log(card))
} CheapClauseSet;
typedef CheapClauseSet* CCS_p;

typedef enum 
{
   CANDIDATE = 0, // still checking... only to be used in initialization
   IS_GATE = 1,
   NOT_GATE = 2
} GateStatus;

struct PETaskCell
{
   FunCode sym;
   CCS_p positive_singular;
   CCS_p negative_singular;
   CCS_p offending_cls;

   CCS_p pos_gates;
   CCS_p neg_gates;
   GateStatus g_status;

   long num_lit;
   double sq_vars;
   long size; // size is set to -1 to denote that maximal 
              // number of occurences has been reached
   
   long heap_idx;
};
typedef struct PETaskCell* PETask_p;

// binarizes boolean values -- ensures that they are 1 or 0 which C
// standard does not guarantee
#define BIN(x) ((x) ? 1 : 0)
#define CAN_SCHEDULE(t) (!(t)->offending_cls->card || (t)->g_status == IS_GATE)
#define IN_HEAP(t) ((t)->heap_idx != -1)
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
// Function: set_proof_object()
// 
//   Set proof object according to given arguments
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void set_proof_object(Clause_p new_clause, Clause_p p1, Clause_p p2,
                      DerivationCode dc)
{
   new_clause->proof_depth =
      MAX(p1->proof_depth, p2->proof_depth) + 1;
   new_clause->proof_size = p1->proof_size + p2->proof_size + 1;
   ClauseSetTPTPType(new_clause, ClauseQueryTPTPType(p1));
   ClauseSetProp(new_clause, ClauseGiveProps(p1, CPIsSOS));
   ClauseSetProp(new_clause, ClauseGiveProps(p2, CPIsSOS));
   // TODO: Clause documentation is not implemented at the moment.
   // DocClauseCreationDefault(clause, inf_efactor, clause, NULL);
   ClausePushDerivation(new_clause, dc, p1, p2);
}

/*-----------------------------------------------------------------------
//
// Function: mk_task()
// 
//   Make a default task cell
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline CCS_p mk_ccs()
{
   CCS_p res = SizeMalloc(sizeof(CheapClauseSet));
   res->set = NULL;
   res->card = 0;
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: mk_task()
// 
//   Make a default task cell
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline void CCSStoreCl(CCS_p set, Clause_p cl)
{
   set->card += PTreeStore(&(set->set), cl) ? 1 : 0;
}

/*-----------------------------------------------------------------------
//
// Function: mk_task()
// 
//   Make a default task cell
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline PETask_p mk_task(FunCode fc)
{
   PETask_p res = SizeMalloc(sizeof(struct PETaskCell));
   res->sym = fc;
   
   res->positive_singular = mk_ccs();
   res->negative_singular = mk_ccs();
   res->offending_cls = mk_ccs();
   res->pos_gates = mk_ccs();
   res->neg_gates = mk_ccs();

   res->num_lit = 0;
   res->sq_vars = 0.0;
   res->size = 0;

   res->heap_idx = -1;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: find_fcode_except()
// 
//   Is there a predicate litaral with the code fc in eqn list cl (ignoring)
//   its literal exc
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
static inline bool find_fcode_except(Eqn_p cl, Eqn_p exc, FunCode fc)
{
   bool found = false;
   while(!found && cl)
   {
      found = cl != exc && !EqnIsEquLit(cl) && cl->lterm->f_code == fc;
      cl = cl->next;
   }
   return found;
}

/*-----------------------------------------------------------------------
//
// Function: term_vars_from_set()
// 
//   Are all variables in t from the set vars?
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
bool term_vars_from_set(Term_p t, PTree_p* vars)
{
   PStack_p subterms = PStackAlloc();
   bool res = true;
   PStackPushP(subterms, t);

   while(res && !PStackEmpty(subterms))
   {
      t = PStackPopP(subterms);
      res = !TermIsFreeVar(t) || PTreeFind(vars, t);
      for(int i=0; res && i<t->arity; i++)
      {
         PStackPushP(subterms, t->args[i]);
      }
   }
   
   PStackFree(subterms);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: potential_gate()
// 
//   Is the clause of the chape (~)p(X1, ..., Xn) \/ C where all Xi are 
//   different and variables in C are subset of X1, ..., Xn
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool potential_gate(Clause_p cl, Eqn_p lit)
{  
   PTree_p vars = NULL;
   Term_p pred = lit->lterm;
   bool unique_vars = true;
   for(int i=0; unique_vars && i < pred->arity; i++)
   {
      unique_vars = TermIsFreeVar(pred->args[i]) &&
                    PTreeStore(&vars, pred->args[i]);
   }

   for(Eqn_p iter = cl->literals; unique_vars && iter; iter = iter->next)
   {
      unique_vars = iter == lit ||
                    (term_vars_from_set(iter->lterm, &vars) && 
                     term_vars_from_set(iter->rterm, &vars));
   }

   PTreeFree(vars);
   return unique_vars;
}

/*-----------------------------------------------------------------------
//
// Function: max_cardinality()
// 
//   What is the maximal cardinality of the set of the clauses that would
//   be created when the symbol is eliminated?
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline long max_cardinality(const PETask_p t)
{
   long res;
   if(t->g_status == IS_GATE)
   {
      assert(!t->offending_cls->card);
      res = t->pos_gates->card*t->negative_singular->card +
            t->neg_gates->card*t->positive_singular->card;
   }
   else
   {
      res = t->positive_singular->card*t->negative_singular->card;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: cmp_tasks()
// 
//   Comparator function used for ordering the tasks in the min heap.
//   Prefers the tasks that are eligible for PE over the ones that are not,
//   then the ones with the smallest cardinality, then the ones that have
//   gates.
//   
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define EXIT_ON_DIFF(res) if(res){return (res);}
int cmp_tasks(IntOrP* ip_a, IntOrP* ip_b)
{
   PETask_p a = ip_a->p_val;
   PETask_p b = ip_b->p_val;

   // prefering the ones that are singular
   int res = BIN(a->offending_cls->card != 0) - BIN(b->offending_cls->card != 0);
   EXIT_ON_DIFF(res);
   
   // prefering smaller sets
   res = CMP(max_cardinality(a), max_cardinality(b));
   EXIT_ON_DIFF(res);

   res = BIN(a->g_status == IS_GATE) - BIN(b->g_status == IS_GATE);
   EXIT_ON_DIFF(res);

   assert(a->sym != b->sym);
   return CMP(a->sym, b->sym);
}

/*-----------------------------------------------------------------------
//
// Function: declare_not_gate()
// 
//   Go through all the tasks and see if their potential gates are 
//   actually gates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void declare_not_gate(PETask_p task)
{
   task->pos_gates->card = 0;
   PTreeFree(task->pos_gates->set);
   task->pos_gates->set = NULL;

   task->neg_gates->card = 0;
   PTreeFree(task->neg_gates->set);
   task->neg_gates->set = NULL;

   task->g_status = NOT_GATE;
}

/*-----------------------------------------------------------------------
//
// Function: find_lit_w_head()
// 
//    Find a (predicate) literal that whose head symbol is f and return it.
//    If such a literal is not found return NULL; Extracts the other
//    literals in rest if rest is not NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define ANY_SIGN 0
#define ONLY_NEG 1
#define ONLY_POS 2

Eqn_p find_lit_w_head(Eqn_p lits, FunCode f, EqnRef rest, int sign)
{
   Eqn_p res = NULL;
   while(lits)
   {
      Eqn_p tmp = lits->next;
      if (lits->lterm->f_code == f &&
         (sign == 0 || ((sign-1) == (EqnIsPositive(lits) ? 1 : 0))))
      {
         res = lits;
         if(rest)
         {
            // extracting the element
            lits->next = NULL;
         }
         else
         {
            break;
         }
      }
      else if(rest)
      {
         EqnListInsertFirst(rest, lits);
      }
      lits = tmp;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: build_neq_resolvent()
// 
//   Builds regular non-equational resolvent between p_cl and n_cl clause
//   more precisely between their literals pos and neg where pos and neg are
//   the first positive/negative literal that contain symbol f. Undefined 
//   behavior if they do not contain f. If resolvent cannot be built 
//   (not unifiable), return NULL
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p inst_term(TB_p bank, Term_p t)
{
   return TBInsert(bank, t, DEREF_ALWAYS);
}

Clause_p build_neq_resolvent(Clause_p p_cl, Clause_p n_cl, FunCode f)
{
   Clause_p p_cpy = ClauseCopyDisjoint(p_cl);
   Eqn_p p_rest = NULL;
   Eqn_p p_lit = find_lit_w_head(p_cl->literals, f, &p_rest, ONLY_POS);
   assert(p_lit);

   Clause_p n_cpy = ClauseCopy(n_cl, p_lit->bank);
   Eqn_p n_rest = NULL;
   Eqn_p n_lit = find_lit_w_head(n_cl->literals, f, &n_rest, ONLY_NEG);
   assert(n_lit);

   Subst_p subst = SubstAlloc();
   Clause_p res = NULL;
   if(SubstComputeMgu(n_lit->lterm, p_lit->lterm, subst))
   {
      EqnListMapTerms(p_rest, (TermMapper_p)inst_term, p_lit->bank);
      EqnListMapTerms(n_rest, (TermMapper_p)inst_term, n_lit->bank);
      p_rest = EqnListAppend(&p_rest, n_rest);
      p_cpy->literals = NULL;
      n_cpy->literals = NULL;
      ClauseFree(p_cpy);
      ClauseFree(n_cpy);
      EqnFree(p_lit);
      EqnFree(n_lit);

      res = ClauseAlloc(p_rest);
      set_proof_object(res, p_cl, n_cl, DCPEResolve);
   }
   else
   {
      EqnFree(p_lit);
      EqnFree(n_lit);
      EqnListFree(p_rest);
      EqnListFree(n_rest);
      p_cpy->literals = NULL;
      n_cpy->literals = NULL;
      ClauseFree(n_cpy);
      ClauseFree(p_cpy);
   }

   SubstDelete(subst);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: build_eq_resolvent()
// 
//   Like build_neq_resolvent() but (1) builds EQ resolvent and 
//   (2) never fails as there is no unification involved.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p build_eq_resolvent(Clause_p p_cl, Clause_p n_cl, FunCode f)
{
   Eqn_p p_disjoint = EqnListCopyDisjoint(p_cl->literals);
   Eqn_p p_rest = NULL;
   Eqn_p p_lit = find_lit_w_head(p_disjoint, f, &p_rest, ONLY_POS);
   assert(p_lit);

   Eqn_p n_copy = EqnListCopy(n_cl->literals, p_lit->bank);
   Eqn_p n_rest = NULL;
   Eqn_p n_lit = find_lit_w_head(n_copy, f, &n_rest, ONLY_NEG);
   assert(n_lit);

   Eqn_p cond = NULL;
   for(int i=0; i<p_lit->lterm->arity; i++)
   {
      Eqn_p neq = EqnAlloc(p_lit->lterm->args[i], n_lit->lterm->args[i],
                           p_lit->bank, false);
      EqnListInsertFirst(&cond, neq);
   }

   cond = EqnListAppend(&cond, p_rest);
   cond = EqnListAppend(&cond, n_rest);

   Clause_p res = ClauseAlloc(p_rest);
   set_proof_object(res, p_cl, n_cl, DCPEResolve);
   EqnFree(p_lit);
   EqnFree(n_lit);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: check_unsat_and_tauto()
// 
//   Checks condition (4) and (5) from Definition 13. in SAT techniques
//   paper (https://matryoshka-project.github.io/pubs/satelimsup_paper.pdf)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void check_tautologies(PETask_p task, PStack_p unsat_core, TB_p tmp_terms)
{
   PStack_p pos = PStackAlloc();
   PStack_p neg = PStackAlloc();

   while(!PStackEmpty(unsat_core))
   {
      Clause_p cl = PStackPopP(unsat_core);
      Eqn_p pred = find_lit_w_head(cl->literals, task->sym, NULL, ANY_SIGN);
      PStackPushP(EqnIsPositive(pred) ? pos : neg, cl);
   }

   bool all_tautologies = true;
   for(PStackPointer i = 0; i < PStackGetSP(pos); i++)
   {
      Clause_p pos_cl = PStackElementP(pos, i);
      for(PStackPointer j = 0; all_tautologies && i < PStackGetSP(neg); i++)
      {
         Clause_p neg_cl = PStackElementP(pos, j);
         Clause_p res = build_neq_resolvent(pos_cl, neg_cl, task->sym);
         // NB: freshly built resolvent is freed here!
         all_tautologies = ClauseIsTautologyReal(tmp_terms, res, false);
      }
   }

   declare_not_gate(task); // removing all_clauses
   if(all_tautologies)
   {
      task->g_status = IS_GATE;
      while(!PStackEmpty(pos))
      {
         CCSStoreCl(task->pos_gates, PStackPopP(pos));
      }
      while(!PStackEmpty(neg))
      {
         CCSStoreCl(task->neg_gates, PStackPopP(neg));
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: check_unsat_and_tauto()
// 
//   Checks condition (4) and (5) from Definition 13. in SAT techniques
//   paper (https://matryoshka-project.github.io/pubs/satelimsup_paper.pdf)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void check_unsat_and_tauto(PETask_p task, TB_p tmp_terms)
{
   PTreeMerge(&(task->pos_gates->set), task->neg_gates->set);
   PTree_p all_gates = task->pos_gates->set;
   Clause_p pivot = PTreeExtractRootKey(&all_gates);
   Clause_p pivot_fresh = ClauseCopyDisjoint(pivot);
   Eqn_p rest_fresh = NULL;
   Eqn_p fresh_lit = find_lit_w_head(pivot_fresh->literals, task->sym,  &rest_fresh, ANY_SIGN);
   TB_p bank = fresh_lit->bank;
   pivot_fresh->literals = rest_fresh;
   ClauseRecomputeLitCounts(pivot_fresh);

   SatSolver_p solver = picosat_init();
   picosat_enable_trace_generation(solver);

   SatClauseSet_p environment = SatClauseSetAlloc();
   SatClauseCreateAndStore(pivot_fresh, environment);

   // map connecting fresh clauses to original ones
   PObjMap_p fresh_original_map = NULL;
   PObjMapStore(&fresh_original_map, pivot_fresh, pivot, PCmpFun);

   // all clauses need to fit to the pivot
   PStack_p iter = PTreeTraverseInit(all_gates);
   PTree_p key;
   Subst_p subst = SubstAlloc();
   while((key = PTreeTraverseNext(iter)))
   {
      Clause_p cl = key->key;
      Eqn_p sym_lit = find_lit_w_head(cl->literals, task->sym, NULL, ANY_SIGN);
#ifndef NDEBUG
      bool subst_res = 
#endif
      SubstComputeMatch(sym_lit->lterm, fresh_lit->lterm, subst);
      assert(subst_res); // both are distinct bound variables, just matching them

      Eqn_p rest = EqnListCopyExcept(cl->literals, sym_lit, bank);
      Clause_p res_cl = ClauseAlloc(rest);
      SubstBacktrack(subst);
      SatClauseCreateAndStore(res_cl, environment);            
   }
   PTreeTraverseExit(iter);

   PStack_p unsat_core = PStackAlloc();
   if(SatClauseSetCheckAndGetCore(environment, solver, unsat_core))
   {
      // restoring original clauses
      for(PStackPointer i=0; i<PStackGetSP(unsat_core); i++)
      {
         PStackAssignP(unsat_core, i,
                      PObjMapFind(&fresh_original_map, 
                                  PStackElementP(unsat_core, i), PCmpFun));
      }

      check_tautologies(task, unsat_core, tmp_terms);
   }
   else
   {
      declare_not_gate(task);
   }

   iter = PStackAlloc();
   iter = PObjMapTraverseInit(fresh_original_map, iter);
   PObjMap_p n = NULL;
   Clause_p fresh = NULL;
   while( (n = PObjMapTraverseNext(iter, (void**)&fresh)) )
   {
      ClauseFree(fresh);
   }
   PObjMapFree(fresh_original_map);
   PStackFree(iter);
   SatClauseSetFree(environment);
   PStackFree(unsat_core);
   SubstDelete(subst);
   picosat_reset(solver);
}


/*-----------------------------------------------------------------------
//
// Function: update_gate_status()
// 
//   Go through all the tasks and see if their potential gates are 
//   actually gates.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void update_gate_status(IntMap_p sym_map, TB_p tmp_terms)
{
   IntMapIter_p iter = IntMapIterAlloc(sym_map, 0, LONG_MAX);
   PETask_p task;
   long dummy;
   while((task = IntMapIterNext(iter, &dummy)))
   {
      if(task->pos_gates->card && task->neg_gates->card)
      {
         check_unsat_and_tauto(task, tmp_terms);
      }
      else
      {
         declare_not_gate(task);
      }
   }
   IntMapIterFree(iter);
}

/*-----------------------------------------------------------------------
//
// Function: update_statistics()
// 
//   Update number of literals, clauses and \mu measure (square of the 
//   number of different variables).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void update_statistics(PETask_p task, Clause_p cl)
{
   task->num_lit += ClauseLiteralNumber(cl);
   task->size += 1;

   PTree_p vars = NULL;
   ClauseCollectVariables(cl, &vars);
   long num_vars = PTreeNodes(vars);
   PTreeFree(vars);
   task->sq_vars += num_vars*num_vars;
}

/*-----------------------------------------------------------------------
//
// Function: PredicateElimination()
// 
//   Preprocess the passive clause set, create corresponding predicate
//   elimination tasks, store them in the symbol map and insert them
//   in the task queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void build_task_queue(ClauseSet_p passive, int max_occs, bool recognize_gates, 
                      IntMap_p* m_ref, MinHeap_p* q_ref, TB_p tmp_terms)
{
   IntMap_p sym_map = *m_ref;
   MinHeap_p task_queue = *q_ref;
   assert(sym_map);
   assert(task_queue);

   for(Clause_p cl = passive->anchor->succ; cl!=passive->anchor; cl = cl->succ)
   {
      for(Eqn_p lit = cl->literals; lit; lit = lit->next)
      {
         if(!EqnIsEquLit(lit))
         {
            FunCode fc = lit->lterm->f_code;
            bool sign = EqnIsPositive(lit);
            PETask_p* task = (PETask_p*)IntMapGetRef(sym_map, fc);
            if(!*task)
            {
               *task = mk_task(fc);
            }

            int occs = (*task)->offending_cls->card + 
                        sign ? (*task)->pos_gates->card + (*task)->positive_singular->card :
                               (*task)->neg_gates->card + (*task)->negative_singular->card;
            
            if(occs >= max_occs)
            {
               // blocking the task
               (*task)->size = -1;
            }

            if((*task)->size != -1)
            {
               if(!find_fcode_except(cl->literals, lit, fc))
               {
                  if(recognize_gates && potential_gate(cl, lit))
                  {
                     CCSStoreCl(sign ? (*task)->pos_gates : (*task)->neg_gates, cl);
                  }
                  CCSStoreCl(sign ? (*task)->positive_singular : (*task)->negative_singular, cl);
                  /* in the beginning, we store potential gate clauses in both
                     singular and gete sets, then we check if something really
                     is a gate at the end and if so remove corresponding clauses
                     from the singular set */
               }
               else
               {
                  CCSStoreCl((*task)->offending_cls, cl);
               }
               update_statistics(*task, cl);
            }
         }
      }
   }
   
   if(recognize_gates)
   {
      update_gate_status(sym_map, tmp_terms);
   }
   
   IntMapIter_p iter = IntMapIterAlloc(sym_map, 0, LONG_MAX);
   long key;
   PETask_p t;
   while( (t = IntMapIterNext(iter, &key)) )
   {
      if(CAN_SCHEDULE(t))
      {
         MinHeapAddP(task_queue, t);
      }
   }
   IntMapIterFree(iter);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PredicateElimination()
// 
//   Does the elimination of predicate symbols by moving clauses with the
//   eliminated symbol from passive to archive. New clauses are added to 
//   passive (with appropriately set proof object). Tracking a predicate
//   symbol will be stopped after it reaches max_occs occurrences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void idx_setter(void* task, int idx)
{
   ((PETask_p)task)->heap_idx = idx;
}

void PredicateElimination(ClauseSet_p passive, ClauseSet_p archive,
                           int max_occs, bool recognize_gates,
                           TB_p tmp_bank)
{
   IntMap_p sym_map = IntMapAlloc();
   MinHeap_p task_queue = MinHeapAllocWithIndex(cmp_tasks, idx_setter);
   // TODO IGNORED max_occs
   build_task_queue(passive, max_occs, recognize_gates, &sym_map, 
                    &task_queue, tmp_bank);
   // eliminate_predicates(passive, archive, sym_map, task_queue, tmp_bank);
}
