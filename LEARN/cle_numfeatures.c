/*-----------------------------------------------------------------------

File  : cle_numfeatures.c

Author: Stephan Schulz

Contents

  Functions for dealing with numerical features of clause sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jul 26 18:55:24 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cle_numfeatures.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: relative_difference()
//
//   Return the relative difference of two values.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double relative_difference(double v1, double v2)
{
   double abs1, abs2, res;

   if((v1==0.0) && (v2==0.0))
   {
      return 0;
   }
   abs1 = ABS(v1);
   abs2 = ABS(v2);
   res = (v1-v2)/(2*MAX(abs1,abs2));

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: arity_distr_distance()
//
//   Compute the normed euclidean distance beween two arity distribution
//   vectors.
//
// Global Variables: -
//
// Side Effects    : May theoretically extend the smaller distribution
//                   very unlikely, and completely harmless)
//
/----------------------------------------------------------------------*/

static double arity_distr_distance(PDArray_p d1, PDArray_p d2, int
               maxarity)
{
   int i, val1, val2;
   double res = 0;

   assert(maxarity >= -1);
   if(maxarity == -1)
   {
      return 0.0;
   }
   for(i=0; i<=maxarity; i++)
   {
      val1 = PDArrayElementInt(d1, i);
      val2 = PDArrayElementInt(d2, i);
      res += relative_difference(val1,val2)*
    relative_difference(val1,val2);
   }
   return sqrt(res)/(double)(maxarity+1);
}


/*-----------------------------------------------------------------------
//
// Function: parse_sig_distrib()
//
//   Parse a list (n0, n1, ... nn) into a PDArray.
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

static int parse_sig_distrib(Scanner_p in, PDArray_p distrib)
{
   int i= -1, symbols;

   AcceptInpTok(in, OpenBracket);

   if(!TestInpTok(in, CloseBracket))
   {
      i++;
      symbols = ParseInt(in);
      PDArrayAssignInt(distrib, i, symbols);
      while(!TestInpTok(in, CloseBracket))
      {
    i++;
    AcceptInpTok(in, Comma);
    symbols = ParseInt(in);
    PDArrayAssignInt(distrib, i, symbols);
      }
   }
   AcceptInpTok(in, CloseBracket);

   return i;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FeaturesAlloc()
//
//   Allocate an empty, initialized FeaturesCell()
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Features_p FeaturesAlloc(void)
{
   Features_p handle = FeaturesCellAlloc();

   handle->pred_distrib = PDIntArrayAlloc(5,5);
   handle->func_distrib = PDIntArrayAlloc(5,5);
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FeaturesFree()
//
//   Free a FeaturesCell()
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FeaturesFree(Features_p junk)
{
   PDArrayFree(junk->pred_distrib);
   PDArrayFree(junk->func_distrib);
   FeaturesCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ComputeClauseSetNumFeatures()
//
//   Compute the numerical features of a clause set. This is not as
//   modular as I would have liked, as I expect this to be done fairly
//   often and hence want to do it in a single pass.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ComputeClauseSetNumFeatures(Features_p features, ClauseSet_p set,
             Sig_p sig)
{
   PStack_p pos_tdepth = PStackAlloc();
   PStack_p neg_tdepth = PStackAlloc();
   PStack_p pos_tsize  = PStackAlloc();
   PStack_p neg_tsize  = PStackAlloc();
   PStack_p pos_lits   = PStackAlloc();
   PStack_p neg_lits   = PStackAlloc();
   double   average, deviation;
   Clause_p clause;
   Eqn_p    eqn;
   long     *symbol_distrib;
   long     i;

   symbol_distrib = SizeMalloc((sig->size)*sizeof(long));
   for(i=0; i< sig->size; i++)
   {
      symbol_distrib[i] = 0;
   }

   ClauseSetAddSymbolDistribution(set, symbol_distrib);
   features->pred_max_arity =
      SigAddSymbolArities(sig, features->pred_distrib, true,
           symbol_distrib);
   features->func_max_arity =
      SigAddSymbolArities(sig, features->func_distrib, false,
           symbol_distrib);
   SizeFree(symbol_distrib,(sig->size)*sizeof(long));

   features->features[0] = 0;
   features->features[1] = 0;
   features->features[2] = 0;

   for(clause = set->anchor->succ; clause != set->anchor; clause =
     clause->succ)
   {
      if(ClauseIsUnit(clause))
      {
    features->features[0]++;
      }
      else if(ClauseIsHorn(clause))
      {
    features->features[1]++;
      }
      else
      {
    features->features[2]++;
      }
      PStackPushInt(pos_lits, clause->pos_lit_no);
      PStackPushInt(neg_lits, clause->neg_lit_no);

      for(eqn = clause->literals; eqn; eqn = eqn->next)
      {
    if(EqnIsPositive(eqn))
    {
       PStackPushInt(pos_tsize, TermWeight(eqn->lterm,
                  DEFAULT_VWEIGHT,
                  DEFAULT_FWEIGHT));
       PStackPushInt(pos_tsize, TermWeight(eqn->rterm,
                  DEFAULT_VWEIGHT,
                  DEFAULT_FWEIGHT));
       PStackPushInt(pos_tdepth, TermDepth(eqn->lterm));
       PStackPushInt(pos_tdepth, TermDepth(eqn->rterm));
    }
    else
    {
       PStackPushInt(neg_tsize, TermWeight(eqn->lterm,
                  DEFAULT_VWEIGHT,
                  DEFAULT_FWEIGHT));
       PStackPushInt(neg_tsize, TermWeight(eqn->rterm,
                  DEFAULT_VWEIGHT,
                  DEFAULT_FWEIGHT));
       PStackPushInt(neg_tdepth, TermDepth(eqn->lterm));
       PStackPushInt(neg_tdepth, TermDepth(eqn->rterm));
    }
      }
   }
   average = PStackComputeAverage(pos_tdepth, &deviation);
   features->features[3] = average;
   features->features[4] = deviation;
   average = PStackComputeAverage(neg_tdepth, &deviation);
   features->features[5] = average;
   features->features[6] = deviation;
   average = PStackComputeAverage(pos_tsize, &deviation);
   features->features[7] = average;
   features->features[8] = deviation;
   average = PStackComputeAverage(neg_tsize, &deviation);
   features->features[9] = average;
   features->features[10] = deviation;
   average = PStackComputeAverage(pos_lits, &deviation);
   features->features[11] = average;
   features->features[12] = deviation;
   average = PStackComputeAverage(neg_lits, &deviation);
   features->features[13] = average;
   features->features[14] = deviation;

   PStackFree(neg_lits);
   PStackFree(pos_lits);
   PStackFree(neg_tsize);
   PStackFree(pos_tsize);
   PStackFree(neg_tdepth);
   PStackFree(pos_tdepth);
}


/*-----------------------------------------------------------------------
//
// Function: NumFeaturesPrint()
//
//   Print the feature cell.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void NumFeaturesPrint(FILE* out, Features_p features)
{
   char *sep;
   long i;

   fputs("PA: (" , out);
   sep = "";
   for(i=0; i<=features->pred_max_arity; i++)
   {
      fprintf(out, "%s%ld", sep,
         PDArrayElementInt(features->pred_distrib,i));
      sep = ", ";
   }
   fputs(")  FA: (", out);
   sep = "";
   for(i=0; i<=features->func_max_arity; i++)
   {
      fprintf(out, "%s%ld", sep,
         PDArrayElementInt(features->func_distrib,i));
      sep = ", ";
   }
   fprintf(out, ")\n(%f", features->features[0]);
   for(i=1; i<FEATURE_NUMBER;i++)
   {
      fprintf(out, ", %f", features->features[i]);
   }
   fputs(")\n",out);
}


/*-----------------------------------------------------------------------
//
// Function: NumFeaturesParse()
//
//   Parse a set of features.
//
// Global Variables: -
//
// Side Effects    : Allocates memory, reads input
//
/----------------------------------------------------------------------*/

Features_p NumFeaturesParse(Scanner_p in)
{
   Features_p handle = FeaturesAlloc();
   long i;

   AcceptInpId(in, "PA");
   AcceptInpTok(in, Colon);
   handle->pred_max_arity = parse_sig_distrib(in,
                     handle->pred_distrib);

   AcceptInpId(in, "FA");
   AcceptInpTok(in, Colon);
   handle->func_max_arity = parse_sig_distrib(in,
                     handle->func_distrib);
   AcceptInpTok(in, OpenBracket);
   handle->features[0] = ParseFloat(in);
   for(i=1; i< FEATURE_NUMBER; i++)
   {
      AcceptInpTok(in, Comma);
      handle->features[i] = ParseFloat(in);
   }
   AcceptInpTok(in, CloseBracket);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: NumFeatureDistance()
//
//   Return the weighted relative distance between the two feature
//   vectors.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double NumFeatureDistance(Features_p f1, Features_p f2, double pred_w,
           double func_w, double* weights)
{
   double res, norm, dist, wsq;
   int i;

   dist = arity_distr_distance(f1->pred_distrib, f2->pred_distrib,
                MAX(f1->pred_max_arity,
               f2->pred_max_arity));
   wsq = pred_w*pred_w;
   res = dist*dist*wsq;
   norm = wsq;

   dist = arity_distr_distance(f1->func_distrib, f2->func_distrib,
                MAX(f1->func_max_arity,
               f2->func_max_arity));
   wsq = func_w*func_w;
   res += dist*dist*wsq;
   norm += wsq;

   for(i=0; i<FEATURE_NUMBER; i++)
   {
      dist = relative_difference(f1->features[i],f2->features[i]);
      wsq  = weights[i] * weights[i];
      res += dist*dist*wsq;
      norm += wsq;
   }
   res = res/norm;
   return sqrt(res);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/




























