/*-----------------------------------------------------------------------

  File  : ccl_freqvectors.c

  Author: Stephan Schulz

  Contents

  Algorithms for frequency count vectors.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Createrd: Tue Jul  8 21:50:44 CEST 2003

  -----------------------------------------------------------------------*/

#include "ccl_freqvectors.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(FreqVecTimer);

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: tuple_3_compare_23lex()
//
//   Compare 2 tuple-2 cells lexicographically, with diff more
//   significant than value, which is more significant than pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int tuple3_compare_23lex(const void* tuple1, const void* tuple2)
{
   const Tuple3Cell *t1 = (const Tuple3Cell *)tuple1;
   const Tuple3Cell *t2 = (const Tuple3Cell *)tuple2;

   if(t1->diff < t2->diff)
   {
      return -1;
   }
   if(t1->diff > t2->diff)
   {
      return 1;
   }
   if(t1->value < t2->value)
   {
      return -1;
   }
   if(t1->value > t2->value)
   {
      return 1;
   }
   if(t1->pos > t2->pos)
   {
      return -1;
   }
   if(t1->pos < t2->pos)
   {
      return 1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: gather_feature_vec()
//
//   Gather a feature from a full feature vector according to
//   cspec.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void gather_feature_vec(FVCollect_p cspec, long* full_vec,
                               FreqVector_p vec, long findex)
{
   long resindex = -1, base = 0, offset = 0, mod = 0;

   if(findex < cspec->ass_vec_len)
   {
      resindex = cspec->assembly_vector[findex];
   }
   else
   {
      switch(findex%4)
      {
      case 0:
            base   = cspec->pos_count_base;
            offset = cspec->pos_count_offset;
            mod    = cspec->pos_count_mod;
            break;
      case 1:
            base   = cspec->pos_depth_base;
            offset = cspec->pos_depth_offset;
            mod    = cspec->pos_depth_mod;
            break;
      case 2:
            base   = cspec->neg_count_base;
            offset = cspec->neg_count_offset;
            mod    = cspec->neg_count_mod;
            break;
      case 3:
            base   = cspec->neg_depth_base;
            offset = cspec->neg_depth_offset;
            mod    = cspec->neg_depth_mod;
            break;
      default:
            assert(false);
            break;
      }
      if(mod)
      {
         resindex = base+(offset+(findex/4))%mod;
      }
   }
   if(resindex != -1)
   {
      vec->array[resindex]+=full_vec[findex];
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PermVectorComputeInternal()
//
//   Find a "good" permutation (and selection) vector for
//   FVIndexing by:
//   - Ordering features from lesser to higher informativity
//   - Selecting the best max_len features
//   - Optionally drop features that have no projected informational
//     value.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PermVector_p PermVectorComputeInternal(FreqVector_p fmax, FreqVector_p fmin,
                                       FreqVector_p fsum,
                                       long max_len,
                                       bool eliminate_uninformative)
{
   Tuple3Cell *array;
   long i, size, start=0, start1=0, diff;
   PermVector_p handle;

   assert(fsum->size == fmax->size);
   assert(fsum->size == fmin->size);

   array = SizeMalloc(fsum->size * sizeof(Tuple3Cell));
   for(i=0; i<fsum->size; i++)
   {
      array[i].pos = i;
      diff = fmax->array[i]-fmin->array[i];
      array[i].diff  = diff;
      array[i].value = fsum->array[i];
   }
   qsort(array, fsum->size, sizeof(Tuple3Cell), tuple3_compare_23lex);

   if(fsum->size >  max_len)
   {
      start = fsum->size - max_len;
   }
   if(eliminate_uninformative)
   {
      for(i=0; i<fsum->size && !array[i].diff; i++)
      {
         /* Intentionally empty */
      };
      start1 = i;
   }
   start = MAX(start, start1);
   if(start == fsum->size)
   {
      start--;
   }
   size = fsum->size - start;

   handle = PermVectorAlloc(size);

   for(i=0; i < size; i++)
   {
      handle->array[i] = array[i+start].pos;
   }
   SizeFree(array, fsum->size * sizeof(Tuple3Cell));
   /* PermVectorPrint(GlobalOut, handle); */
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorAlloc()
//
//   Allocate a frequency vector that can hold up to sig_start
//   non function symbol count features and sig_count function symbol
//   counts (in both positive and negative variety).
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

FreqVector_p FreqVectorAlloc(long size)
{
   FreqVector_p handle = FreqVectorCellAlloc();

   /* printf("Size: %ld\n", size); */
   handle->size         = size;
   handle->array  = SizeMalloc(sizeof(long)*handle->size);
   FreqVectorInitialize(handle, 0);
   handle->clause = NULL;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorFree()
//
//   Free a frequency vector.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FreqVectorFreeReal(FreqVector_p junk)
{
   assert(junk);

   if(junk->array)
   {
      SizeFree(junk->array, sizeof(long)*junk->size);
#ifndef NDEBUG
      junk->array = NULL;
      junk->clause = NULL;
#endif
   }
   FreqVectorCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorInitialize()
//
//   Store value in all fields of vec.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorInitialize(FreqVector_p vec, long value)
{
   long i;

   for(i=0; i<vec->size;i++)
   {
      vec->array[i] = value;
   }
}

/*-----------------------------------------------------------------------
//
// Function: FreqVectorPrint()
//
//   Print a frequency vector.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FreqVectorPrint(FILE* out, FreqVector_p vec)
{
   long i;

   assert(vec);
   if(vec->clause)
   {
      fprintf(out, COMCHAR" FV for: ");
      ClausePrint(out, vec->clause, true);
      fprintf(out, "\n");
   }
   else
   {
      fprintf(out, COMCHAR" FV, no clause given.\n");
   }
   fprintf(out, COMCHAR" FV(len=%ld):", vec->size);
   for(i=0; i<vec->size; i++)

   {
      fprintf(out, " %ld", vec->array[i]);
   }
   fprintf(out, "\n");
}


/*-----------------------------------------------------------------------
//
// Function: VarFreqVectorAddVals()
//
//   Add values for up to symbol type features to the freq vector.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarFreqVectorAddVals(FreqVector_p vec, long symbols, FVIndexType features,
                          Clause_p clause)
{
   long *unused, *pfreqstart, *nfreqstart, *pdepthstart, *ndepthstart;
   long unused_size = 0;
   Eqn_p handle;

   assert(clause);
   assert((features == FVIACFeatures) ||
          (features == FVISSFeatures) ||
          (features == FVIAllFeatures));
   assert(vec);
   assert(vec->size == FVSize(symbols, features));

   switch(features)
   {
   case FVIACFeatures:
         vec->array[0] += clause->pos_lit_no;
         vec->array[1] += clause->neg_lit_no;
         unused_size = symbols+1;
         unused = SizeMalloc(sizeof(long)*unused_size);
         pdepthstart = ndepthstart = unused;
         nfreqstart = &(vec->array[FV_CLAUSE_FEATURES]);
         pfreqstart = &(vec->array[FV_CLAUSE_FEATURES+1*(symbols+1)]);
         break;
   case FVISSFeatures:
         unused_size = symbols+1;
         unused = SizeMalloc(sizeof(long)*unused_size);
         pfreqstart = nfreqstart = unused;
         ndepthstart = &(vec->array[0]);
         pdepthstart = &(vec->array[0+1*(symbols+1)]);
         break;
   case FVIAllFeatures:
         vec->array[0] += clause->pos_lit_no;
         vec->array[1] += clause->neg_lit_no;
         unused = NULL;
         nfreqstart  = &(vec->array[FV_CLAUSE_FEATURES]);
         pfreqstart  = &(vec->array[FV_CLAUSE_FEATURES+1*(symbols+1)]);
         pdepthstart = &(vec->array[FV_CLAUSE_FEATURES+2*(symbols+1)]);
         ndepthstart = &(vec->array[FV_CLAUSE_FEATURES+3*(symbols+1)]);
         break;
   default:
         assert(features == FVINoFeatures);
         assert(false);
         return; /* Cheapest way to fix compiler warning */
   }
   if(unused)
   { /* Stiffle insure warnings - we don't use unused (duh!), but
        insure does not know that */
      long i;

      for(i=0; i<unused_size; i++)
      {
         unused[i] = 0;
      }
   }
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         EqnAddSymbolFeaturesLimited(handle,
                                     pfreqstart,
                                     pdepthstart,
                                     symbols);
      }
      else
      {
         EqnAddSymbolFeaturesLimited(handle,
                                     nfreqstart,
                                     ndepthstart,
                                     symbols);
      }
   }
   if(unused)
   {
      SizeFree(unused, sizeof(long)*unused_size);
   }
}


/*-----------------------------------------------------------------------
//
// Function: VarFreqVectorCompute()
//
//   Allocate and return a frequency vector for clause based on the
//   other supplied parameters.
//
// Global Variables: -
//
// Side Effects    : Memory allocation.
//
/----------------------------------------------------------------------*/

FreqVector_p VarFreqVectorCompute(Clause_p clause, FVCollect_p cspec)
{
   long size;
   FreqVector_p vec;

   assert(clause);

   assert((cspec->features == FVIACFeatures) ||
          (cspec->features == FVISSFeatures) ||
          (cspec->features == FVIAllFeatures) ||
          (cspec->features == FVICollectFeatures));

   if(cspec->features == FVICollectFeatures)
   {
      vec = FVCollectFreqVectorCompute(clause, cspec);
   }
   else
   {
      size = FVSize(cspec->max_symbols, cspec->features);

      vec = FreqVectorAlloc(size);
      vec->clause = clause;
      FreqVectorInitialize(vec, 0);
      VarFreqVectorAddVals(vec, cspec->max_symbols,
                           cspec->features, clause);
   }
   return vec;
}


/*-----------------------------------------------------------------------
//
// Function: OptimizedVarFreqVectorCompute()
//
//   Compute an "optimized" frequency count vector, based on a given
//   permutation vector. If no permutation vector is given, return a
//   VarFreqVector.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FreqVector_p OptimizedVarFreqVectorCompute(Clause_p clause,
                                           PermVector_p perm,
                                           FVCollect_p cspec)
{
   FreqVector_p vec, res;

   assert((cspec->features == FVIACFeatures) ||
          (cspec->features == FVISSFeatures) ||
          (cspec->features == FVIAllFeatures) ||
          (cspec->features == FVICollectFeatures));

   PERF_CTR_ENTRY(FreqVecTimer);


   /* printf("Symbols used: %ld\n", sig_symbols); */
   vec = VarFreqVectorCompute(clause, cspec);
   /* FreqVectorPrint(stderr, vec); */
   if(perm)
   {
      long i;

      res = FreqVectorAlloc(perm->size);
      for(i=0; i<perm->size; i++)
      {
         assert(perm->array[i]>=0);
         assert(perm->array[i]<vec->size);
         res->array[i] = vec->array[perm->array[i]];
      }
      res->clause = clause;
      FreqVectorFree(vec);
      PERF_CTR_EXIT(FreqVecTimer);
      return res;
   }
   PERF_CTR_EXIT(FreqVecTimer);
   return vec;
}


/*-----------------------------------------------------------------------
//
// Function: FVCollectInit()
//
//   Initialize an FVCollectCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

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
                   long  neg_depth_mod)
{
   long i;

   handle->features         = features;
   handle->use_litcount     = use_litcount;
   handle->ass_vec_len      = ass_vec_len;
   handle->res_vec_len      = res_vec_len;
   handle->assembly_vector  = SizeMalloc(sizeof(long)*ass_vec_len);
   for(i=0; i< ass_vec_len; i++)
   {
      handle->assembly_vector[i] = -1;
   }
   handle->pos_count_base   = pos_count_base;
   handle->pos_count_offset = pos_count_offset;
   handle->pos_count_mod    = pos_count_mod;
   handle->neg_count_base   = neg_count_base;
   handle->neg_count_offset = neg_count_offset;
   handle->neg_count_mod    = neg_count_mod;
   handle->pos_depth_base   = pos_depth_base;
   handle->pos_depth_offset = pos_depth_offset;
   handle->pos_depth_mod    = pos_depth_mod;
   handle->neg_depth_base   = neg_depth_base;
   handle->neg_depth_offset = neg_depth_offset;
   handle->neg_depth_mod    = neg_depth_mod;

   handle->max_symbols      = FVINDEX_MAX_FEATURES_DEFAULT;
}


/*-----------------------------------------------------------------------
//
// Function: FVCollectAlloc()
//
//   Allocate an initialized FVCollectCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

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
                           long  neg_depth_mod)
{
   FVCollect_p handle = FVCollectCellAlloc();

   FVCollectInit(handle,
                 features,
                 use_litcount,
                 ass_vec_len,
                 res_vec_len,
                 pos_count_base,
                 pos_count_offset,
                 pos_count_mod,
                 neg_count_base,
                 neg_count_offset,
                 neg_count_mod,
                 pos_depth_base,
                 pos_depth_offset,
                 pos_depth_mod,
                 neg_depth_base,
                 neg_depth_offset,
                 neg_depth_mod);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FVCollectFree()
//
//   Free a FVCollectCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FVCollectFree(FVCollect_p junk)
{
   SizeFree(junk->assembly_vector,sizeof(long)*junk->ass_vec_len);
   FVCollectCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: FVCollectFreqVectorCompute()
//
//   Compute a Feature Vector for the clause based on cspec.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FreqVector_p FVCollectFreqVectorCompute(Clause_p clause, FVCollect_p cspec)
{
   static size_t  full_vec_len = 0;
   static long*   full_vec     = NULL;

   FreqVector_p vec = FreqVectorAlloc(cspec->res_vec_len);

   vec->clause = clause;
   FreqVectorInitialize(vec, 0);

   if(!ClauseIsEmpty(clause))
   {
      PStack_p mod_stack = PStackAlloc();
      long max_fun = clause->literals->bank->sig->f_count;
      long findex;

      if(cspec->use_litcount)
      {
         vec->array[0] = clause->pos_lit_no;
         vec->array[1] = clause->neg_lit_no;
      }
      full_vec = RegMemProvide(full_vec, &full_vec_len, sizeof(long)*(max_fun+1)*4);

      ClauseAddSymbolFeatures(clause, mod_stack, full_vec);

      while(!PStackEmpty(mod_stack))
      {
         findex = PStackPopInt(mod_stack);
         gather_feature_vec(cspec, full_vec, vec, findex);
         full_vec[findex] = 0;
         gather_feature_vec(cspec, full_vec, vec, findex+1);
         full_vec[findex+1] = 0;
      }

      PStackFree(mod_stack);
   }
   return vec;
}



/*-----------------------------------------------------------------------
//
// Function: BillFeaturesCollectAlloc()
//
//   Generate a CollectSpec as follows
//   - positive literals
//   - negative literals
//   foreach relation symbol
//      positive occurrences
//      negative occurrences
//   foreach function symbol
//     positive occurrences
//     negative occurrences
//     positive maxdepth
//     negative maxdepth
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVCollect_p BillFeaturesCollectAlloc(Sig_p sig, long len)
{
   long p_no = SigCountSymbols(sig, true);
   long f_no = SigCountSymbols(sig, false);
   FunCode i, pos;
   FVCollect_p cspec;

   assert(len>2);

   while((2+2*p_no+4*f_no) > len)
   {
      if(p_no > f_no)
      {
         p_no--;
      }
      else
      {
         f_no--;
      }
   }

   cspec = FVCollectAlloc(FVICollectFeatures,
                          true,
                          (sig->f_count+1)*4+2,
                          len,
                          0, 0, 0,
                          0, 0, 0,
                          0, 0, 0,
                          0, 0, 0);
   pos = 2;
   for(i=sig->internal_symbols+1; p_no; i++)
   {
      /* printf("p = %ld (%s)\n", i, SigFindName(sig,i)); */
      if(!SigIsSpecial(sig, i) && SigIsPredicate(sig, i))
      {
         cspec->assembly_vector[4*i] = pos;
         pos++;
         cspec->assembly_vector[4*i+1] = pos;
         pos++;
         p_no--;
      }
   }

   for(i=sig->internal_symbols+1; f_no; i++)
   {
      /* printf("f = %ld (%s)\n", i, SigFindName(sig,i)); */
      if(!SigIsSpecial(sig, i) && SigIsFunction(sig,i))
      {
         cspec->assembly_vector[4*i] = pos;
         pos++;
         cspec->assembly_vector[4*i+1] = pos;
         pos++;
         cspec->assembly_vector[4*i+2] = pos;
         pos++;
         cspec->assembly_vector[4*i+3] = pos;
         pos++;
         f_no--;
      }
   }
   return cspec;
}

/*-----------------------------------------------------------------------
//
// Function: BillPlusFeaturesCollectAlloc()
//
//   Generate a CollectSpec as follows
//   - positive literals
//   - negative literals
//   foreach relation symbol
//      positive occurrences
//      negative occurrences
//   foreach function symbol
//     positive occurrences
//     negative occurrences
//     positive maxdepth
//     negative maxdepth
//   All overflow counts
//   All overflow depths
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVCollect_p BillPlusFeaturesCollectAlloc(Sig_p sig, long len)
{
   long p_no = SigCountSymbols(sig, true);
   long f_no = SigCountSymbols(sig, false);
   FunCode i, pos;
   FVCollect_p cspec;

   assert(len>2);

   while((6+2*p_no+4*f_no) > len)
   {
      if(p_no > f_no)
      {
         p_no--;
      }
      else
      {
         f_no--;
      }
   }

   cspec = FVCollectAlloc(FVICollectFeatures,
                          true,
                          (sig->f_count+1)*4+2,
                          len,
                          len-4, 0, 1,
                          len-3, 0, 1,
                          len-2, 0, 1,
                          len-1, 0, 1);
   pos = 2;
   for(i=sig->internal_symbols+1; p_no; i++)
   {
      /* printf("p = %ld (%s)\n", i, SigFindName(sig,i)); */
      if(!SigIsSpecial(sig, i) && SigIsPredicate(sig, i))
      {
         cspec->assembly_vector[4*i] = pos;
         pos++;
         cspec->assembly_vector[4*i+1] = pos;
         pos++;
         p_no--;
      }
   }

   for(i=sig->internal_symbols+1; f_no; i++)
   {
      /* printf("f = %ld (%s)\n", i, SigFindName(sig,i)); */
      if(!SigIsSpecial(sig, i) && SigIsFunction(sig,i))
      {
         cspec->assembly_vector[4*i] = pos;
         pos++;
         cspec->assembly_vector[4*i+1] = pos;
         pos++;
         cspec->assembly_vector[4*i+2] = pos;
         pos++;
         cspec->assembly_vector[4*i+3] = pos;
         pos++;
         f_no--;
      }
   }
   return cspec;
}



/*-----------------------------------------------------------------------
//
// Function: FVPackClause()
//
//   If index is an index, compute and return a StandardFreqVector for
//   clause, otherwise pack clause into a dummy frequency vector cell
//   and return than.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FVPackedClause_p FVPackClause(Clause_p clause, PermVector_p perm,
                              FVCollect_p cspec)
{
   FVPackedClause_p res;

   if(cspec && (cspec->features != FVINoFeatures))
   {
      return OptimizedVarFreqVectorCompute(clause, perm, cspec);
   }
   res = FreqVectorCellAlloc();
   res->array = NULL;
   res->clause = clause;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FVUnpackClause()
//
//   Unpack a packed clause, i.e. return the clause and throw away the
//   container.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p FVUnpackClause(FVPackedClause_p pack)
{
   Clause_p res = pack->clause;

#ifndef NDEBUG
   pack->clause = NULL;
#endif
   FreqVectorFree(pack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FVPackedClauseFreeReal()
//
//   Fully free a packed clause.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FVPackedClauseFreeReal(FVPackedClause_p pack)
{
   if(pack->clause)
   {
      ClauseFree(pack->clause);
   }
   FreqVectorFree(pack);
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorAdd()
//
//   Component-wise addition of both sources. Guaranteed to work if
//   dest is a source (but not maximally efficient - who cares). Yes,
//   it's worth mentioning it ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorAdd(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = s1->array[i]+s2->array[i];
   }
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorMulAdd()
//
//   Component-wise addition of both weighted sources. Guaranteed to
//   work if dest is a source (but not maximally efficient - who
//   cares). Yes, it's worth mentioning it ;-)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorMulAdd(FreqVector_p dest, FreqVector_p s1, long f1,
                      FreqVector_p s2, long f2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = f1*s1->array[i]+f2*s2->array[i];
   }
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorMax()
//
//   Compute componentwise  max of vectors. See above.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorMax(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = MAX(s1->array[i],s2->array[i]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: FreqVectorMin()
//
//   Compute componentwise  min of vectors. See above.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void FreqVectorMin(FreqVector_p dest, FreqVector_p s1, FreqVector_p s2)
{
   long i;

   assert(s1 && s2 && dest);
   assert(s1->size == dest->size);
   assert(s2->size == dest->size);

   for(i=0; i<dest->size; i++)
   {
      dest->array[i] = MIN(s1->array[i],s2->array[i]);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
