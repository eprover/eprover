/*-----------------------------------------------------------------------

  File  : che_specsigfeatures.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Functions for determining various features of specifications,
  i.e. clause and (later) formula sets. This is analoguous to
  che_clausesetfeatures.[ch], but uses different features.


  Copyright 2017 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Wed Aug 30 11:40:05 CEST 2017

  -----------------------------------------------------------------------*/

#include "che_specsigfeatures.h"



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
// Function: term_collect_sig_features_rek()
//
//    Collect information of number and depth of occurrence of function
//    symbols of different arity in term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void term_collect_sig_features_rek(Sig_p sig, Term_p t, long* features,
                                   long depth)
{
   if(t->f_code > 0)
   {
      int i;
      features[SigGetFeatureOffset(sig, t->f_code)]++;
      if(!SigIsPredicate(sig, t->f_code) &&
         (depth > features[SigGetDepthFeatureOffset(sig, t->f_code)]))
      {
         features[SigGetDepthFeatureOffset(sig, t->f_code)] = depth;
      }
      for(i=0; i<t->arity; i++)
      {
         term_collect_sig_features_rek(sig, t->args[i], features, depth+1);
      }
   }
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: SpecSigFeatureInit()
//
//   Initialize (set to 0) all features. Could use memset(), but this
//   is more transparent and non critical...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SpecSigFeatureInit(SpecSigFeature_p specftrs)
{
   int i;

   for(i=0; i<SPECSIG_TOTAL_FTR_NO; i++)
   {
      specftrs->features[i] = 0;
   }
}

/*-----------------------------------------------------------------------
//
// Function: SpecSigFeaturePrint
//
//   Print all individual features as a coma-separated list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SpecSigFeaturePrint(FILE*out, SpecSigFeature_p specftrs)
{
   int i;
   char* sep="";

   for(i=0; i<SPECSIG_TOTAL_FTR_NO; i++)
   {
      fprintf(out, "%s%5ld", sep, specftrs->features[i]);
      sep =", ";
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermCollectSigFeatures()
//
//    Collect information of number and depth of occurrence of function
//    symbols of different arity in term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermCollectSigFeatures(Sig_p sig, Term_p term, long* features)
{
   term_collect_sig_features_rek(sig, term, features, 1);
}





/*-----------------------------------------------------------------------
//
// Function: ClauseCollectSigFeatures()
//
//   Collect positive and negative signature features (distribution of
//   arities) for the clause.
//
//   Structure of features (L = SIG_FEATURE_ARITY_LIMIT)
//   features[0]: Number of positive equational literals
//   features[1]: Number of negative equational literals
//   For positive literals:
//   features[...2+L]: Frequency of of pred-symbols of arity n
//   features[...2+2L]: Frequency of fun-symbols of arity n
//   features[...2+3L]: Max depth of fun-symbols of arity n
//   For negative literals:
//   features[...2+4L]: Frequency of of pred-symbols of arity n
//   features[...2+5L]: Frequency of fun-symbols of arity n
//   features[...2+6L]: Max depth of fun-symbols of arity n
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseCollectSigFeatures(Clause_p clause, long* features)
{
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         EqnCollectSigFeatures(handle, features+2);
         if(EqnIsEquLit(handle))
         {
            features[0]++;
         }
      }
      else
      {
         EqnCollectSigFeatures(handle, features+2+3*SIG_FEATURE_ARITY_LIMIT);
         if(EqnIsEquLit(handle))
         {
            features[1]++;
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseComputeSigFeatures()
//
//   Compute the signature-based features of the clause. As above, but
//   zeros out the result vector first.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ClauseComputeSigFeatures(Clause_p clause, long* features)
{
   memset(features, 0, (2+6*SIG_FEATURE_ARITY_LIMIT)*sizeof(long));
   ClauseCollectSigFeatures(clause, features);
}


/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ClauseSetCollectSigFeatures(Sig_p sig, ClauseSet_p set,
                                 SpecSigFeature_p specftrs)
{
   Clause_p handle;
   FunCode i;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         ClauseCollectSigFeatures(handle, specftrs->features+SPECSIG_CJ_FTRS);
      }
      else
      {
         ClauseCollectSigFeatures(handle, specftrs->features+SPECSIG_AX_FTRS);
      }
      if(ClauseIsUnit(handle))
      {
         specftrs->features[SPECSIG_GLOBAL_UNIT]++;
      }
      else if(ClauseIsHorn(handle))
      {
         specftrs->features[SPECSIG_GLOBAL_HORN]++;
      }
      else
      {
         specftrs->features[SPECSIG_GLOBAL_GNRL]++;
      }
   }
   for(i=sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      specftrs->features[SPECSIG_GLOBAL_SIG+SigGetFeatureOffset(sig, i)]++;
      //printf(COMCHAR" %s : %d -> %d\n", sig->f_info[i].name, sig->f_info[i].arity,
      //SigGetFeatureOffset(sig, i));
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
