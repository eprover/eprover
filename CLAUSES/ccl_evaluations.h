/*-----------------------------------------------------------------------

File  : ccl_evaluations.h

Author: Stephan Schulz

Contents
 
  Data type for representing evaluations of clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr  9 02:00:51 MET DST 1998
    New
<2> Thu Jan 28 00:58:19 MET 1999
    Replaced AVL trees with Splay-Trees

-----------------------------------------------------------------------*/

#ifndef CCL_EVALUATIONS

#define CCL_EVALUATIONS

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

typedef struct eval_cell
{
   EvalPriority      priority;   /* Technical considerations */
   float             heuristic;  /* Heuristical evaluation   */
   long              eval_count; /* Evaluation cell count, used as
				    FIFO tiebreaker */
   void*             object;     /* Evaluated object.*/
   struct eval_cell* next;       /* For chaining together multiple
				    evaluations */
   struct eval_cell* lson;       /* Successors in ordered tree */
   struct eval_cell* rson;
}EvalCell, *Eval_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern long EvaluationCounter;

#define EvalCellAlloc()   (EvalCell*)SizeMalloc(sizeof(EvalCell))
#define EvalCellFree(junk) SizeFree(junk, sizeof(EvalCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define EVALCELL_MEM 32
#else
#define EVALCELL_MEM MEMSIZE(EvalCell)
#endif

Eval_p   EvalAlloc(void);

void     EvalPrint(FILE* out, Eval_p eval);
void     EvalPrintComment(FILE* out, Eval_p eval);
void     EvalListPrint(FILE* out, Eval_p list);
void     EvalListPrintComment(FILE* out, Eval_p list);
void     EvalListSetPriority(Eval_p list, EvalPriority priority);
void     EvalListChangePriority(Eval_p list, EvalPriority diff);

bool     EvalGreater(Eval_p ev1, Eval_p ev2);
long     EvalCompare(Eval_p ev1, Eval_p ev2);

void     EvalListFree(Eval_p junk);
void     EvalTreeFree(Eval_p junk);
Eval_p   EvalTreeInsert(Eval_p *root, Eval_p newnode);
Eval_p   EvalTreeFind(Eval_p *root, Eval_p key);
Eval_p   EvalTreeExtractEntry(Eval_p *root, Eval_p key);
bool     EvalTreeDeleteEntry(Eval_p *root, Eval_p key);
Eval_p   EvalTreeFindSmallest(Eval_p root);

AVL_TRAVERSE_DECLARATION(EvalTree,Eval_p)

#define EvalTreeTraverseExit(stack) PStackFree(stack)

/* PStack_p EvalTreeTraverseInit(Eval_p root);
Eval_p   EvalTreeTraverseNext(PStack_p state);
*/

void     EvalTreePrintInOrder(FILE* out, Eval_p tree);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





