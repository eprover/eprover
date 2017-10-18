/*-----------------------------------------------------------------------

File  : ccl_axiomsorter.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code implementing axiom sorting.

  Copyright 2009 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 14 01:10:37 CEST 2009
    New

-----------------------------------------------------------------------*/

#include "ccl_axiomsorter.h"



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
// Function: WAxiomAlloc()
//
//   Allocate and initialze a weighted axiom cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WAxiom_p WAxiomAlloc(void* axiom, AxiomType type)
{
   WAxiom_p ax = WAxiomCellAlloc();

   ax->weight = 0.0;
   switch(type)
   {
   case ClauseAxiom:
         ax->ax.clause = axiom;
         break;
   case FormulaAxiom:
         ax->ax.form = axiom;
         break;
   default:
         assert(false);
   }
   return ax;
}


/*-----------------------------------------------------------------------
//
// Function: WAxiomAddRelEval()
//
//   Given a vector of relevance levels for the symbols, assign a
//   clause or formula (in a WAxiom data structure) the average
//   relevancy of its symbols.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void WAxiomAddRelEval(WAxiom_p ax, Sig_p sig, PDArray_p rel_vec)
{
   PStack_p      f_codes = PStackAlloc();
   double        sum = 0.0;
   long          count = 0, rel;
   FunCode       f;
   PStackPointer i;

   switch(ax->type)
   {
   case ClauseAxiom:
         ClauseReturnFCodes(ax->ax.clause, f_codes);
         break;
   case FormulaAxiom:
         WFormulaReturnFCodes(ax->ax.form, f_codes);
         break;
   default:
         assert(false);
   }
   for(i=0; i<PStackGetSP(f_codes); i++)
   {
      f = PStackElementInt(f_codes, i);
      if(!SigIsSpecial(sig, f))
      {
         rel = PDArrayElementInt(rel_vec, f);
         if(rel)
         {
            sum+= rel;
            count++;
         }
      }
   }
   if(count)
   {
      ax->weight = sum/count;
   }
   PStackFree(f_codes);
}



/*-----------------------------------------------------------------------
//
// Function: WAxiomCmp()
//
//   Compare two WAxioms by weight, with some extra work to make the
//   ordering total.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int WAxiomCmp(WAxiom_p s1, WAxiom_p s2)
{
   if(s1->weight < s2->weight)
   {
      return -1;
   }
   if(s1->weight > s2->weight)
   {
      return 1;
   }
   if(s1->type < s2->type)
   {
      return -1;
   }
   if(s1->type > s2->type)
   {
      return 1;
   }
   return PCmp(s1, s2);
}

/*-----------------------------------------------------------------------
//
// Function: WAxiomCmpWrapper()
//
//   Compare two IntOrP's pointing to WAxioms by WAxiom weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int WAxiomCmpWrapper(const void* s1, const void* s2)
{
   const IntOrP* step1 = (const IntOrP*) s1;
   const IntOrP* step2 = (const IntOrP*) s2;

   return WAxiomCmp(step1->p_val, step2->p_val);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


