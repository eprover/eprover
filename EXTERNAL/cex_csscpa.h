/*-----------------------------------------------------------------------

File  : cex_csscpa.h

Author: Stephan Schulz, Geoff Sutcliffe

Contents

  Functions and datetype realizing the CSSCPA control component.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Apr 10 00:10:07 GMT 2000
    New

-----------------------------------------------------------------------*/

#ifndef CEX_CSSCPA

#define CEX_CSSCPA

#include <cio_output.h>
#include <ccl_subsumption.h>
#include <ccl_tautologies.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct csscpa_state_cell
{
   Sig_p             sig;
   TB_p              terms;
   TB_p              tmp_terms;
   ClauseSet_p       pos_units;
   ClauseSet_p       neg_units;
   ClauseSet_p       non_units;
   long              literals;
   long              clauses;
   long              weight;
}CSSCPAStateCell, *CSSCPAState_p;

typedef enum
{
/*---Added by Geoff */
   contradicts,
   improved,
   rejected,
   forced,
   requested,
   unknown
}
ClauseStatusType;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define CSSCPAStateCellAlloc()    (CSSCPAStateCell*)SizeMalloc(sizeof(CSSCPAStateCell))
#define CSSCPAStateCellFree(junk) SizeFree(junk, sizeof(CSSCPAStateCell))

CSSCPAState_p CSSCPAStateAlloc(void);
void          CSSCPAStateFree(CSSCPAState_p junk);

bool CSSCPAProcessClause(CSSCPAState_p state, Clause_p clause,
         bool accept, float weight_delta, float average_delta);

void CSSCPALoop(Scanner_p in, CSSCPAState_p state);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
