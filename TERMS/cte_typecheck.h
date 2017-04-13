/*-----------------------------------------------------------------------

File  : cte_typecheck.h

Author: Simon Cruanes

Contents

  Type checking and inference for Simple types

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1>   Mon Jul  8 17:15:05 CEST 2013
      New

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

bool     TypeCheckConsistent(Sig_p sig, Term_p term);
void     TypeInferSort(Sig_p sig, Term_p term);
void     TypeDeclareIsPredicate(Sig_p sig, Term_p term);
void     TypeDeclareIsNotPredicate(Sig_p sig, Term_p term);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
