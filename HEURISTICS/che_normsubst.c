/*-----------------------------------------------------------------------

File  : cle_normsubst.c

Author: Stephan Schulz

Contents

   2nd order Substitutions mapping function symbols and variables to
   norm values.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 16 01:27:39 MET 1998

-----------------------------------------------------------------------*/

#include "che_normsubst.h"


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
// Function: NormSubst_p NormSubstAlloc()
//
//   Allocate an empty norm substitution.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

NormSubst_p NormSubstAlloc(void)
{
   NormSubst_p handle = NormSubstCellAlloc();

   handle->used_ids  = NULL;
   handle->norm_funs = NULL;
   handle->norm_vars = NULL;

   return handle;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


