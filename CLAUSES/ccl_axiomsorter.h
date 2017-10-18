/*-----------------------------------------------------------------------

File  : ccl_axiomsorter.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Datatypes an code for implementing generic evaluation and sorting of
  axiomsets (clauses and formulas).

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 14 00:31:54 CEST 2009
    New

-----------------------------------------------------------------------*/

#ifndef CCL_AXIOMSORTER

#define CCL_AXIOMSORTER

#include <ccl_proofstate.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   NoAxiom,
   ClauseAxiom,
   FormulaAxiom
}AxiomType;


/* Structure for representing an axiom with weight. */

typedef struct w_axiom_cell
{
   AxiomType type;
   double    weight;
   union
   {
      WFormula_p form;
      Clause_p   clause;
   }ax;
}WAxiomCell, *WAxiom_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define WAxiomCellAlloc()    (WAxiomCell*)SizeMalloc(sizeof(WAxiomCell))
#define WAxiomCellFree(junk) SizeFree(junk, sizeof(WAxiomCell))

WAxiom_p WAxiomAlloc(void* axiom, AxiomType type);

void WAxiomAddRelEval(WAxiom_p ax, Sig_p sig, PDArray_p rel_vec);

int WAxiomCmp(WAxiom_p s1, WAxiom_p s2);
int WAxiomCmpWrapper(const void* s1, const void* s2);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





