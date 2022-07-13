/*-----------------------------------------------------------------------

File  : cio_simplestuff.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Simple I/O functions needing a home.

  Copyright 2012 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Jul 27 01:36:57 CEST 2012
    New

-----------------------------------------------------------------------*/

#include "cio_simplestuff.h"



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
// Function: ReadTextBlock()
//
//   Read lines from fp until terminator is encountered (on a line by
//   itself). Note that termiantor has to end in \n for this to ever
//   work. The read text, up to, but not including, terminator, is
//   appended to result (which is not cleared!). Returns
//   success/failure.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool ReadTextBlock(DStr_p result, FILE* fp, char* terminator)
{
   char buf[256];
   char* res;

   while(true)
   {
      res = fgets(buf, 256, fp);
      if(!res)
      {
         return false;
      }
      if(strcmp(buf, terminator) == 0)
      {
         break;
      }
      DStrAppendStr(result, buf);
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: TCPReadTextBlock()
//
//   Read lines from network socket until terminator is encountered (on a line by
//   itself). Note that termiantor has to end in \n for this to ever
//   work. The read text, up to, but not including, terminator, is
//   appended to result (which is not cleared!). Returns
//   success/failure.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool TCPReadTextBlock(DStr_p result, int fd, char* terminator)
{
   char* res;
   while(true)
   {
      res = TCPStringRecvX(fd);
      if(strcmp(res, terminator) == 0)
      {
        FREE(res);
        break;
      }
      DStrAppendStr(result, res);
      FREE(res);
   }
   return true;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


