/*-----------------------------------------------------------------------

File  : ccl_efficent_subsumption_index.h

Author: Constantin Ruhdorfer

Contents

  Interface for indexing clauses for subsumption.

Copyright 2019-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_EFFICENT_SUBSUMPTION_INDEX
#define CCL_EFFICENT_SUBSUMPTION_INDEX

#include <ccl_fcvindexing.h>
#include <ccl_unitclause_index.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct efficent_subsumption_index
{
   FVIAnchor_p       fvindex;          /* Used for non-unit subsumption */
   UnitclauseIndex_p unitclasue_index; /* Used for unit clauses subsuption */
} EfficentSubsumptionIndex, *EfficentSubsumptionIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EfficentSubsumptionIndexAllocRaw()\
        (EfficentSubsumptionIndex*)SizeMalloc(sizeof(EfficentSubsumptionIndex))
#define EfficentSubsumptionIndexFreeRaw(junk) SizeFree(junk, sizeof(junk))

// TODO: FPIndexStorage?
// #define EfficentSubsumptionIndexStorgae(index)
//         (FVIndexStorage(index->fvindex))

EfficentSubsumptionIndex_p EfficentSubsumptionIndexAlloc(FVCollect_p cspec,
                                                         PermVector_p perm);
void EfficentSubsumptionIndexFree(EfficentSubsumptionIndex_p clauseset_indexes);
void EfficentSubsumptionIndexUnitClauseIndexInit(EfficentSubsumptionIndex_p index,
                                                 Sig_p sig, 
                                                 char* unitclause_index_type);
void EfficentSubsumptionIndexInsertClause(EfficentSubsumptionIndex_p index, 
                                          Clause_p clause);
Clause_p ClausesetIndexDeleteEntry(EfficentSubsumptionIndex_p index, 
                                   Clause_p junk);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

void EfficentSubsumptionIndexInsert(EfficentSubsumptionIndex_p index, 
                                    FVPackedClause_p newclause);

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif