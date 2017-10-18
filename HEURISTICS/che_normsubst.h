/*-----------------------------------------------------------------------

File  : cle_normsubst.h

Author: Stephan Schulz

Contents

  Substitutions mapping function symbols and variables to norm
  values.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 16 01:04:12 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLE_NORMSUBST

#define CLE_NORMSUBST

#include <clb_numtrees.h>
#include <cte_signature.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct normsubstcell
{
   NumTree_p used_ids;
   NumTree_p norm_funs;
   NumTree_p norm_vars;
}NormSubstCell, *NormSubst_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define NormSubstCellAlloc() (NormSubstCell*)SizeMalloc(sizeof(NormSubstCell))
#define NormSubstCellFree(junk)        SizeFree(junk, sizeof(NormSubstCell))

NormSubst_p NormSubstAlloc(void);
void        NormSubstFree(NormSubst_p junk);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





