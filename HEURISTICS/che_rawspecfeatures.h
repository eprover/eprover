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
   char      class[8];
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





