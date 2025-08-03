/*-----------------------------------------------------------------------

  File  : che_to_weightgen.c

  Author: Stephan Schulz

  Contents

  Functions implementing several simple weight generation schemes for
  the KBO.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  <1> Created: Fri Sep 25 02:49:11 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_to_weightgen.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/



/* Cell for sorting by precedence */

typedef struct prec_rank_cell
{
   FunCode f_code;
   OCB_p   ocb; /* Needed for precedence comparisons */
}PrecRankCell, *PrecRank_p;



/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/* #define PRINT_FUNWEIGHTS */

#ifdef PRINT_FUNWEIGHTS

/*-----------------------------------------------------------------------
//
// Function: print_weight_array()
//
//   Print the function symbol weights.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_weight_array(FILE* out,OCB_p ocb)
{
   FunCode i;

   fprintf(out, COMCHAR" Ordering weights: ");
   for(i = 1; i<=ocb->sig->f_count; i++)
   {
      if(!SigIsSpecial(ocb->sig, i))
      {
         fprintf(out, "%s:%ld ", SigFindName(ocb->sig,i),
                 OCBFunWeight(ocb,i));
      }
   }
   fprintf(out, "\n");
}

#endif




/*-----------------------------------------------------------------------
//
// Function: prec_rank_cell_cmp()
//
//   Comparison function for sorting signatures by precedence.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int prec_rank_cell_cmp(const void* rc1, const void* rc2)
{
   PrecRank_p
      r1 = (const PrecRank_p)rc1,
      r2 = (const PrecRank_p)rc2;

   if(r1->f_code == r2->f_code)
   {
      return 0;
   }
   if(OCBFunCompare(r1->ocb, r1->f_code, r2->f_code)== to_lesser)
   {
      return -1;
   }
   else if(OCBFunCompare(r1->ocb, r2->f_code, r1->f_code)== to_lesser)
   {
      return 1;
   }
   return SigGetAlphaRank(r1->ocb->sig, r1->f_code)-
      SigGetAlphaRank(r1->ocb->sig, r2->f_code);
}


/*-----------------------------------------------------------------------
//
// Function: generate_precrank_weights()
//
//    Sort symbols by precedence, split them into n ranks, then assign
//    weights to each rank (lowest rank = 1, then up). Assumes a total
//    precedence (and will make it total by alpha-rank in a
//    pinch). Note to self: Is is always kosher? Better only try on
//    complete precedences (but then, most are).
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_precrank_weights(OCB_p ocb, float ranks)
{
   FunCode    i;
   PrecRank_p array;
   long       size = ocb->sig->f_count+1;
   long       symb_no;

   assert(ocb->precedence||ocb->prec_weights);

   array = SizeMalloc(size*sizeof(PrecRankCell));
   for(i=1; i<=ocb->sig->f_count; i++)
   {
      array[i].f_code = i;
      array[i].ocb = ocb;
   }
   qsort(&(array[SIG_TRUE_CODE+1]),
         size-(SIG_TRUE_CODE+1),
         sizeof(PrecRankCell),
         prec_rank_cell_cmp);

   symb_no = size-(SIG_TRUE_CODE+1);
   for(i=0; i<symb_no; i++)
   {
      *OCBFunWeightPos(ocb, array[i+SIG_TRUE_CODE+1].f_code) =
         ((i/(symb_no/ranks))+1)*W_DEFAULT_WEIGHT;
   }
   SizeFree(array, size*sizeof(PrecRankCell));
}



/*-----------------------------------------------------------------------
//
// Function: find_max_symbols()
//
//   Find all maximal (in the precedence) symbols in ocb->sig and
//   return a stack containing them.
//
// Global Variables: -
//
// Side Effects    : Allocation of the stack
//
/----------------------------------------------------------------------*/

static PStack_p find_max_symbols(OCB_p ocb)
{
   PStack_p res = PStackAlloc();
   PStackPointer j;
   FunCode i;
   CompareResult cmpres;
   bool max;

   assert(ocb&&(ocb->precedence||ocb->prec_weights));

   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      max = true;
      for(j = PStackGetSP(res); j--; )
      {
         cmpres = OCBFunCompare(ocb, i, PStackElementInt(res,j));

         if(cmpres == to_lesser)
         {  /* New candidate is dominated */
            max = false;
            break;
         }
         else if(cmpres == to_greater)
         {  /* New candidate dominates */
            PStackDiscardElement(res,j);
         }
      }
      if(max)
      {
         PStackPushInt(res, i);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: set_maximal_0()
//
//   Set the weight of the first non-constant maximal symbol in OCB to
//   0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void set_maximal_0(OCB_p ocb)
{
   assert(ocb->precedence||ocb->prec_weights);
   if(problemType == PROBLEM_HO)
   {
      return; // no checks if it is unary -- our KBO works only then
   }

   PStack_p maxsymbs = find_max_symbols(ocb);
   if(!PStackEmpty(maxsymbs))
   {
      FunCode first = PStackElementInt(maxsymbs, 0);
      PStackPointer i;
      for(i=1; i<PStackGetSP(maxsymbs); i++)
      {
         if(PStackElementInt(maxsymbs, i)<first)
         {
            first = PStackElementInt(maxsymbs, i);
         }
      }
      *OCBFunWeightPos(ocb, first) = 0;
   }
   PStackFree(maxsymbs);
}

/*-----------------------------------------------------------------------
//
// Function: set_maximal_unary_0()
//
//   Set the weight of the first unary maximal symbol in OCB to
//   0.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void set_maximal_unary_0(OCB_p ocb)
{
   assert(ocb->precedence||ocb->prec_weights);

   PStack_p maxsymbs = find_max_symbols(ocb);
   if(!PStackEmpty(maxsymbs))
   {
      FunCode first = PStackElementInt(maxsymbs, 0);
      PStackPointer i;
      for(i=1; i<PStackGetSP(maxsymbs); i++)
      {
         if(PStackElementInt(maxsymbs, i)<first)
         {
            first = PStackElementInt(maxsymbs, i);
         }
      }
      if(SigFindArity(ocb->sig, first) == 1)
      {
         *OCBFunWeightPos(ocb, first) = 0;
      }
   }
   PStackFree(maxsymbs);
}


/*-----------------------------------------------------------------------
//
// Function: generate_constant_weights()
//
//   Assign the constant W_DEFAULT_WEIGHT to all smbols.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_constant_weights(OCB_p ocb)
{
   FunCode i;

   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      *OCBFunWeightPos(ocb, i) = W_DEFAULT_WEIGHT;
   }
}

/*-----------------------------------------------------------------------
//
// Function: generate_selmax_weights()
//
//   Assign weight W_DEFAULT_WEIGHT to all symbols except the first
//   maximal one, which get weight 0. Constants alway get
//   W_DEFAULT_WEIGHT.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_selmax_weights(OCB_p ocb)
{
   generate_constant_weights(ocb);
   set_maximal_0(ocb);  //no checks if symbol is unary
}


/*-----------------------------------------------------------------------
//
// Function: generate_arity_weights()
//
//   Generate arity-based weights for function symbols.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void generate_arity_weights(OCB_p ocb, TOWeightGenMethod method)
{
   FunCode i;
   int arity, weight = W_DEFAULT_WEIGHT, maxarity;

   maxarity = SigFindMaxUsedArity(ocb->sig);
   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      arity = SigFindArity(ocb->sig, i);
      switch(method)
      {
      case WArityWeight:
      case WArityMax0:
            weight = arity+1;
            break;
      case WModArityWeight:
      case WModArityMax0:
            weight = arity+W_TO_BASEWEIGHT;
            break;
      case WAritySqWeight:
      case WAritySqMax0:
            weight = arity*arity+1;
            break;
      case WInvArityWeight:
      case WInvArityMax0:
            weight = maxarity-arity+1;
            break;
      case WInvAritySqWeight:
      case WInvAritySqMax0:
            weight = (maxarity*maxarity)-(arity*arity)+1;
            break;
      default:
            assert(false);
            break;
      }
      *OCBFunWeightPos(ocb, i) = weight*W_DEFAULT_WEIGHT;
   }
   if((method == WArityMax0)||(method == WModArityMax0)||
      (method == WInvArityMax0)||(method == WAritySqMax0)||
      (method == WInvAritySqMax0))
   {
      set_maximal_0(ocb);
   }
}


/*-----------------------------------------------------------------------
//
// Function: generate_precedence_weights()
//
//    Weight(f) = |{g|g<f}|, i.e. weight is number of smaller function
//    symbols in the signature (+1).
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_precedence_weights(OCB_p ocb)
{
   FunCode i, j;
   int     weight;

   assert(ocb->precedence||ocb->prec_weights);

   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      weight = 1;
      for(j=SIG_TRUE_CODE+1; j<=ocb->sig_size; j++)
      {
         if(OCBFunCompare(ocb, i,j)== to_greater)
         {
            weight++;
         }
      }
      *OCBFunWeightPos(ocb, i) = weight*W_DEFAULT_WEIGHT;
   }
}


/*-----------------------------------------------------------------------
//
// Function: generate_invprecedence_weights()
//
//    Weight(f) = |{g|g>f}|, i.e. weight is number of bigger function
//    symbols in the signature (+1).
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_invprecedence_weights(OCB_p ocb)
{
   FunCode i, j;
   int     weight;

   assert(ocb->precedence||ocb->prec_weights);

   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      weight = 1;
      for(j=SIG_TRUE_CODE+1; j<=ocb->sig_size; j++)
      {
         if(OCBFunCompare(ocb, i,j)== to_lesser)
         {
            weight++;
         }
      }
      *OCBFunWeightPos(ocb, i) = weight*W_DEFAULT_WEIGHT;
   }
}


/*-----------------------------------------------------------------------
//
// Function: generate_precrank5_weights()
//
//    Weight(f) = rank (of 5) in the precedence.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_precrank5_weights(OCB_p ocb)
{
   generate_precrank_weights(ocb, 5);
}


/*-----------------------------------------------------------------------
//
// Function: generate_precrank10_weights()
//
//    Weight(f) = rank (of 10) in the precedence.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_precrank10_weights(OCB_p ocb)
{
   generate_precrank_weights(ocb, 10);
}




/*-----------------------------------------------------------------------
//
// Function: generate_precrank20_weights()
//
//    Weight(f) = rank (of 20) in the precedence.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

static void generate_precrank20_weights(OCB_p ocb)
{
   generate_precrank_weights(ocb, 20);
}




/*-----------------------------------------------------------------------
//
// Function: generate_freq_weights()
//
//   Make the weight of a function symbol equal to its frequency count
//   in the axiom set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_freq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      *OCBFunWeightPos(ocb, i) = MAX(array->array[i].freq,1)
         *W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: generate_type_freq_weights()
//
//    Assign function symbols weights that are equal to
//    sum of occurrences of all function symbols that are of
//    the same type.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_type_freq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FunCode       i;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      *OCBFunWeightPos(ocb, i) = MAX(type_counts[sym_type_id],1)*W_DEFAULT_WEIGHT;
   }
   SizeFree(type_counts, max_types*sizeof(long));
}

/*-----------------------------------------------------------------------
//
// Function: generate_comb_freq_weights()
//
//    Assign function symbols weights that are equal to
//    sum of occurrences of all function symbols that are of
//    the same type + double the occurrence of the symbol itself.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_comb_freq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   long max_types = ocb->sig->type_bank->types_count+1;
   PDArray_p type_counts = PDIntArrayAlloc(max_types,10);

   for(i=SIG_TRUE_CODE+1; i <= ocb->sig->f_count; i++)
   {
      long sym_freq = array->array[i].freq;
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      PDArrayAssignInt(type_counts, sym_type_id,
                       PDArrayElementInt(type_counts, sym_type_id) + sym_freq);
   }

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_freq = array->array[i].freq;
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      *OCBFunWeightPos(ocb, i) =
         MAX(PDArrayElementInt(type_counts, sym_type_id)+2*sym_freq,1)
         *W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
   PDArrayFree(type_counts);
}

/*-----------------------------------------------------------------------
//
// Function: generate_inv_comb_freq_weights()
//
//    Inverse version of generate_comb_freq_weights()
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_inv_comb_freq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }
   long max_comb = 0;

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(i=SIG_TRUE_CODE+1; i <= ocb->sig->f_count; i++)
   {
      long sym_freq = array->array[i].freq;
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      max_comb = MAX(max_comb, type_counts[sym_type_id] + 2*sym_freq);
   }
   max_comb++;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_freq = array->array[i].freq;
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      *OCBFunWeightPos(ocb, i) =
         (max_comb - MAX(type_counts[sym_type_id] + 2*sym_freq,1))
         *W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
   SizeFree(type_counts, max_types*sizeof(long));
}


/*-----------------------------------------------------------------------
//
// Function: generate_inv_typefreq_weights()
//
//    Assign function symbols weights that are equal to
//    difference of maximal sum of occurences of symbols of one type
//    and sum of occurrences of all function symbols that are of
//    the same type.
//
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_inv_type_freq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FunCode       i;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }

   long max_aggregate = 0;

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(long i=0; i<max_types; i++)
   {
      max_aggregate = MAX(max_aggregate, type_counts[i]);
   }
   max_aggregate++;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      *OCBFunWeightPos(ocb, i) =
         (max_aggregate - MAX(type_counts[sym_type_id],1))
         *W_DEFAULT_WEIGHT;
   }
   SizeFree(type_counts, max_types*sizeof(long));
}

#endif

/*-----------------------------------------------------------------------
//
// Function: generate_invfreq_weights()
//
//   Make the weight of a function symbol equal to the maximum
//   frequency count minus its frequency count in the axiom set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long max_count = 1;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      max_count = MAX(array->array[i].freq,max_count);
   }
   max_count++;
   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      *OCBFunWeightPos(ocb, i) =
         (max_count-MAX(array->array[i].freq,1))*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_freqrank_weights()
//
//   Make the weight of a function symbol equal to its "frequency
//   rank".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_freqrank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      if(freq!=array->array[i].freq)
      {
         freq=array->array[i].freq;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         // avoiding assigning 0 weight
         MAX(weight,1)*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: generate_type_freq_rank_weights()
//
//   Make the weight of a function symbol equal "frequency rank"
//   of its type.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_type_freq_rank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      array->array[i].key1 = type_counts[sym_type_id];
   }
   FCodeFeatureArraySort(array);
   freq = -1;
   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      if(freq!=array->array[i].key1)
      {
         freq=array->array[i].key1;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
   SizeFree(type_counts, max_types*sizeof(long));
}

/*-----------------------------------------------------------------------
//
// Function: generate_comb_freq_rank_weights()
//
//   Make the weight of a function symbol equal to
//   rank of "frequency of type + 2*frequency of symbol"
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_comb_freq_rank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_freq = array->array[i].freq;
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      array->array[i].key1 = type_counts[sym_type_id] + 2*sym_freq;
   }
   FCodeFeatureArraySort(array);
   freq = -1;
   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      if(freq!=array->array[i].key1)
      {
         freq=array->array[i].key1;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
   SizeFree(type_counts, sizeof(max_types*sizeof(long)));
}

#endif

/*-----------------------------------------------------------------------
//
// Function: generate_invfreqrank_weights()
//
//   Make the weight of a function symbol equal to its inverse
//   "frequency rank".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreqrank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   for(i=ocb->sig->f_count; i>=SIG_TRUE_CODE+1; i--)
   {
      if(freq!=array->array[i].freq)
      {
         freq=array->array[i].freq;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         // making sure 0 is not given to any symbol
         MAX(weight,1)*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: generate_inv_type_freq_rank_weights()
//
//   Make the weight of a function symbol equal to inverse of its type
//   "frequency rank".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_inv_type_freq_rank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      array->array[i].key1 = type_counts[sym_type_id];
   }
   FCodeFeatureArraySort(array);
   freq = -1;
   for(i=ocb->sig->f_count; i>=SIG_TRUE_CODE+1; i--)
   {
      if(freq!=array->array[i].key1)
      {
         freq=array->array[i].key1;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
   SizeFree(type_counts, max_types*sizeof(long));
}

/*-----------------------------------------------------------------------
//
// Function: generate_inv_comb_freq_rank_weights()
//
//   Make the weight of a function symbol equal to inverse of its type
//   "frequency rank".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_inv_comb_freq_rank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   long max_types = ocb->sig->type_bank->types_count+1;
   long* type_counts = SizeMalloc(max_types*sizeof(long));
   for(long i=0; i<max_types; i++)
   {
      type_counts[i] = 0;
   }

   ClauseSetAddTypeDistribution(axioms, type_counts);

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      long sym_freq = array->array[i].freq;
      long sym_type_id = SigGetType(ocb->sig, i) ? SigGetType(ocb->sig, i)->type_uid : 0;
      array->array[i].key1 = type_counts[sym_type_id] + 2*sym_freq;
   }
   FCodeFeatureArraySort(array);
   freq = -1;
   for(i=ocb->sig->f_count; i>=SIG_TRUE_CODE+1; i--)
   {
      if(freq!=array->array[i].key1)
      {
         freq=array->array[i].key1;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
   SizeFree(type_counts, max_types*sizeof(long)) ;
}

#endif

/*-----------------------------------------------------------------------
//
// Function: generate_invconjfreqrank_weights()
//
//   Make the weight of a function symbol equal to its inverse
//   "conjecture frequency rank".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invconjfreqrank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq, conjfreq;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].conjfreq?
         (INT_MAX-array->array[i].conjfreq):0;
      array->array[i].key2 = -array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   conjfreq = 0;
   for(i=SIG_TRUE_CODE+1; i<=ocb->sig->f_count ;i++)
   {
      if((freq!=array->array[i].freq) ||
         (conjfreq!=array->array[i].conjfreq))
      {
         freq=array->array[i].freq;
         conjfreq=array->array[i].conjfreq;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}



/*-----------------------------------------------------------------------
//
// Function: generate_freqranksq_weights()
//
//   Make the weight of a function symbol equal to its "frequency
//   rank" squared.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_freqranksq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      if(freq!=array->array[i].freq)
      {
         freq=array->array[i].freq;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_invfreqranksq_weights()
//
//   Make the weight of a function symbol equal to the square of its
//   inverse "frequency rank".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_invfreqranksq_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, freq;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   for(i=ocb->sig->f_count; i>=SIG_TRUE_CODE+1; i--)
   {
      if(freq!=array->array[i].freq)
      {
         freq=array->array[i].freq;
         weight++;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_inv_modfreqrank_weights()
//
//   Make the weight of a function symbol equal to its modified
//   frequency rank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_inv_modfreqrank_weights(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, base=0, freq;

   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   for(i=ocb->sig->f_count; i>=SIG_TRUE_CODE+1; i--)
   {
      base++;
      if(freq!=array->array[i].freq)
      {
         freq=array->array[i].freq;
         weight=base;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}


/*-----------------------------------------------------------------------
//
// Function: generate_inv_modfreqrank_weights_max0()
//
//   Make the weight of a function symbol equal to its modified
//   frequency rank, but make the first unary maximal symbol 0.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void generate_inv_modfreqrank_weights_max_0(OCB_p ocb, ClauseSet_p axioms)
{
   FCodeFeatureArray_p array = FCodeFeatureArrayAlloc(ocb->sig, axioms);
   FunCode       i;
   long          weight = 0, base=0, freq;


   for(i=SIG_TRUE_CODE+1; i<= ocb->sig->f_count; i++)
   {
      array->array[i].key1 = array->array[i].freq;
   }
   FCodeFeatureArraySort(array);
   freq = 0;
   for(i=ocb->sig->f_count; i>=SIG_TRUE_CODE+1; i--)
   {
      base++;
      if(freq!=array->array[i].freq)
      {
         freq=array->array[i].freq;
         weight=base;
      }
      *OCBFunWeightPos(ocb, array->array[i].symbol) =
         weight*W_DEFAULT_WEIGHT;
   }
   set_maximal_unary_0(ocb);
   FCodeFeatureArrayFree(array);
}




/*-----------------------------------------------------------------------
//
// Function: set_user_weights()
//
//   Given a user weight string, set the symbols to the desired
//   weight.
//
// Global Variables: -
//
// Side Effects    : May fail with syntax error
//
/----------------------------------------------------------------------*/

void set_user_weights(OCB_p ocb, char* pre_weights)
{
   Scanner_p in = CreateScanner(StreamTypeUserString, pre_weights,
                                true, NULL, true);

   TOWeightsParse(in, ocb);

   DestroyScanner(in);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TOTranslateWeightGenMethod()
//
//   Given a string, return the corresponding TOWeightGenMethod
//   token.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


TOWeightGenMethod TOTranslateWeightGenMethod(char* name)
{
   int method;

   method = StringIndex(name, TOWeightGenNames);

   if(method == -1)
   {
      method = WNoMethod;
   }
   return (TOWeightGenMethod)method;
}


/*-----------------------------------------------------------------------
//
// Function: TOGenerateWeights()
//
//   Given a pre-initialized OCB, assign weights to the function
//   symbols. Some methods require a precedence, some require the
//   axioms.
//
// Global Variables: -
//
// Side Effects    : Sets weights in ocb.
//
/----------------------------------------------------------------------*/

void TOGenerateWeights(OCB_p ocb, ClauseSet_p axioms, char *pre_weights,
                       OrderParms_p oparms)
{
   FunCode i;

   assert(ocb);
   assert(ocb->weights);
   assert(ocb->sig);

   *OCBFunWeightPos(ocb, SIG_TRUE_CODE) = 1;
   VERBOUTARG("Generating ordering weight with ",
              TOWeightGenNames[oparms->to_weight_gen]);

   switch(oparms->to_weight_gen)
   {
   case WConstantWeight:
         generate_constant_weights(ocb);
         break;
   case WSelectMaximal:
         generate_selmax_weights(ocb);
         break;
   case WModArityWeight:
   case WModArityMax0:
         generate_arity_weights(ocb, oparms->to_weight_gen);
         if(oparms->to_const_weight == WConstNoSpecialWeight)
         {
            ocb->var_weight = W_TO_BASEWEIGHT;
         }
         else
         {
            ocb->var_weight = oparms->to_const_weight;
         }
         break;
   case WArityWeight:
   case WArityMax0:
   case WAritySqWeight:
   case WAritySqMax0:
   case WInvArityWeight:
   case WInvArityMax0:
   case WInvAritySqWeight:
   case WInvAritySqMax0:
         generate_arity_weights(ocb, oparms->to_weight_gen);
         break;
   case WPrecedence:
         generate_precedence_weights(ocb);
         break;
   case WPrecedenceInv:
         generate_invprecedence_weights(ocb);
         break;
   case WPrecRank5:
         generate_precrank5_weights(ocb);
         break;
   case WPrecRank10:
         generate_precrank10_weights(ocb);
         break;
   case WPrecRank20:
         generate_precrank20_weights(ocb);
         break;
   case WFrequency:
         generate_freq_weights(ocb, axioms);
         break;
   case WInvFrequency:
         generate_invfreq_weights(ocb, axioms);
         break;
   case WFrequencyRank:
         generate_freqrank_weights(ocb, axioms);
         break;
   case WInvFrequencyRank:
         generate_invfreqrank_weights(ocb, axioms);
         break;
   case WInvConjFrequencyRank:
         generate_invconjfreqrank_weights(ocb, axioms);
         break;
   case WFrequencyRankSq:
         generate_freqranksq_weights(ocb, axioms);
         break;
   case WInvFrequencyRankSq:
         generate_invfreqranksq_weights(ocb, axioms);
         break;
   case WInvModFreqRank:
         generate_inv_modfreqrank_weights(ocb, axioms);
         break;
   case WInvModFreqRankMax0:
         generate_inv_modfreqrank_weights_max_0(ocb, axioms);
         break;
   case WNoMethod:
         generate_selmax_weights(ocb);
         break;
#ifdef ENABLE_LFHO
   case WTypeFrequencyRank:
         assert(problemType == PROBLEM_HO);
         generate_type_freq_rank_weights(ocb, axioms);
         break;
   case WTypeFrequencyCount:
         assert(problemType == PROBLEM_HO);
         generate_type_freq_weights(ocb, axioms);
         break;
   case WInvTypeFrequencyRank:
         assert(problemType == PROBLEM_HO);
         generate_inv_type_freq_rank_weights(ocb, axioms);
         break;
   case WInvTypeFrequencyCount:
         assert(problemType == PROBLEM_HO);
         generate_inv_type_freq_weights(ocb, axioms);
         break;
   case WCombFrequencyRank:
         assert(problemType == PROBLEM_HO);
         generate_comb_freq_rank_weights(ocb, axioms);
         break;
   case WCombFrequencyCount:
         assert(problemType == PROBLEM_HO);
         generate_comb_freq_weights(ocb, axioms);
         break;
   case WInvCombFrequencyRank:
         assert(problemType == PROBLEM_HO);
         generate_inv_comb_freq_rank_weights(ocb, axioms);
         break;
   case WInvCombFrequencyCount:
         assert(problemType == PROBLEM_HO);
         generate_inv_comb_freq_weights(ocb, axioms);
         break;
#endif
   default:
         assert(false && "Weight generation method unimplemented");
         break;
   }
   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      if(SigFindArity(ocb->sig, i)==0)
      {
         if(oparms->to_const_weight != WConstNoSpecialWeight)
         {
            *OCBFunWeightPos(ocb, i) = MAX(oparms->to_const_weight,1); // at least 1
         }
         assert(OCBFunWeight(ocb,i)>0);
         if(oparms->force_kbo_var_weight)
         {
            ocb->var_weight = MIN(ocb->var_weight, OCBFunWeight(ocb, i));
         }
      }
   }
   *OCBFunWeightPos(ocb, SIG_TRUE_CODE) = ocb->var_weight;
#ifdef ENABLE_LFHO
   *OCBFunWeightPos(ocb, SIG_PHONY_APP_CODE) = 0;
#endif
   if(pre_weights)
   {
      fprintf(stderr, "setting user weights\n");
      set_user_weights(ocb, pre_weights);
   }

   ocb->lam_weight = oparms->lam_w;
   ocb->db_weight = oparms->db_w;

#ifdef PRINT_FUNWEIGHTS
   print_weight_array(GlobalOut,ocb);
#endif
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
