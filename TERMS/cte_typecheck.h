/*-----------------------------------------------------------------------

  File  : cte_typecheck.h

  Author: Simon Cruanes, Petar Vucmirovic, Stephan Schulz

  Contents

  Type checking and inference for Simple types

  Copyright 2011-2020 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Mon Jul  8 17:15:05 CEST 2013

  -----------------------------------------------------------------------*/

#ifndef CTE_TYPECHECK

#define CTE_TYPECHECK

#include <cte_signature.h>
#include <cte_termtypes.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


Type_p TypeCheckEq(Sig_p sig, Term_p t);
Type_p TypeCheckDistinct(Sig_p sig, Term_p t);
Type_p TypeCheckArithBinop(Sig_p sig, Term_p t);
Type_p TypeCheckArithConv(Sig_p sig, Term_p t);


bool     TypeCheckConsistent(Sig_p sig, Term_p term);
void     TypeInferSort(Sig_p sig, Term_p term, Scanner_p in);
void     TypeDeclareIsPredicate(Sig_p sig, Term_p term);
void     TypeDeclareIsNotPredicate(Sig_p sig, Term_p term, Scanner_p in);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
