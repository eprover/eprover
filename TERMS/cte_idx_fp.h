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

extern char* FPIndexNames[];


#define MAX_PM_INDEX_NAME_LEN 20

#define   BELOW_VAR     -2
#define   ANY_VAR       -1
#define   NOT_IN_TERM    0

FunCode   TermFPSampleFO(Term_p term, va_list ap);
FunCode   TermFPFlexSampleFO(Term_p term, IntOrP* *seq);
FunCode   TermFPSample(Term_p term, ...);
#ifdef ENABLE_LFHO
FunCode   TermFPSampleHO(Term_p term, va_list ap);
FunCode   TermFPFlexSampleHO(Term_p term, IntOrP* *seq);
FunCode   TermFPFlexSample(Term_p term, IntOrP* *seq);
#else 
#define   TermFPFlexSample(term, seq)    (TermFPFlexSampleFO(term, seq))
#endif

IndexFP_p IndexFP0Create(Term_p t);
IndexFP_p IndexFPfpCreate(Term_p t);
IndexFP_p IndexFP1Create(Term_p t);
IndexFP_p IndexFP2Create(Term_p t);
IndexFP_p IndexFP3DCreate(Term_p t);
IndexFP_p IndexFP3WCreate(Term_p t);
IndexFP_p IndexFP4DCreate(Term_p t);
IndexFP_p IndexFP4WCreate(Term_p t);
IndexFP_p IndexFP4MCreate(Term_p t);
IndexFP_p IndexFP5MCreate(Term_p t);
IndexFP_p IndexFP6MCreate(Term_p t);
IndexFP_p IndexFP7Create(Term_p t);
IndexFP_p IndexFP7MCreate(Term_p t);
IndexFP_p IndexFP4X2_2Create(Term_p t);
IndexFP_p IndexFPFlexCreate(Term_p t, PStack_p pos, int len);
IndexFP_p IndexFP3DFlexCreate(Term_p t);

IndexFP_p IndexDTCreate(Term_p t);

void      IndexFPFree(IndexFP_p junk);

FPIndexFunction GetFPIndexFunction(char* name);

void      IndexFPPrint(FILE* out, IndexFP_p fp);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





