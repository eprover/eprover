/*-----------------------------------------------------------------------

File  : ccl_overlap_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A simple index mapping symbols to ClauseTPos trees.
  See .c file for details on functionality.

Changes

<1> Thu Jun  3 11:30:36 CEST 2010
    New

-----------------------------------------------------------------------*/

#include <ccl_clauses.h>
#include <clb_intmap.h>

#ifndef CCL_EXT_DEC_IDX

#define CCL_EXT_DEC_IDX


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef IntMap_p ExtIndex_p;
#define TYPE_EXT_ELIGIBLE(t) (TypeIsBool(t) || TypeIsArrow((t)))



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ExtIdxAlloc()   IntMapAlloc()

bool TermHasExtEligSubterm(Term_p t);


void CollectExtSupFromPos(Clause_p cl, PStack_p pos_stack);
void CollectExtSupIntoPos(Clause_p cl, PStack_p pos_stack);

void ExtIndexInsertIntoClause(ExtIndex_p into_index, Clause_p cl, int max_depth);
void ExtIndexDeleteIntoClause(ExtIndex_p into_index, Clause_p cl);
void ExtIndexInsertFromClause(ExtIndex_p into_index, Clause_p cl, int max_depth);
void ExtIndexDeleteFromClause(ExtIndex_p into_index, Clause_p cl);
void ExtIndexFree(ExtIndex_p into_index);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





