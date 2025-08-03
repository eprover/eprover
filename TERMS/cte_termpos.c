/*-----------------------------------------------------------------------

File  : cte_termpos.c

Author: Stephan Schulz

Contents

  Positions in terms

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun May 10 17:40:18 MET DST 1998

-----------------------------------------------------------------------*/

#include "cte_termpos.h"



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
// Function:  TermPosNextLIPosition()
//
//   Given an (implicit) term and a position, compute the next
//   position (in leftmost-innermost order) and return the
//   corresponding term.
//
// Global Variables: -
//
// Side Effects    : Changes pos
//
/----------------------------------------------------------------------*/

Term_p TermPosNextLIPosition(TermPos_p pos)
{
   Term_p super;
   int    idx;

   if(PStackEmpty(pos))
   {
      return NULL;
   }
   idx = PStackPopInt(pos);
   super = PStackPopP(pos);
   if(idx<super->arity-1)
   {
      idx++;
      while(super->arity)
      {
    PStackPushP(pos, super);
    PStackPushInt(pos, idx);
    super = super->args[idx];
    idx = 0;
      }
   }
   return super;
}


/*-----------------------------------------------------------------------
//
// Function: TermPosPrint()
//
//   Print the position as a doted list.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TermPosPrint(FILE* out, TermPos_p pos)
{
   PStackPointer i;

   if(PStackEmpty(pos))
   {
      return;
   }
   fprintf(out, "%ld", PStackElementInt(pos, 1));
   for(i=2; i<PStackGetSP(pos); i+=2)
   {
      fprintf(out, ".%ld\n", PStackElementInt(pos, i+1));
   }
}



/*-----------------------------------------------------------------------
//
// Function: TermPosDebugPrint()
//
//   Print a position in a term. If sig!=NULL, print terms, otherwise
//   print adddresses
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void TermPosDebugPrint(FILE* out, Sig_p sig, TermPos_p pos)
{
   PStackPointer i;

   fprintf(out, COMCHAR" TermPos--\n");
   for(i=0; i<PStackGetSP(pos); i+=2)
   {
      fprintf(out, COMCHAR" ");
      if(sig)
      {
    TermPrint(out, PStackElementP(pos, i), sig, DEREF_NEVER);
    fprintf(out, "...");
    TermPrint(out, PStackElementP(pos, i), sig, DEREF_ALWAYS);
      }
      else
      {
    fprintf(out, "<%p>", PStackElementP(pos, i));
      }
      fprintf(out, " Subterm %ld\n", PStackElementInt(pos, i+1));
   }
   fprintf(out, COMCHAR" --TermPos\n");
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
