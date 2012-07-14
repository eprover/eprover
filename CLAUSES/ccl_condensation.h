/*-----------------------------------------------------------------------

File  : ccl_condensation.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Implementation of the condensation rule:

  C
  == if C' is a factor of C, C' subsumes C
  C'

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 11 01:44:53 CEST 2012
    New

-----------------------------------------------------------------------*/

#ifndef CCL_CONDENSATION

#define CCL_CONDENSATION


#include "ccl_subsumption.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


Clause_p CondenseOnce(Clause_p clause);
Clause_p Condense(Clause_p clause);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





