/*-----------------------------------------------------------------------

File  : ccl_global_indices.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code abstracting several (optional) indices into one structure.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri May  7 21:13:39 CEST 2010
    New

-----------------------------------------------------------------------*/

#ifndef CCL_GLOBAL_INDICES

#define CCL_GLOBAL_INDICES

#include <ccl_subterm_index.h>
#include <ccl_overlap_index.h>
#include <ccl_ext_index.h>
#include <ccl_clausesets.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct global_indices_cell
{
   char              rw_bw_index_type[MAX_PM_INDEX_NAME_LEN];
   char              pm_from_index_type[MAX_PM_INDEX_NAME_LEN];
   char              pm_into_index_type[MAX_PM_INDEX_NAME_LEN];
   char              pm_negp_index_type[MAX_PM_INDEX_NAME_LEN];
   Sig_p             sig;
   SubtermIndex_p    bw_rw_index;
   OverlapIndex_p    pm_from_index;
   OverlapIndex_p    pm_into_index;
   OverlapIndex_p    pm_negp_index;
#ifdef ENABLE_LFHO
   ExtIndex_p        ext_sup_into_index;
   ExtIndex_p        ext_sup_from_index;
   int               ext_rules_max_depth;
#endif
}GlobalIndices, *GlobalIndices_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PERF_CTR_DECL(PMIndexTimer);
PERF_CTR_DECL(BWRWIndexTimer);

#ifdef ENABLE_LFHO
#define GetExtIntoIdx(g)    (g)->ext_sup_into_index
#define GetExtFromIdx(g)    (g)->ext_sup_from_index
#define GetExtMaxDepth(g)   (g)->ext_rules_max_depth
#define SetExtIntoIdx(g, v) (g)->ext_sup_into_index = (v)
#define SetExtFromIdx(g, v) (g)->ext_sup_from_index = (v)
#define SetExtMaxDepth(g, v)   (g)->ext_rules_max_depth = (v)
#else
#define GetExtIntoIdx(g)   NULL
#define GetExtFromIdx(g)   NULL
#define GetExtMaxDepth(g)  0
#define SetExtIntoIdx(g, v)   /* */
#define SetExtFromIdx(g, v)   /* */
#define SetExtMaxDepth(g, v)  /* */
#endif


void GlobalIndicesNull(GlobalIndices_p indices);
void GlobalIndicesInit(GlobalIndices_p indices,
                       Sig_p sig,
                       char* rw_bw_index_type,
                       char* pm_from_index_type,
                       char* pm_into_index_type,
                       int   ext_rules_max_depth);

void GlobalIndicesFreeIndices(GlobalIndices_p indices);
void GlobalIndicesReset(GlobalIndices_p indices);


void GlobalIndicesInsertClause(GlobalIndices_p indices, Clause_p clause, bool lambda_demod);
void GlobalIndicesDeleteClause(GlobalIndices_p indices, Clause_p clause, bool lambda_demod);
void GlobalIndicesInsertClauseSet(GlobalIndices_p indices, ClauseSet_p set, bool lambda_demod);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





