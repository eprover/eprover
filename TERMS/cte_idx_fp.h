/*-----------------------------------------------------------------------

File  : cte_idx_fp.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Compute a fingerprint of a term suitable for fingerprint indexing. A
  fingerprint is a vector of individual samples for positions p, where
  the result is t|p->f_code if p is a position in t, BELOW_VAR
  (=LONG_MIN) if p<=q, t|q=Xn, 0 otherwise.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Feb 20 19:19:23 EET 2010
    New

-----------------------------------------------------------------------*/

#ifndef CTE_IDX_FP

#define CTE_IDX_FP

#include <stdarg.h>
#include <cte_termtypes.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Fingerprints of n elements are FunCode (long) arrays, with the
 * first element containing the lenghts (inclusive), the others the
 * results of the sampling */

typedef FunCode *IndexFP_p;

typedef IndexFP_p (*FPIndexFunction)(Term_p t);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define   BELOW_VAR     -2
#define   ANY_VAR       -1
#define   NOT_IN_TERM    0
FunCode   TermFPSample(Term_p term, ...);
IndexFP_p IndexFP0Create(Term_p t);
IndexFP_p IndexFP1Create(Term_p t);
IndexFP_p IndexFP7Create(Term_p t);
void      IndexFPFree(IndexFP_p junk);
void      IndexFPPrint(FILE* out, IndexFP_p fp);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





