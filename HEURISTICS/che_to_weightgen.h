/*-----------------------------------------------------------------------

  File  : che_to_weightgen.h

  Author: Stephan Schulz

  Contents

  Routines for generating weights for term orderings

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Sep 25 02:49:11 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CHE_TO_WEIGHTGEN

#define CHE_TO_WEIGHTGEN


#include <che_fcode_featurearrays.h>
#include <che_to_params.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/* Used as base weight for ModArity */
#define W_TO_BASEWEIGHT 4


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/



#define TOGenerateDefaultWeights(ocb)                  \
   TOGenerateWeights((ocb), NULL, WSelectMaximal,      \
                     W_DEFAULT_WEIGHT)

void TOGenerateWeights(OCB_p ocb, ClauseSet_p axioms, char *pre_weights,
                       OrderParms_p oparms);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
