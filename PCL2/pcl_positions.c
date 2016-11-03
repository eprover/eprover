/*-----------------------------------------------------------------------

File  : pcl_positions.c

Author: Stephan Schulz

Contents

  Positions in PCL2 clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 22 19:32:20 MET 2000
    New

-----------------------------------------------------------------------*/


#include "pcl_positions.h"



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
// Function: PCL2PosAlloc()
//
//   Allocate an initialized PCL2 position data structure.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PCL2Pos_p PCL2PosAlloc(void)
{
   PCL2Pos_p handle = PCL2PosCellAlloc();

   handle->literal    = -1;
   handle->side       = NoSide;
   handle->termposlen = 0;
   handle->termpos    = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: PCL2PosFree()
//
//   Free a PCL2 position.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PCL2PosFree(PCL2Pos_p pos)
{
   assert(pos);

   if(pos->termpos)
   {
      PDArrayFree(pos->termpos);
   }
   PCL2PosCellFree(pos);
}


/*-----------------------------------------------------------------------
//
// Function: PCL2PosParse()
//
//   Parse a PCL2 position of the format
//   <pos-int> [. L|R [ .<pos-int> ]*].
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

PCL2Pos_p PCL2PosParse(Scanner_p in)
{
   PCL2Pos_p handle = PCL2PosAlloc();
   long      i;

   handle->literal = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   if(TestInpTok(in, Fullstop))
   {
      NextToken(in);
      CheckInpId(in, "L|R");
      if(TestInpId(in, "L"))
      {
    handle->side = LeftSide;
      }
      else
      {
    handle->side = RightSide;
      }
      NextToken(in);

      if(TestInpTok(in, Fullstop))
      {
    handle->termpos = PDArrayAlloc(5,10);
      }
      i=0;
      while(TestInpTok(in, Fullstop))
      {
    NextToken(in);
    PDArrayAssignInt(handle->termpos,i++,AktToken(in)->numval);
    AcceptInpTok(in, PosInt);
      }
      handle->termposlen=i;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PCL2PosPrint()
//
//   Print a PCL2 position.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PCL2PosPrint(FILE* out, PCL2Pos_p pos)
{
   long i;

   assert(pos);

   fprintf(out, "%ld", pos->literal);
   if(pos->side!=NoSide)
   {
      assert((pos->side==LeftSide) || (pos->side==RightSide));
      fprintf(out, ".%c", (pos->side==LeftSide)?'L':'R');

      i=0;
      while(i<pos->termposlen)
      {
    assert(pos->termpos);
    fprintf(out, "%ld", PDArrayElementInt(pos->termpos,i++));
      }
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


