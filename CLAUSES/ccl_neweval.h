/*-----------------------------------------------------------------------

File  : ccl_neweval.h

Author: Stephan Schulz

Contents

  Data type for representing evaluations of clauses.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr  9 02:00:51 MET DST 1998
    New
<2> Thu Jan 28 00:58:19 MET 1999
    Replaced AVL trees with Splay-Trees
<3> Thu Apr 20 00:32:11 CEST 2006
    Imported code and history for new, more efficient evaluations for
    ccl_evaluations.h

-----------------------------------------------------------------------*/

#ifndef CCL_NEWEVAL

#define CCL_NEWEVAL

#include <clb_avlgeneric.h>
#include <clb_ptrees.h>
#include <clb_sysdate.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef long EvalPriority;

#define PrioBest     0
#define PrioPrefer  30
#define PrioNormal  40
#define PrioDefer   50
#define PrioLargestReasonable MEGA


typedef struct simple_eval_cell
{
   EvalPriority      priority;   /* Technical considerations */
   float             heuristic;  /* Heuristical evaluation   */
   struct eval_cell* lson;       /* Successors in ordered tree */
   struct eval_cell* rson;
}SimpleEvalCell, *SimpleEval_p;

typedef struct eval_cell
{
   int               eval_no;    /* Number of simple evaluations */
   long              eval_count; /* Evaluation cell count, used as
                FIFO tiebreaker */
   void*             object;     /* Evaluated object.*/
   SimpleEvalCell    evals[];
}EvalCell, *Eval_p;


/*---------------------------------------------------------------------*/
/*        Macros for a common interface with old evaluations           */
/*---------------------------------------------------------------------*/

//#define EvalsFree(eval) EvalsFree(eval)
//#define EvalTreeFindSmallestWrap(root, pos) EvalTreeFindSmallest((root), (pos))
//#define EvalTreePrintInOrderWrap(file, root, pos) EvalTreePrintInOrder(file, root, pos)

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern long EvaluationCounter;

#define EVAL_SIZE(eval_no) (sizeof(EvalCell)+((eval_no)*sizeof(SimpleEvalCell)))
#define EvalCellAlloc(eval_no)   (EvalCell*)SizeMalloc(EVAL_SIZE(eval_no))
#define EvalCellFree(junk, eval_no) SizeFree(junk, EVAL_SIZE(eval_no))

#ifdef CONSTANT_MEM_ESTIMATE
#define EVAL_MEM(eval_no) (32+(4*(eval_no)))
#else
#define EVAL_MEM(eval_no) (MEMSIZE(EvalCell)+(EVAL_SIZE((eval_no))))
#endif

Eval_p   EvalsAlloc(int eval_no);
void     EvalsFree(Eval_p junk);

void     EvalPrint(FILE* out, Eval_p list, int pos);
void     EvalPrintComment(FILE* out, Eval_p list, int pos);
void     EvalListPrint(FILE* out, Eval_p list);
void     EvalListPrintComment(FILE* out, Eval_p list);

void     EvalSetPriority(Eval_p list, EvalPriority priority);
void     EvalListChangePriority(Eval_p list, EvalPriority diff);

bool     EvalGreater(Eval_p ev1, Eval_p ev2, int pos);
long     EvalCompare(Eval_p ev1, Eval_p ev2, int pos);

Eval_p   EvalTreeInsert(Eval_p *root, Eval_p newnode, int pos);
Eval_p   EvalTreeFind(Eval_p *root, Eval_p key, int pos);
Eval_p   EvalTreeExtractEntry(Eval_p *root, Eval_p key, int pos);
bool     EvalTreeDeleteEntry(Eval_p *root, Eval_p key, int pos);
Eval_p   EvalTreeFindSmallest(Eval_p root, int pos);

/* AVL_TRAVERSE_DECLARATION(EvalTree,Eval_p) */

#define EvalTreeTraverseExit(stack) PStackFree(stack)

PStack_p EvalTreeTraverseInit(Eval_p root, int pos);
Eval_p   EvalTreeTraverseNext(PStack_p state, int pos);

void EvalTreePrintInOrder(FILE* out, Eval_p tree, int pos);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





