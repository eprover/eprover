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
#include <ccl_unitclause_index.h>
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
}GlobalIndices, *GlobalIndices_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PERF_CTR_DECL(PMIndexTimer);
PERF_CTR_DECL(BWRWIndexTimer);


void GlobalIndicesNull(GlobalIndices_p indices);
void GlobalIndicesInit(GlobalIndices_p indices,
                       Sig_p sig,
                       char* rw_bw_index_type,
                       char* pm_from_index_type,
                       char* pm_into_index_type);

void GlobalIndicesFreeIndices(GlobalIndices_p indices);
void GlobalIndicesReset(GlobalIndices_p indices);


void GlobalIndicesInsertClause(GlobalIndices_p indices, Clause_p clause);
void GlobalIndicesDeleteClause(GlobalIndices_p indices, Clause_p clause);
void GlobalIndicesInsertClauseSet(GlobalIndices_p indices, ClauseSet_p set);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





