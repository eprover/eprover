/*-----------------------------------------------------------------------

File  : che_rawspecfeatures.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code and datatypes for handling rough classification of raw problem
  specs.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue May 22 01:16:15 CEST 2012
    New

-----------------------------------------------------------------------*/

#include "che_rawspecfeatures.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: RawSpecFeaturesCompute()
//
//   Compute the raw features of state.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void RawSpecFeaturesCompute(RawSpecFeature_p features, ProofState_p state)
{
   features->sentence_no = ClauseSetCardinality(state->axioms)+
      ClauseSetCardinality(state->f_axioms);
   features->term_size   = ClauseSetStandardWeight(state->axioms)+
      FormulaSetStandardWeight(state->f_axioms);
   features->hypothesis_count = 0;
   features->conjecture_count = 0;
   features->conjecture_count +=
      ClauseSetCountConjectures(state->axioms,
                                &(features->hypothesis_count));
   features->conjecture_count +=
      FormulaSetCountConjectures(state->f_axioms,
                                 &(features->hypothesis_count));

   features->sig_size    = SigCountSymbols(state->terms->sig, true)+
      SigCountSymbols(state->terms->sig,false);

   features->predc_size = SigCountAritySymbols(state->terms->sig, 0, true);
   features->func_size  = SigCountAritySymbols(state->terms->sig, 0, false);
   features->pred_size = SigCountSymbols(state->terms->sig, true)-
      SigCountAritySymbols(state->terms->sig, 0, true);
   features->fun_size  = SigCountSymbols(state->terms->sig, false)-
      SigCountAritySymbols(state->terms->sig, 0, false);


   features->class[0] = '\0';
}

#define RAW_CLASSIFY(index, value, some, many)\
   if((value) < (some))\
   {                                            \
      features->class[index] = 'S';             \
   }                                            \
   else if((value) < (many))                    \
   {                                            \
      features->class[index] = 'M';             \
   }                                            \
   else                                         \
   {                                            \
      features->class[index] = 'L';             \
   }


/*-----------------------------------------------------------------------
//
// Function: RawSpecFeaturesClassify()
//
//   Add a classifiction based on limits to the (initialized)
//   features.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void RawSpecFeaturesClassify(RawSpecFeature_p features, SpecLimits_p limits,
                             char* pattern)
{
   RAW_CLASSIFY(0, features->sentence_no,
                limits->ax_some_limit, limits->ax_many_limit);
   RAW_CLASSIFY(1, features->term_size,
                limits->term_medium_limit, limits->term_large_limit);
   RAW_CLASSIFY(2, features->sig_size,
                limits->symbols_medium_limit, limits->symbols_large_limit);

   RAW_CLASSIFY(3, features->pred_size,
                limits->pred_medium_limit, limits->pred_large_limit);
   RAW_CLASSIFY(4, features->predc_size,
                limits->predc_medium_limit, limits->predc_large_limit);
   RAW_CLASSIFY(5, features->fun_size,
                limits->fun_medium_limit, limits->fun_large_limit);
   RAW_CLASSIFY(6, features->func_size,
                limits->func_medium_limit, limits->func_large_limit);

   if(pattern)
   {
      char* handle;

      for(handle = features->class;*pattern; pattern++, handle++)
      {
         if(*pattern=='-')
         {
            *handle = '-';
         }
      }
   }
   features->class[7] = '\0';
}



/*-----------------------------------------------------------------------
//
// Function: RawSpecFeaturesParse()
//
//   Parse a rawspecfeatures line.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void RawSpecFeaturesParse(Scanner_p in, RawSpecFeature_p features)
{
   char *class;

   AcceptInpTok(in, OpenBracket);
   features->sentence_no = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->term_size   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->sig_size   = ParseInt(in);

   AcceptInpTok(in, Comma);
   features->pred_size   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->predc_size   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->fun_size   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->func_size   = ParseInt(in);

   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Colon);
   class = ParsePlainFilename(in);
   if(strlen(class) != 7)
   {
      Error("Raw class name must have 7 characters", SYNTAX_ERROR);
   }
   strcpy(features->class, class);
   FREE(class);
}


/*-----------------------------------------------------------------------
//
// Function: RawSpecFeaturesPrint()
//
//   Print the features.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void RawSpecFeaturesPrint(FILE* out, RawSpecFeature_p features)
{
      fprintf(out, "(%7ld, %7lld, %6d, %6d, %6d, %6d, %6d) : %s",
              features->sentence_no,
              features->term_size,
              features->sig_size,
              features->pred_size,
              features->predc_size,
              features->fun_size,
              features->func_size,
              features->class);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


