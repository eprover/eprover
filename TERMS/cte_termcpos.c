/*-----------------------------------------------------------------------

File  : cte_termcpos.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  See cte_termcpos.h file 

  Copyright 2006 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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
   Term_p res=0;

   if(pos == 0)
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

TermCPos TermCPosFromTermPos(TermPos_p termpos);
bool     TermPosFromTermCPos(Term_p term, TermCPos pos);


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


