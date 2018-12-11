/*-----------------------------------------------------------------------

  File  : ccl_freqvectors.h

  Author: Stephan Schulz

  Contents

  Functions for handling frequency count vectors and permutation
  vectors.

  2003-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jul  8 21:48:35 CEST 2003

  -----------------------------------------------------------------------*/

#ifndef CCL_FREQVECTORS

#define CCL_FREQVECTORS

#include <clb_pdarrays.h>
#include <clb_fixdarrays.h>
#include <clb_regmem.h>
#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef FixedDArray_p PermVector_p;

typedef struct tuple3_cell
{
   long pos;
   long diff;
   long value;
}Tuple3Cell;

#define FVINDEX_MAX_FEATURES_DEFAULT 17  /* Maximal lenght of feature vector */
#define FVINDEX_SYMBOL_SLACK_DEFAULT 0   /* Reserve symbols for splitting */

typedef struct freq_vector_cell
{
   long size;        /* How many fields? */
   long *array;
   Clause_p clause; /* Just an unprotected reference */
}FreqVectorCell, *FreqVector_p, *FVPackedClause_p;

/* Where do the symbol-specific features in classival FV-Vectors
 * begin? */
#define FV_CLAUSE_FEATURES 2

typedef enum
{
   FVINoFeatures,
   FVIACFeatures,
   FVISSFeatures,
   FVIAllFeatures,
   FVIBillFeatures,
   FVIBillPlusFeatures,
   FVIACFold,
   FVIACStagger,
   FVICollectFeatures,
}FVIndexType;


/* Describe how to assemble a feature vector out of a full signature
 * feature vector. */

typedef struct fv_collect_cell
{
   FVIndexType features;
   bool  use_litcount;       /* Use pos_lit_no/neg_lit_no */
   long* assembly_vector;    /* Mapping from full positions to reduced
                                positions */
   long  ass_vec_len;        /* Size of the assembly vector */
   long  res_vec_len;        /* How long is the result? */
   /* The rest describe how to handle index values that are larger
      than  ass_vec_len. If _mod is zero, the value is discarded,
      otherwise it is added to  _offset+(f_code%_mod) */
   long  pos_count_base;
   long  pos_count_offset;
   long  pos_count_mod;
   long  neg_count_base;
   long  neg_count_offset;
   long  neg_count_mod;
   long  pos_depth_base;
   long  pos_depth_offset;
   long  pos_depth_mod;
   long  neg_depth_base;
   long  neg_depth_offset;
   long  neg_depth_mod;
   /* Legacy parameters for classical implementation. These are not
    * supported by the allocator and must be overwritten manually. */
   long        max_symbols;
}FVCollectCell, *FVCollect_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PERF_CTR_DECL(FreqVecTimer);

#define PermVectorAlloc(size) FixedDArrayAlloc(size)
#define PermVectorFree(junk)  FixedDArrayFree(junk)
#define PermVectorCopy(vec)   FixedDArrayCopy(vec)
#define PermVectorPrint(out,vec) FixedDArrayPrint((out),(vec))

PermVector_p PermVectorComputeInternal(FreqVector_p fmax, FreqVector_p fmin,
                                       FreqVector_p sums,
                                       long max_len,
                                       bool eliminate_uninformative);


#define FreqVectorCellAlloc()    (FreqVectorCell*)SizeMalloc(sizeof(FreqVectorCell))
#define FreqVectorCellFree(junk) SizeFree(junk, sizeof(FreqVectorCell))


#define FVACCompatSize(size)    ((size+1)*2+FV_CLAUSE_FEATURES)
#define FVSSCompatSize(size)    ((size+1)*2)
#define FVFullSize(size)        ((size+1)*4+FV_CLAUSE_FEATURES)
#define FVSize(size, features) (((features)==FVIACFeatures)?FVACCompatSize(size): \
                                (((features)==FVISSFeatures)?FVSSCompatSize(size): \
                                 FVFullSize(size)))


#define FVCollectCellAlloc()    (FVCollectCell*)SizeMalloc(sizeof(FVCollectCell))
#define FVCollectCellFree(junk) SizeFree(junk, sizeof(FVCollectCell))

FreqVector_p FreqVectorAlloc(long size);

void         FreqVectorFreeReal(FreqVector_p junk);
#ifndef NDEBUG
#define FreqVectorFree(junk) FreqVectorFreeReal(junk);junk=NULL
#else
#define FreqVectorFree(junk) FreqVectorFreeReal(junk)
#endif

void         FreqVectorInitialize(FreqVector_p vec, long value);

void         FreqVectorPrint(FILE* out, FreqVector_p vec);

void VarFreqVectorAddVals(FreqVector_p vec, long symbols, FVIndexType features,
                          Clause_p clause);
FreqVector_p VarFreqVectorCompute(Clause_p clause, FVCollect_p cspec);
FreqVector_p OptimizedVarFreqVectorCompute(Clause_p clause,
                                           PermVector_p perm,
                                           FVCollect_p cspec);

void FVCollectInit(FVCollect_p handle,
                   FVIndexType features,
                   bool  use_litcount,
                   long  ass_vec_len,
                   long  res_vec_len,
                   long  pos_count_base,
                   long  pos_count_offset,
                   long  pos_count_mod,
                   long  neg_count_base,
                   long  neg_count_offset,
                   long  neg_count_mod,
                   long  pos_depth_base,
                   long  pos_depth_offset,
                   long  pos_depth_mod,
                   long  neg_depth_base,
                   long  neg_depth_offset,
                   long  neg_depth_mod);


FVCollect_p FVCollectAlloc(FVIndexType features,
                           bool  use_litcount,
                           long  ass_vec_len,
                           long  res_vec_len,
                           long  pos_count_base,
                           long  pos_count_offset,
                           long  pos_count_mod,
                           long  neg_count_base,
                           long  neg_count_offset,
                           long  neg_count_mod,
                           long  pos_depth_base,
                           long  pos_depth_offset,
                           long  pos_depth_mod,
                           long  neg_depth_base,
                           long  neg_depth_offset,
                           long  neg_depth_mod);

void FVCollectFree(FVCollect_p junk);

FreqVector_p FVCollectFreqVectorCompute(Clause_p clause, FVCollect_p cspec);

FVCollect_p BillFeaturesCollectAlloc(Sig_p sig, long len);
FVCollect_p BillPlusFeaturesCollectAlloc(Sig_p sig, long len);


FVPackedClause_p FVPackClause(Clause_p clause, PermVector_p perm,
                              FVCollect_p cspec);
Clause_p         FVUnpackClause(FVPackedClause_p pack);

void             FVPackedClauseFreeReal(FVPackedClause_p pack);
#ifndef NDEBUG
#define FVPackedClauseFree(junk) FVPackedClauseFreeReal(junk);junk=NULL
#else
#define FVPackedClauseFree(junk) FVPackedClauseFreeReal(junk)
#endif


void FreqVectorAdd(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2);
void FreqVectorMulAdd(FreqVector_p dest, FreqVector_p s1, long f1,
                      FreqVector_p s2, long f2);
#define FreqVectorSub(dest, s1, s2) FreqVectorMulAdd((dest),(s1), 1, (s2), -1)
void FreqVectorMax(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2);
void FreqVectorMin(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
