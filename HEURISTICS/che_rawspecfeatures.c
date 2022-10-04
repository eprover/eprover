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
   Sig_p sig = state->terms->sig;
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

   features->sig_size    = SigCountSymbols(sig, true) + SigCountSymbols(sig,false);

   features->predc_size = SigCountAritySymbols(sig, 0, true);
   features->func_size  = SigCountAritySymbols(sig, 0, false);
   features->pred_size = SigCountSymbols(sig, true)-
      SigCountAritySymbols(sig, 0, true);
   features->fun_size  = SigCountSymbols(sig, false)-
      SigCountAritySymbols(sig, 0, false);
   features->has_choice_sym = SigHasChoiceSym(sig);

   features->order = 1;
   features->conj_order = 1;
   for(WFormula_p f = state->f_axioms->anchor->succ;
       f != state->f_axioms->anchor;
       f = f->succ)
   {
      int ord = TermComputeOrder(f->terms->sig, f->tformula);
      features->order = MAX(features->order, ord);
      if(FormulaIsConjecture(f) || FormulaIsHypothesis(f))
      {
         features->conj_order = MAX(features->conj_order, ord);
      }
   }

   FormulaSetDefinitionStatistics(state->f_axioms, state->f_ax_archive,
                                  state->terms,
                                  &(features->num_of_definitions),
                                  &(features->perc_of_form_defs),
                                  &(features->num_lambdas),
                                  &(features->app_var_lits));
   features->class[0] = '\0';
}

#define ADJUST_FOR_HO(limit, scale) (limit)
#define RAW_CLASSIFY(index, value, some, many, ho_scale_some, ho_scale_many)\
   if((value) < (ADJUST_FOR_HO(some, ho_scale_some)))\
   {                                            \
      features->class[index] = 'S';             \
   }                                            \
   else if((value) < (ADJUST_FOR_HO(many, ho_scale_many)))                    \
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
   features->class[0] = problemType == PROBLEM_HO ? 'H' : 'F';
   RAW_CLASSIFY(1, features->sentence_no,
                limits->ax_some_limit, limits->ax_many_limit, 5, 7);
   RAW_CLASSIFY(2, features->term_size,
                limits->term_medium_limit, limits->term_large_limit, 2, 4);
   RAW_CLASSIFY(3, features->sig_size,
                limits->symbols_medium_limit, limits->symbols_large_limit, 5, 7);

   RAW_CLASSIFY(4, features->pred_size,
                limits->pred_medium_limit, limits->pred_large_limit, 10, 10);
   RAW_CLASSIFY(5, features->predc_size,
                limits->predc_medium_limit, limits->predc_large_limit, 1, 1);
   RAW_CLASSIFY(6, features->fun_size,
                limits->fun_medium_limit, limits->fun_large_limit, 9, 5);
   RAW_CLASSIFY(7, features->func_size,
                limits->func_medium_limit, limits->func_large_limit, 2 ,10);
   RAW_CLASSIFY(8, features->num_of_definitions,
                limits->num_of_defs_medium_limit, limits->num_of_defs_large_limit, 1, 1);
   RAW_CLASSIFY(9, features->perc_of_form_defs,
                limits->perc_form_defs_medium_limit, limits->perc_form_defs_large_limit, 1, 1);
   RAW_CLASSIFY(10, features->num_lambdas,
                limits->num_of_lams_medium_limit, limits->num_of_lams_large_limit, 1, 1);
   features->class[11] = features->has_choice_sym ? 'C' : 'N';
   features->class[12] = features->order == 1 ? 'F' : (features->order == 2 ? 'S' : 'H');
   features->class[13] = features->conj_order == 0 ? 'N' :
                           (features->conj_order == 1 ? 'F' : (features->conj_order == 2 ? 'S' : 'H'));
   features->class[14] = features->app_var_lits ? 'A' : 'N';
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
   features->class[15] = '\0';
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
   AcceptInpTok(in, Comma);
   features->num_of_definitions   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->perc_of_form_defs   = ParseFloat(in);
   AcceptInpTok(in, Comma);
   features->num_lambdas   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->has_choice_sym   = ParseBool(in);
   AcceptInpTok(in, Comma);
   features->order   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->conj_order = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->app_var_lits   = ParseBool(in);

   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Colon);
   class = ParsePlainFilename(in);
   if(strlen(class) != 14)
   {
      Error("Raw class name must have 10 characters", SYNTAX_ERROR);
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
      fprintf(out, "(%7ld, %7lld, %6d, %6d, %6d, %6d, %6d, %6d, %.3f, %d, %d, %d, %d ) : %s",
              features->sentence_no,
              features->term_size,
              features->sig_size,
              features->pred_size,
              features->predc_size,
              features->fun_size,
              features->func_size,
              features->num_of_definitions,
              features->perc_of_form_defs,
              features->num_lambdas,
              features->order,
              features->conj_order,
              features->app_var_lits,
              features->class);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
