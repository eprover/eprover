/*-----------------------------------------------------------------------

  File  : clb_pred_elim.c

  Author: Petar Vukmirovic

  Contents

  Implements (defined) predicate elimination as described in
  SAT-inspired eliminations for superposition
  (https://ieeexplore.ieee.org/document/9617710).

  Copyright 2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created:  Fri Jan 7 2022 13:34:39 CET

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
   IS_GATE = 1, //values important
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
   long size;

   long last_check_num_lit;
   double last_check_sq_vars;

   long heap_idx;
};
typedef struct PETaskCell* PETask_p;

typedef Clause_p (*ResolverFun_p)(Clause_p, Clause_p, FunCode);

// binarizes boolean values -- ensures that they are 1 or 0 which C
// standard does not guarantee
#define BIN(x) ((x) ? 1 : 0)
#define CAN_SCHEDULE(t) (!(t)->offending_cls->card || (t)->g_status == IS_GATE)
#define IN_HEAP(t) ((t)->heap_idx != -1)

#define CCSFree(junk) PTreeFree(junk->set); SizeFree(junk, sizeof(CheapClauseSet))
#define TASK_BLOCKED (-1)

// lifted lambda that simply gives DEREF NEVER to TBInsertOpt
Term_p reassign_vars(void* bank, Term_p t)
{
   return TBInsertOpt(bank, t, DEREF_NEVER);
}

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
// Function: dbg_print()
//
//   Print the given clause to the given output stream.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void print_node(void* vout, void* vcl)
{
   FILE* out = vout;
   Clause_p cl = vcl;

   ClausePrint(out, cl, true);
   fputs("; ", out);
}


/*-----------------------------------------------------------------------
//
// Function: dbg_print()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void dbg_print(FILE* out, Sig_p sig, PETask_p t)
{
   fprintf(out, "%s(%ld):\n", SigFindName(sig, t->sym), t->size);

   DBG_PRINT(out, "+singular:\n >", PTreeVisitInOrder(t->positive_singular->set,
                                                      print_node, out), ".\n");
   DBG_PRINT(out, "-singular:\n >", PTreeVisitInOrder(t->negative_singular->set,
                                                      print_node, out), ".\n");
   if(t->g_status == IS_GATE)
   {
      DBG_PRINT(out, "+gate:\n >", PTreeVisitInOrder(t->neg_gates->set,
                                                     print_node, out), ".\n");
      DBG_PRINT(out, "-gate:\n >", PTreeVisitInOrder(t->pos_gates->set,
                                                     print_node, out), ".\n");
   }
   DBG_PRINT(out, "offending:\n >", PTreeVisitInOrder(t->offending_cls->set,
                                                      print_node, out), ".\n");
}

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

void update_proof_object(Clause_p new_clause, Clause_p p1, Clause_p p2,
                         DerivationCode dc)
{
   new_clause->proof_depth = MAX(p1->proof_depth, p2->proof_depth) + 1;
   new_clause->proof_size = p1->proof_size + p2->proof_size + 1;

   ClauseSetProp(new_clause, ClauseGiveProps(p1, CPIsSOS)|ClauseGiveProps(p2, CPIsSOS));
   ClauseSetTPTPType(new_clause,
      TPTPTypesCombine(ClauseQueryTPTPType(p1), ClauseQueryTPTPType(p2)));
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
// Function: CCSStoreCl()
//
//   Store a clause in the set that tracks the number of elements.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool CCSStoreCl(CCS_p set, Clause_p cl)
{
   bool inserted = PTreeStore(&(set->set), cl);
   if(inserted)
   {
      set->card++;
   }
   return inserted;
}

/*-----------------------------------------------------------------------
//
// Function: CCSRemoveCl()
//
//   Remove a clause from the set that tracks the number of elements.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool CCSRemoveCl(CCS_p set, Clause_p cl)
{
   PTree_p removed = PTreeExtractEntry(&(set->set), cl);
   if(removed)
   {
      set->card--;
      PTreeCellFree(removed);
      return true;
   }
   else
   {
      return false;
   }
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
   res->g_status = NOT_GATE;

   res->num_lit = 0;
   res->sq_vars = 0.0;
   res->size = 0;

   res->last_check_num_lit = LONG_MAX;
   res->last_check_sq_vars = INFINITY;

   res->heap_idx = -1;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PETaskFree()
//
//   Release the memory used by task object.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void PETaskFree(PETask_p task)
{
   CCSFree(task->positive_singular);
   CCSFree(task->negative_singular);
   CCSFree(task->offending_cls);
   CCSFree(task->pos_gates);
   CCSFree(task->neg_gates);
   SizeFree(task, sizeof(struct PETaskCell));
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
static inline Eqn_p find_fcode_except(Eqn_p cl, Eqn_p exc, FunCode fc)
{
   Eqn_p found = NULL;
   while(!found && cl)
   {
      if(cl != exc && !EqnIsEquLit(cl) && cl->lterm->f_code == fc)
      {
         found = cl;
      }
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
// Function: unique_distinct_vars()
//
//   Is the term of the form p(X1, ..., Xn) where all X1...Xn are distinct
//   free variables. varset is not freed, only (possibly) modified
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool unique_distinct_vars(Term_p t, PTree_p* varset)
{
   bool res =  true;
   for(int i=0; res && i < t->arity; i++)
   {
      res = TermIsFreeVar(t->args[i]) && PTreeStore(varset, t->args[i]);
   }
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
   bool unique_vars = unique_distinct_vars(pred, &vars);

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

void update_statistics(long* num_lit, long* set_size, double* mu,
                       Clause_p cl, bool negate)
{
   const int multiplier = negate ? -1 : 1;
   *num_lit += ClauseLiteralNumber(cl)*multiplier;
   *set_size += 1*multiplier;

   PTree_p vars = NULL;
   ClauseCollectVariables(cl, &vars);
   long num_vars = PTreeNodes(vars);
   PTreeFree(vars);
   *mu += num_vars*num_vars*multiplier;
}

/*-----------------------------------------------------------------------
//
// Function: scan_clause_for_predicates()
//
//   Scans the clause for all the predicate literals and updates
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void scan_clause_for_predicates(Clause_p cl, IntMap_p sym_map, MinHeap_p queue,
                                long max_occs, bool recognize_gates, bool use_tptp_sos,
                                bool ignore_conj_syms, bool* eqn_found)
{
   bool cl_is_conj =
      (ClauseQueryTPTPType(cl) == CPTypeConjecture)||(ClauseIsGoal(cl));
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
            (sign ?
            ((*task)->pos_gates->card + (*task)->positive_singular->card) :
             ((*task)->neg_gates->card + (*task)->negative_singular->card));

         if((ignore_conj_syms && cl_is_conj) || (max_occs > 0 && occs >= max_occs))
         {
            // blocking the task
            (*task)->size = TASK_BLOCKED;
         }

         if((*task)->size != TASK_BLOCKED)
         {
            bool clause_inserted = false;
            if(!find_fcode_except(cl->literals, lit, fc))
            {
               if(recognize_gates && potential_gate(cl, lit))
               {
                  clause_inserted =
                     CCSStoreCl(sign ? (*task)->pos_gates : (*task)->neg_gates, cl);
               }
               clause_inserted =
                  CCSStoreCl(sign ? (*task)->positive_singular : (*task)->negative_singular, cl) ||
                  clause_inserted;
               /* in the beginning, we store potential gate clauses in both
                  singular and gete sets, then we check if something really
                  is a gate at the end and if so remove corresponding clauses
                  from the singular set */
            }
            else
            {
               clause_inserted =
                  CCSStoreCl((*task)->offending_cls, cl) || clause_inserted;
            }
            if(clause_inserted)
            {
               update_statistics(&(*task)->num_lit, &(*task)->size,
                                 &(*task)->sq_vars, cl, false);
            }
            if(IN_HEAP(*task))
            {
               (CAN_SCHEDULE(*task) ? MinHeapUpdateElement : MinHeapRemoveElement)
                                      (queue, (*task)->heap_idx);
            }
         }
      }
      else if(eqn_found)
      {
         *eqn_found = true;
      }
   }
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
      res = t->pos_gates->card*t->negative_singular->card +
            t->neg_gates->card*t->positive_singular->card +
            t->pos_gates->card*t->offending_cls->card +
            t->neg_gates->card*t->offending_cls->card;
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

   // prefering the ones that can be scheduled
   int res = BIN(CAN_SCHEDULE(a)) - BIN(CAN_SCHEDULE(b));
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
      if (!res && lits->lterm->f_code == f &&
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
   assert(p_cl != n_cl);

   Clause_p p_cpy = ClauseCopyDisjoint(p_cl);
   Eqn_p p_rest = NULL;
   Eqn_p p_lit = find_lit_w_head(p_cpy->literals, f, &p_rest, ONLY_POS);
   assert(p_lit);

   Clause_p n_cpy = ClauseCopy(n_cl, p_lit->bank);
   Eqn_p n_rest = NULL;
   Eqn_p n_lit = find_lit_w_head(n_cpy->literals, f, &n_rest, ONLY_NEG);
   assert(n_lit);

   Subst_p subst = SubstAlloc();
   Clause_p res = NULL;

   if(SubstComputeMgu(n_lit->lterm, p_lit->lterm, subst))
   {
      EqnListMapTerms(p_rest, (TermMapper_p)inst_term, p_lit->bank);
      EqnListMapTerms(n_rest, (TermMapper_p)inst_term, n_lit->bank);
      EqnListAppend(&p_rest, n_rest);
      p_cpy->literals = NULL;
      n_cpy->literals = NULL;
      ClauseFree(p_cpy);
      ClauseFree(n_cpy);
      EqnFree(p_lit);
      EqnFree(n_lit);

      EqnListRemoveResolved(&p_rest);
      EqnListRemoveDuplicates(p_rest);
      res = ClauseAlloc(p_rest);
      update_proof_object(res, p_cl, n_cl, DCPEResolve);
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

   PTree_p p_vars = NULL;
   PTree_p n_vars = NULL;
   Eqn_p res_lits = NULL;

   // if we have unique distinct vars (cheap to check)
   // -- we can do better
   if(unique_distinct_vars(p_lit->lterm, &p_vars) ||
      unique_distinct_vars(n_lit->lterm, &n_vars))
   {
      Subst_p subst = SubstAlloc();
#ifndef NDEBUG
      bool unif =
#endif
      SubstComputeMgu(p_lit->lterm, n_lit->lterm, subst);
      assert(unif);

      EqnListMapTerms(p_rest, (TermMapper_p)inst_term, p_lit->bank);
      EqnListMapTerms(n_rest, (TermMapper_p)inst_term, n_lit->bank);
      EqnListAppend(&res_lits, p_rest);
      EqnListAppend(&res_lits, n_rest);
      SubstDelete(subst);
   }
   else
   {
      for(int i=0; i<p_lit->lterm->arity; i++)
      {
         Eqn_p neq = EqnAlloc(p_lit->lterm->args[i], n_lit->lterm->args[i],
                              p_lit->bank, false);
         neq->next = res_lits;
         res_lits = neq;
      }

      EqnListAppend(&res_lits, p_rest);
      EqnListAppend(&res_lits, n_rest);
   }
   EqnListRemoveResolved(&res_lits);
   EqnListRemoveDuplicates(res_lits);
   Clause_p res = ClauseAlloc(res_lits);
   update_proof_object(res, p_cl, n_cl, DCPEResolve);
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
      assert(cl);
      Eqn_p pred = find_lit_w_head(cl->literals, task->sym, NULL, ANY_SIGN);
      PStackPushP(EqnIsPositive(pred) ? pos : neg, cl);
   }

   bool all_tautologies = true;
   for(PStackPointer i = 0; i < PStackGetSP(pos); i++)
   {
      Clause_p pos_cl = PStackElementP(pos, i);
      for(PStackPointer j = 0; all_tautologies && i < PStackGetSP(neg); i++)
      {
         Clause_p neg_cl = PStackElementP(neg, j);
         Clause_p res = build_neq_resolvent(pos_cl, neg_cl, task->sym);
         // NB: freshly built resolvent is freed here!
         all_tautologies = ClauseIsTautologyReal(tmp_terms, res, false);
      }
   }

   declare_not_gate(task); // removing all_clauses
   if(all_tautologies)
   {
      task->g_status = IS_GATE;
      // fprintf(stderr, "is_gate: %s;\n+: ", SigFindName(tmp_terms->sig, task->sym));
      while(!PStackEmpty(pos))
      {
         Clause_p cl = PStackPopP(pos);
         // DBG_PRINT(stderr, "", ClausePrint(stderr, cl, true), "; ");
         CCSStoreCl(task->pos_gates, cl);
         CCSRemoveCl(task->positive_singular, cl);
      }
      // fprintf(stderr,"-:");
      while(!PStackEmpty(neg))
      {
         Clause_p cl = PStackPopP(neg);
         // DBG_PRINT(stderr, "", ClausePrint(stderr, cl, true), "; ");
         CCSStoreCl(task->neg_gates, cl);
         CCSRemoveCl(task->negative_singular, cl);
      }
   }

   PStackFree(pos);
   PStackFree(neg);
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
   PStack_p all_gates = PStackAlloc();
   PTreeToPStack(all_gates, task->pos_gates->set);
   PTreeToPStack(all_gates, task->neg_gates->set);

   Clause_p pivot = PStackPopP(all_gates);
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
   Subst_p subst = SubstAlloc();
   while(!PStackEmpty(all_gates))
   {
      Clause_p cl = PStackPopP(all_gates);
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
      PObjMapStore(&fresh_original_map, res_cl, cl, PCmpFun);
   }
   PStackFree(all_gates);

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

   PStack_p iter = PStackAlloc();
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
   EqnFree(fresh_lit);
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

void build_task_queue(ClauseSet_p passive, const HeuristicParms_p parms,
                      IntMap_p* m_ref, MinHeap_p* q_ref, TB_p tmp_terms,
                      bool* eqn_found)
{
   IntMap_p sym_map = *m_ref;
   MinHeap_p task_queue = *q_ref;
   assert(sym_map);
   assert(task_queue);

   for(Clause_p cl = passive->anchor->succ; cl!=passive->anchor; cl = cl->succ)
   {
      scan_clause_for_predicates(cl, sym_map, task_queue, parms->pred_elim_max_occs,
                                 parms->pred_elim_gates, parms->use_tptp_sos,
                                 parms->pred_elim_ignore_conj_syms, eqn_found);
   }

   if(parms->pred_elim_gates)
   {
      update_gate_status(sym_map, tmp_terms);
   }

   IntMapIter_p iter = IntMapIterAlloc(sym_map, 0, LONG_MAX);
   long key;
   PETask_p t;
   while( (t = IntMapIterNext(iter, &key)) )
   {
      if(!parms->pred_elim_gates)
      {
         assert(t->pos_gates->card == 0 && t->neg_gates->card == 0);
         t->g_status = NOT_GATE;
      }
      if(CAN_SCHEDULE(t))
      {
         MinHeapAddP(task_queue, t);
      }
   }
   IntMapIterFree(iter);
}

/*-----------------------------------------------------------------------
//
// Function: do_singular_elimination()
//
//   Assuming the clauses in pos_cls tree are the ones that have the positive
//   singular occurence of sym and the neg_cls have the negative one,
//   compute the only possible resolvent between them.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_singular_elimination(PTree_p pos_cls_tree, PTree_p neg_cls_tree,
                             FunCode sym, ResolverFun_p resolver,
                             PStack_p cls, TB_p tmp_terms)
{
   PStack_p pos_cls = PStackAlloc();
   PStack_p neg_cls = PStackAlloc();
   PTreeToPStack(pos_cls, pos_cls_tree);
   PTreeToPStack(neg_cls, neg_cls_tree);

   for(PStackPointer i=0; i<PStackGetSP(pos_cls); i++)
   {
      Clause_p pcl = PStackElementP(pos_cls, i);
      for(PStackPointer j=0; j<PStackGetSP(neg_cls); j++)
      {
         Clause_p ncl = PStackElementP(neg_cls, j);
         Clause_p rcl = resolver(pcl, ncl, sym);
         if(rcl && !ClauseIsTautology(tmp_terms, rcl))
         {
            PStackPushP(cls, rcl);
         }
         else if(rcl)
         {
            ClauseFree(rcl);
         }
      }
   }

   PStackFree(pos_cls);
   PStackFree(neg_cls);
}

/*-----------------------------------------------------------------------
//
// Function: do_gates_against_offending()
//
//   Fixpoint computation in which all occurrences of sym in offending
//   clauses are removed one by one by using the clauses in gates.
//
// Global Variables, ClauseSet_p archive: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void do_gates_against_offending(PETask_p task, PStack_p cls,
                                TB_p tmp_terms, VarBank_p freshvars,
                                ClauseSet_p archive)
{
   PStack_p pos_cls = PStackAlloc();
   PStack_p neg_cls = PStackAlloc();
   PStack_p worklist = PStackAlloc();
   PTreeToPStack(pos_cls, task->pos_gates->set);
   PTreeToPStack(neg_cls, task->neg_gates->set);
   PTreeToPStack(worklist, task->offending_cls->set);

   while(!PStackEmpty(worklist))
   {
      Clause_p offending = PStackPopP(worklist);
      Eqn_p sym_occ = find_fcode_except(offending->literals, NULL, task->sym);
      if(!sym_occ)
      {
         // all occurrences of sym eliminated
         PStackPushP(cls, offending);
      }
      else
      {
         bool sign = EqnIsPositive(sym_occ);
         PStack_p gate_set = sign ? neg_cls : pos_cls;
         for(PStackPointer i = 0; i<PStackGetSP(gate_set); i++)
         {
            Clause_p gate_cl = PStackElementP(gate_set, i);
            Clause_p res =
               build_neq_resolvent(sign ? offending : gate_cl,
                                   sign ? gate_cl : offending, task->sym);
            assert(res);
            if(!ClauseIsTautology(tmp_terms, res))
            {
               ClauseNormalizeVars(res, freshvars);
               EqnListMapTerms(res->literals, reassign_vars, sym_occ->bank);
               PStackPushP(worklist, res);
            }
            else
            {
               ClauseFree(res);
            }
         }
         if(!PTreeFind(&(task->offending_cls->set), offending))
         {
            // this was intermediary clause
            ClauseSetInsert(archive, offending);
         }
      }
   }

   PStackFree(pos_cls);
   PStackFree(neg_cls);
   PStackFree(worklist);
}

/*-----------------------------------------------------------------------
//
// Function: try_gate_elimination()
//
//   Fills cls with all the following resolvents: positive gates against
//   singular negative clauses, negative gates against negative singular
//   clauses and gates against all clauses in which symbol occurs multiple
//   times.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void try_gate_elimination(PETask_p task, PStack_p cls, TB_p tmp_terms,
                          VarBank_p freshvars, ClauseSet_p archive)
{
   do_singular_elimination(task->pos_gates->set, task->negative_singular->set,
                           task->sym, build_neq_resolvent, cls, tmp_terms);
   do_singular_elimination(task->positive_singular->set, task->neg_gates->set,
                           task->sym, build_neq_resolvent, cls, tmp_terms);
   do_gates_against_offending(task, cls, tmp_terms, freshvars, archive);
}

/*-----------------------------------------------------------------------
//
// Function: try_singular_elimination()
//
//   Tries to eliminate the symbol described by task by performing classical
//   singular predicate elimination.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
void try_singular_elimination(PETask_p task, PStack_p cls,
                              ResolverFun_p resolver, TB_p tmp_terms)
{
   do_singular_elimination(task->positive_singular->set, task->negative_singular->set,
                           task->sym, resolver, cls, tmp_terms);
}

/*-----------------------------------------------------------------------
//
// Function: react_clause_added()
//
//   Update data structures to reflect adding of the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void react_clause_added(Clause_p cl, IntMap_p sym_map, MinHeap_p h, long max_occs)
{
   scan_clause_for_predicates(cl, sym_map, h, max_occs, false, false, false, NULL);
}

/*-----------------------------------------------------------------------
//
// Function: react_clause_removed()
//
//   Update data structures to reflect removing of a clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void react_clause_removed(Clause_p cl, IntMap_p sym_map, MinHeap_p h)
{
   for(Eqn_p lit = cl->literals; lit; lit = lit->next)
   {
      if(!EqnIsEquLit(lit))
      {
         PETask_p task = IntMapGetVal(sym_map, lit->lterm->f_code);
         assert(task);
         bool sign = EqnIsPositive(lit);
         if(task->size != TASK_BLOCKED)
         {
            bool removed = false;
            removed = CCSRemoveCl(sign ? task->pos_gates : task->neg_gates, cl);
            if(!removed)
            {
               removed =
                  CCSRemoveCl(sign ? task->positive_singular : task->negative_singular, cl)
                  || removed;
               removed = CCSRemoveCl(task->offending_cls, cl) || removed;
               if(removed)
               {
                  update_statistics(&(task)->num_lit, &(task)->size,
                                    &(task)->sq_vars, cl, true);
                  if(!IN_HEAP(task) &&
                     CAN_SCHEDULE(task) &&
                     task->last_check_num_lit > task->num_lit &&
                     task->last_check_sq_vars > task->sq_vars)
                  {
                     MinHeapAddP(h, task);
                  }
               }
            }
            else
            {
               assert(task->g_status == IS_GATE);
               PTreeMerge(&(task->positive_singular->set), task->pos_gates->set);
               PTreeMerge(&(task->negative_singular->set), task->neg_gates->set);
               task->neg_gates->set = task->pos_gates->set = NULL;
               declare_not_gate(task);
               if(IN_HEAP(task) && !CAN_SCHEDULE(task))
               {
                  MinHeapRemoveElement(h, task->heap_idx);
               }
               update_statistics(&(task)->num_lit, &(task)->size,
                                    &(task)->sq_vars, cl, true);
            }

         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: remove_clauses_from_state()
//
//   After symbol has successfully been eliminated, remove all clauses
//   in which symbol appeared. Then check if this elimination makes
//   elimination of some other symbol possible
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void remove_clauses_from_state(PETask_p task, IntMap_p sym_map,
                               MinHeap_p task_queue, ClauseSet_p archive)
{
   task->size = TASK_BLOCKED; // blocking the symbol!
   PStack_p all_cls = PStackAlloc();
   PTreeToPStack(all_cls, task->pos_gates->set);
   PTreeToPStack(all_cls, task->neg_gates->set);
   PTreeToPStack(all_cls, task->positive_singular->set);
   PTreeToPStack(all_cls, task->negative_singular->set);
   PTreeToPStack(all_cls, task->offending_cls->set);

   while( !PStackEmpty(all_cls) )
   {
      Clause_p cl = PStackPopP(all_cls);
      // DBG_PRINT(stderr, "removing cl: ", ClausePrintDBG(stderr, cl), ";\n");
      ClauseSetMoveClause(archive, cl);
      react_clause_removed(cl, sym_map, task_queue);
   }
   PStackFree(all_cls);
}

/*-----------------------------------------------------------------------
//
// Function: measure_decreases()
//
//   Check if replacing the symbol decreases the measure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool measure_decreases(PETask_p task, PStack_p new_cls, int tolerance,
                       bool force_mu_decrease)
{
   long num_lits = 0;
   long set_size = 0;
   double mu = 0.0;
   for(PStackPointer i=0; i < PStackGetSP(new_cls); i++)
   {
      update_statistics(&num_lits, &set_size, &mu,
                        PStackElementP(new_cls, i), false);
   }

   bool lit_cl_decrease = (num_lits < task->num_lit + tolerance) ||
                          (set_size < task->size + tolerance);
   bool mu_decrease = (mu < task->sq_vars + tolerance);

   return force_mu_decrease ? mu_decrease && lit_cl_decrease
                            : mu_decrease || lit_cl_decrease;
}

/*-----------------------------------------------------------------------
//
// Function: eliminate_predicates()
//
//   Driver that does actual predicate elimination.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void eliminate_predicates(ClauseSet_p passive, ClauseSet_p archive,
                          IntMap_p sym_map, MinHeap_p task_queue,
                          TB_p bank, TB_p tmp_bank, ResolverFun_p resolver,
                          const HeuristicParms_p parms, VarBank_p freshvars)
{
   PStack_p cls = PStackAlloc();
   while(MinHeapSize(task_queue))
   {
      PETask_p task = MinHeapPopMinP(task_queue);
      // DBG_PRINT(stderr, "chosen task:\n", dbg_print(stderr, tmp_bank->sig, task), ".\n");
      task->last_check_num_lit = task->num_lit;
      task->last_check_sq_vars = task->sq_vars;
      if(task->size == TASK_BLOCKED)
      {
         continue;
      }

      if(task->g_status == IS_GATE)
      {
         try_gate_elimination(task, cls, tmp_bank, freshvars, archive);
      }
      else
      {
         assert(!task->offending_cls->card);
         try_singular_elimination(task, cls, resolver, tmp_bank);
      }

      if(measure_decreases(task, cls, parms->pred_elim_tolerance,
                           parms->pred_elim_force_mu_decrease))
      {
         // DBG_PRINT(stderr, "removal successful:\n", dbg_print(stderr, tmp_bank->sig, task), ".\n");
         remove_clauses_from_state(task, sym_map, task_queue, archive);
         while(!PStackEmpty(cls))
         {
            Clause_p cl = PStackPopP(cls);
            // DBG_PRINT(stderr, "|> ", ClausePrint(stderr, cl, true), "; ");
            ClauseNormalizeVars(cl, freshvars);
            EqnListMapTerms(cl->literals, reassign_vars, bank);
            ClauseSetInsert(passive, cl);
            react_clause_added(cl, sym_map, task_queue, parms->pred_elim_max_occs);
         }
         // fprintf(stderr, "\n");
      }
      else
      {
         while(!PStackEmpty(cls))
         {
            ClauseFree(PStackPopP(cls));
         }
      }
      PStackReset(cls);
   }
   PStackFree(cls);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: idx_setter()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void idx_setter(void* task, int idx)
{
   ((PETask_p)task)->heap_idx = idx;
}

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

void PredicateElimination(ClauseSet_p passive, ClauseSet_p archive,
                          const HeuristicParms_p parms,  TB_p bank,
                          TB_p tmp_bank, VarBank_p fresh_vars)
{
   long pre_elimination_cnt = ClauseSetCardinality(passive);
   fprintf(stdout, "%% PE start: %ld\n", pre_elimination_cnt);
   IntMap_p sym_map = IntMapAlloc();
   MinHeap_p task_queue = MinHeapAllocWithIndex(cmp_tasks, idx_setter);
   bool eqn_found;
   build_task_queue(passive, parms, &sym_map, &task_queue, tmp_bank, &eqn_found);
   ResolverFun_p resolver = eqn_found ? build_eq_resolvent : build_neq_resolvent;
   eliminate_predicates(passive, archive, sym_map, task_queue,
                        bank, tmp_bank, resolver, parms, fresh_vars);
   fprintf(stdout, "%% PE eliminated: %ld\n",
           pre_elimination_cnt - ClauseSetCardinality(passive));

   MinHeapFree(task_queue);
   IntMapIter_p iter = IntMapIterAlloc(sym_map, 0, LONG_MAX);
   long _;
   PETask_p junk;
   while( (junk = IntMapIterNext(iter, &_)))
   {
      PETaskFree(junk);
   }
   IntMapIterFree(iter);
   IntMapFree(sym_map);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
