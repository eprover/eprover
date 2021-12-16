/*-----------------------------------------------------------------------

File  : pcl_idents.c

Author: Stephan Schulz

Contents

  Identifiers for PCL2 - lists on posititive numbers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 22 19:32:20 MET 2000
    New

-----------------------------------------------------------------------*/

#include "pcl_idents.h"



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
// Function: PCLIdParse()
//
//   Parse a PCL-Identifier, i.e. a usually short list of pos-ints
//   separated by spaces.
//
// Global Variables: -
//
// Side Effects    : Input, memory allocation
//
/----------------------------------------------------------------------*/

PCLId_p PCLIdParse(Scanner_p in)
{
   PCLId_p id;
   long    i;


   CheckInpTok(in, PosInt);
   id = PCLIdAlloc();
   i=0;
   PDArrayAssignInt(id,i++,AktToken(in)->numval);
   NextToken(in);
   while(TestInpTok(in, Fullstop))
   {
      AcceptInpTok(in, Fullstop);
      PDArrayAssignInt(id,i++,AktToken(in)->numval);
      AcceptInpTok(in, PosInt);
   }
   PDArrayAssignInt(id,i,NO_PCL_ID_ELEMENT);
   return id;
}


/*-----------------------------------------------------------------------
//
// Function: PCLIdPrintFormatted()
//
//   Print a PCL identifier.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLIdPrintFormatted(FILE* out, PCLId_p id, bool formatted)
{
   long i;

   assert(id);
   assert(PDArrayElementInt(id,0)>=0);
   assert(PDArrayElementInt(id,0)!=NO_PCL_ID_ELEMENT); /* Redundant,
                       bute
                       well...erm...*/
   fprintf(out, formatted?"%7ld":"%ld", PDArrayElementInt(id,0));
   for(i=1;PDArrayElementInt(id,i)!=NO_PCL_ID_ELEMENT;i++)
   {
      fprintf(out, ".%ld", PDArrayElementInt(id,i));
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLIdPrintTSTP()
//
//   Print a PCL identifier in a format suitable for TSTP. If a single
//   number, print it, otherwise convert it to pclid<no1>_<no2>...
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCLIdPrintTSTP(FILE* out, PCLId_p id)
{
   long i;

   assert(id);
   assert(PDArrayElementInt(id,0)>=0);
   assert(PDArrayElementInt(id,0)!=NO_PCL_ID_ELEMENT); /* Redundant,
                       but
                       well...erm...*/

   if(PDArrayElementInt(id,1)==NO_PCL_ID_ELEMENT)
   {
      fprintf(out, "%ld", PDArrayElementInt(id,0));
   }
   else
   {
      fprintf(out, "pclid%ld", PDArrayElementInt(id,0));
      for(i=1;PDArrayElementInt(id,i)!=NO_PCL_ID_ELEMENT;i++)
      {
    fprintf(out, "_%ld", PDArrayElementInt(id,i));
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLIdCompare()
//
//   Compare two PCL identifiers lexicographically.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int PCLIdCompare(PCLId_p id1, PCLId_p id2)
{
   long i, e1,e2;
   long res=0;

   assert(id1&&id2);

   for(i=0; !res; i++)
   {
      e1 = PDArrayElementInt(id1,i);
      e2 = PDArrayElementInt(id2,i);
      if((e1==NO_PCL_ID_ELEMENT) && (e2==NO_PCL_ID_ELEMENT))
      {
    assert(res==0);
    break;
      }
      res = e1-e2;
   }
   return (int)res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


