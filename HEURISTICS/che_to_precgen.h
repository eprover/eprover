/*-----------------------------------------------------------------------

  File  : che_to_precgen.h

  Author: Stephan Schulz

  Contents

  Routines for generating precedences for term orderings

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Oct 16 17:01:23 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CHE_TO_PRECGEN

#define CHE_TO_PRECGEN

#include <che_to_params.h>
#include <che_fcode_featurearrays.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FREQ_SEMI_INFTY 2000000 /* Bigger than any expected frequency,
                                 * small enough to never cause over-
                                 * or underflow */


#define TOGenerateDefaultPrecedence(ocb,axioms)                 \
   TOGeneratePrecedence((ocb), (axioms),NULL, PUnaryFirst)

void TOGeneratePrecedence(OCB_p ocb, ClauseSet_p axioms,
                          char* predefined, OrderParms_p oparms);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
