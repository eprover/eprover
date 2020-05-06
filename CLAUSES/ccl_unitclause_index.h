/*-----------------------------------------------------------------------

File  : ccl_unitclause_index.h

Author: Constantin Ruhdorfer

Contents

  A simple index for unitclauses.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_UNITCLAUSESE_INDEXES

#define CCL_UNITCLAUSESE_INDEXES

#include <ccl_clauses.h>
#include <cte_fp_index.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
Datastructure that indexes unit clauses by representing them as a single 
equals term (e.g. =(lterm, rterm)). This indexing technique should use a
special Fingerprintindexing function that ignores the first symbol as it 
is always the same.
The leaves store the indexed term.
-----------------------------------------------------------------------*/
typedef FPIndex_p UCIndex_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
bool UCIndexDeleteClause(UCIndex_p index, Clause_p clause);
bool UCIndexInsertClause(UCIndex_p index, Clause_p clause);
long UCIndexFindSubsumedCandidates(UCIndex_p index, Clause_p clause, 
                                   PStack_p candidates);
void UCIndexFreeWrapper(void *junk);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/
bool uc_index_insert_cell(PTree_p *root, Clause_p clause);
bool uc_index_delete_clause_cell(PTree_p *root, Clause_p indexed);
bool uc_index_delete_indexed_clause(UCIndex_p index, Term_p indexedterm,
                                    Clause_p indexed);
bool uc_index_insert(UCIndex_p index, Term_p indexterm, Clause_p payload); 
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/