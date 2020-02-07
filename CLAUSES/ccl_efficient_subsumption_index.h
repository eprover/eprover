/*-----------------------------------------------------------------------

File  : ccl_efficient_subsumption_index.h

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_EFFICIENT_SUBSUMPTION_INDEX
#define CCL_EFFICIENT_SUBSUMPTION_INDEX

#include <ccl_fcvindexing.h>
#include <ccl_unitclause_index.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct efficient_subsumption_index
{
   FVIAnchor_p       fvindex;          /* Used for non-unit subsumption */
   UnitclauseIndex_p unitclasue_index; /* Used for unit clauses subsuption */
} EfficientSubsumptionIndex, *EfficientSubsumptionIndex_p;

typedef long REWRITE_CONSTANT;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EfficientSubsumptionIndexAllocRaw()\
        (EfficientSubsumptionIndex*)SizeMalloc(sizeof(EfficientSubsumptionIndex))
#define EfficientSubsumptionIndexFreeRaw(junk) SizeFree(junk, sizeof(junk))

// TODO: FPIndexStorage?
// #define EfficientSubsumptionIndexStorage(index)
//         (FVIndexStorage(index->fvindex))

EfficientSubsumptionIndex_p EfficientSubsumptionIndexAlloc(FVCollect_p cspec,
                                                         PermVector_p perm);
void EfficientSubsumptionIndexFree(EfficientSubsumptionIndex_p clauseset_indexes);
void EfficientSubsumptionIndexUnitClauseIndexInit(EfficientSubsumptionIndex_p index,
                                                 Sig_p sig, 
                                                 char* unitclause_index_type);
void EfficientSubsumptionIndexInsertClause(EfficientSubsumptionIndex_p index, 
                                          Clause_p clause);
Clause_p ClausesetIndexDeleteEntry(EfficientSubsumptionIndex_p index, 
                                   Clause_p junk);
void RewriteConstants(Clause_p clause);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

Term_p RewriteConstantsOnTerm(Term_p source, VarBank_p vars, 
                              DerefType deref);
Term_p RewriteConstantsOnTermCell(Term_p source);

void EfficentSubsumptionIndexInsert(EfficentSubsumptionIndex_p index, 
                                    FVPackedClause_p newclause);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif