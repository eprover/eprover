/*-----------------------------------------------------------------------

File  : che_to_weightgen.c

Author: Stephan Schulz

Contents
 
  Functions implementing several simple weight generation schemes for
  the KBO.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Sep 25 02:49:11 MET DST 1998
    New
<2> Mon Jan 11 19:54:13 MET 1999
    Eleminated all those weight generation schemes that did not result
    in an reduction ordering. Hit my head on the desk 15 times, too!

-----------------------------------------------------------------------*/

#include "che_to_weightgen.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* TOWeightGenNames[]=
{
   "none",               /* WNoMethod */
   "firstmaximal0",      /* WSelectMaximal */
   "arity",              /* WArityWeight */
   "aritymax0",          /* WArityMax0 */
   "modarity",           /* WModArityWeight */
   "modaritymax0",       /* WModArityMax0 */
   "aritysquared",       /* WAritySqWeight */
   "aritysquaredmax0",   /* WAritySqMax0 */
   "invarity",           /* WInvArityWeight */
   "invaritymax0",       /* WInvArityMax0 */
   "invaritysquared",    /* WInvSqArityWeight */
   "invaritysquaredmax0",/* WInvAritySqMax0 */
   "precedence",         /* WPrecedence */
   "invprecedence",      /* WPrecedenceInv */
   "freqcount",
   "invfreqcount",
   "freqrank",
   "invfreqrank",
   "freqranksquare",
   "invfreqranksquare",
   "invmodfreqrank",     /* WModFreqRank */
   "invmodfreqrankmax0", /* WModFreqRankMax0 */
   "constant",           /* WConstantWeight */
   NULL
};


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

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
/* Uncomment for debugging
static void print_weight_array(FILE* out,OCB_p ocb)
{
   FunCode i;
   
   fprintf(out, "# Ordering weights: ");
   for(i = 1; i<=ocb->sig->f_count; i++)
   {
      if(!SigIsSpecial(ocb->sig, i))
      {
	 fprintf(out, "%s:%ld ", SigFindName(ocb->sig,i),
		 OCBFunWeight(ocb,i));
      }
   }
   fprintf(out, "\n");
}*/

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
   FunCode i,j;
   bool    max;
   assert(ocb->precedence||ocb->prec_weights);
   
   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      max = true;
      
      for(j=1; j<=ocb->sig_size; j++)
      {
	 if(OCBFunCompare(ocb, i, j) == to_lesser)
	 {
	    max = false;
	    break;
	 }	 
      }   
      if(max && (SigFindArity(ocb->sig, i)>0))
      {
	 *OCBFunWeightPos(ocb, i) = 0;
	 break;
      }
   }
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
   FunCode i,j;
   bool    max;
   assert(ocb->precedence||ocb->prec_weights);
   
   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      if(SigFindArity(ocb->sig, i) == 1)
      {
	 max = true;
      
	 for(j=1; j<=ocb->sig_size; j++)
	 {
	    if(OCBFunCompare(ocb, i, j) == to_lesser)
	    {
	       max = false;
	       break;
	    }	 
	 }   
	 if(max)
	 {
	    *OCBFunWeightPos(ocb, i) = 0;
	    break;
	 }
      }
   }
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
   set_maximal_0(ocb);
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
	 weight*W_DEFAULT_WEIGHT;
   }
   FCodeFeatureArrayFree(array);
}

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
//   Make the weight of a function symbol equal to its inverse
//   "frequency rank". 
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

void TOGenerateWeights(OCB_p ocb, ClauseSet_p axioms,
		       TOWeightGenMethod method, long const_weight)
{
   FunCode i;

   assert(ocb);
   assert(ocb->weights);
   assert(ocb->sig);

   *OCBFunWeightPos(ocb, SIG_TRUE_CODE) = 1;

   switch(method)
   {
   case WConstantWeight:
	 generate_constant_weights(ocb);
	 break;
   case WSelectMaximal:
	 generate_selmax_weights(ocb);
	 break;
   case WModArityWeight:
   case WModArityMax0:
	 generate_arity_weights(ocb, method);
	 if(const_weight == WConstNoSpecialWeight)
	 {
	    ocb->var_weight = W_TO_BASEWEIGHT;
	 }
	 else
	 {
	    ocb->var_weight = const_weight;
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
	 generate_arity_weights(ocb, method);
	 break;
   case WPrecedence:
	 generate_precedence_weights(ocb);
	 break;
   case WPrecedenceInv:
	 generate_invprecedence_weights(ocb);
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
   default:
	 assert(false && "Weight generation method unimplemented");
	 break;
   }      
   for(i=SIG_TRUE_CODE+1; i<=ocb->sig_size; i++)
   {
      if(SigFindArity(ocb->sig, i)==0)
      {
	 if(const_weight != WConstNoSpecialWeight)
	 {	    
	    *OCBFunWeightPos(ocb, i) = const_weight;
	 }
	 assert(OCBFunWeight(ocb,i)>0);
      }
   }
   *OCBFunWeightPos(ocb, SIG_TRUE_CODE) = ocb->var_weight;
   /*  print_weight_array(GlobalOut,ocb); */
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


