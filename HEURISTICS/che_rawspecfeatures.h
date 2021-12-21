/*-----------------------------------------------------------------------

File  : che_rawspecfeatures.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code and datatypes for handling rough classification of raw problem
  specs.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue May 22 01:10:30 CEST 2012
    New

-----------------------------------------------------------------------*/

#ifndef RAWSPECFEATURES

#define RAWSPECFEATURES

#include <che_clausesetfeatures.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define NUM_RAW_FEATURES
#define RAW_CLASS_SIZE 16


typedef struct raw_spec_feature_cell
{
   long      sentence_no;
   long long term_size;
   int       sig_size;
   int       pred_size;
   int       predc_size;
   int       fun_size;
   int       func_size;
   long      conjecture_count;
   long      hypothesis_count;
   int       num_lambdas;
   bool      has_choice_sym; // has a symobl in the signature of the type
                             // that fits a monomorphized choice
   int       num_of_definitions; // number of formulas tagged with definition
   double    perc_of_form_defs; // percentage of which defines formulas
   int       order; // order of signature
   int       conj_order; // order of conjecture symbols
   bool      app_var_lits; // do the formulas have applied variable literals
   char      class[RAW_CLASS_SIZE];
}RawSpecFeatureCell, *RawSpecFeature_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


void RawSpecFeaturesCompute(RawSpecFeature_p features, ProofState_p state);
void RawSpecFeaturesClassify(RawSpecFeature_p features, SpecLimits_p limits,
                             char* pattern);
void RawSpecFeaturesParse(Scanner_p in, RawSpecFeature_p features);
void RawSpecFeaturesPrint(FILE* out, RawSpecFeature_p features);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





