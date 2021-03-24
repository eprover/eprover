/*-----------------------------------------------------------------------

  File  : cte_lambda.h

  Author: Petar Vukmirovic

  Contents

  Functions that implement main operations of lambda calculus

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Mar 24 15:54:00 CET 2021

-----------------------------------------------------------------------*/

#ifndef CTE_LAMBDA

#define CTE_LAMBDA

#include "cte_termtypes.h"


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void NamedLambdaSNF(Term_p t);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
