/*-----------------------------------------------------------------------

File  : ctr_idx_fp.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Compute and handle term fingerprints for indexing.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Feb 24 01:28:18 EET 2010
    New

-----------------------------------------------------------------------*/

#include "cte_idx_fp.h"



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
// Function: TermFPSample()
//
//   Sample the term at the position described by the optional
//   arguments (encoding a (-1)-terminated position.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

FunCode TermFPSample(Term_p term, ...)
{
  va_list ap;
  va_start(ap, term);
  int pos = 0;
  FunCode res = 0;

  for(pos = va_arg(ap, int); pos != -1;  pos = va_arg(ap, int))
  {
     if(TermIsVar(term))
     {
        res = BELOW_VAR;
        break;
     }
     if(pos >= term->arity)
     {
        res = NOT_IN_TERM;
        break;
     }
     term = term->args[pos];
  }
  if(pos == -1)
  {
     res = TermIsVar(term)?ANY_VAR:term->f_code;
  }
  va_end(ap);   

  return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP0Create()
//
//   Create a dummy fingerprint structure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP0Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*1);

   res[0] = 1;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: IndexFP1Create()
//
//   Create a fingerprint structure representing top symbol hashing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP1Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*2);

   res[0] = 2;
   res[1] = TermFPSample(t, -1);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: IndexFP7Create()
//
//   Create a fingerprint structure with samples at positions epsilon,
//   0, 1, 0.0, 01, 1.0, 1.1 (using E's internal numbering). 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IndexFP_p IndexFP7Create(Term_p t)
{
   IndexFP_p res = SizeMalloc(sizeof(FunCode)*8);

   res[0] = 8;
   res[1] = TermFPSample(t, -1);
   res[2] = TermFPSample(t, 0, -1);
   res[3] = TermFPSample(t, 1, -1);
   res[4] = TermFPSample(t, 0, 0, -1);
   res[5] = TermFPSample(t, 0, 1, -1);
   res[6] = TermFPSample(t, 1, 0, -1);
   res[7] = TermFPSample(t, 1, 1, -1);

   return res;
}
   

/*-----------------------------------------------------------------------
//
// Function: IndexFPFree()
//
//   Free an IndexFP data-structure (i.e. a self-describing FunCode
//   array). 
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void IndexFPFree(IndexFP_p junk)
{
   SizeFree(junk, sizeof(FunCode)*junk[0]);
}


/*-----------------------------------------------------------------------
//
// Function: IndexFPPrint()
//
//   Print a term fingerprint.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void IndexFPPrint(FILE* out, IndexFP_p fp)
{
   int i, limit=fp[0];
   
   if(limit>=2)
   {
      fprintf(stdout, "<%ld", fp[1]);
      for(i=2; i<limit; i++)
      {
         fprintf(stdout, ",%ld", fp[i]);
      }      
      fprintf(stdout, ">");
   }   
   else
   {
      fprintf(stdout, "<>");
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


