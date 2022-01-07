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
   long size;
   
   long heap_idx;
};
typedef struct PETaskCell* PETask_p;

// binarizes boolean values -- ensures that they are 1 or 0 which C
// standard does not guarantee
#define BIN(x) ((x) ? 1 : 0)
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
      res = !TermIsFreeVar(t) || !PTreeFind(vars, t);
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
   asser(TBTermIsTypeTerm(lit->lterm));
   
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
                      IntMap_p* m_ref, MinHeap_p* q_ref)
{
   IntMap_p sym_map = *m_ref;
   MinHeap_p task_queue = *q_ref;
   assert(sym_map);
   assert(task_queue);

   for(Clause_p cl = passive->anchor->succ; cl!=passive->anchor; cl = cl->succ)
   {
      for(Eqn_p lit = cl->literals; lit; lit->next)
      {
         if(!EqnIsEquLit(lit))
         {
            FunCode fc = lit->lterm->f_code;
            bool sign = EqnIsPositive(lit);
            PETask_p* task = IntMapGetRef(sym_map, fc);
            if(!*task)
            {
               *task = mk_task(fc);
            }

            if(!find_fcode_except(cl, lit, fc))
            {
               if(recognize_gates && potential_gate(cl, lit))
               {
                  CCSStoreCl(sign ? (*task)->pos_gates : (*task)->neg_gates, cl);
               }
               else
               {
                  CCSStoreCl(
                     sign ? (*task)->positive_singular : (*task)->negative_singular,
                     cl);
               }
            }
            else
            {
               // else it is offending
               CCSStoreCl((*task)->offending_cls, cl);
            }
         }
      }
   }
   
   if(recognize_gates)
   {
      update_gate_status(sym_map);
   }
   
   IntMapIter_p iter = IntMapIterAlloc(sym_map, 0, LONG_MAX);
   long key;
   PETask_p t;
   while( (t = IntMapIterNext(iter, &key)) )
   {
      MinHeapAddP(task_queue, t);
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

void PredicateElimination(ClauseSet_p passive, ClauseSet_p archive,
                           int max_occs, bool recognize_gates,
                           TB_p tmp_bank)
{
   IntMap_p sym_map = IntMapAlloc();
   MinHeap_p task_queue = MinHeapAlloc(cmp_tasks);
   // TODO IGNORED max_occs
   build_task_queue(passive, max_occs, recognize_gates, &sym_map, &task_queue);
   eliminate_predicates(passive, archive, sym_map, task_queue, tmp_bank);
}
