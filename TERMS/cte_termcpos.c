/*-----------------------------------------------------------------------

File  : cte_termcpos.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  See cte_termcpos.h file

  Copyright 2006 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Nov 12 14:28:24 ICT 2006
    New

-----------------------------------------------------------------------*/

#include "cte_termcpos.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


Term_p term_cpos_get_subterm(Term_p term, TermCPos* pos)
{
   int i;
   Term_p res = NULL;

   if(*pos == 0)
   {
      return term;
   }
   (*pos)--;
   for(i=0; i<term->arity; i++)
   {
      res = term_cpos_get_subterm(term->args[i], pos);
      if(res)
      {
         break;
      }
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function:
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Term_p TermCPosGetSubterm(Term_p term, TermCPos pos)
{
   return term_cpos_get_subterm(term, &pos);
}

/*-----------------------------------------------------------------------
//
// Function: TermPrintAllCPos()
//
//   Print all compact positions in a term, with the associated
//   subterm. Probably only for testing and debugging.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void TermPrintAllCPos(FILE* out, TB_p bank, Term_p term)
{
   TermCPos i;
   Term_p t;
   PStack_p stack = PStackAlloc();

   TermLinearize(stack, term);
   for(i=0; (t=TermCPosGetSubterm(term, i));  i++)
   {
      fprintf(out, "\nPos %3ld: ", i);
      TBPrintTermFull(out, bank, t);
      fprintf(out, " === ");
      TBPrintTermFull(out, bank, PStackElementP(stack, i));
   }
   fprintf(out, "\n");
   PStackFree(stack);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


