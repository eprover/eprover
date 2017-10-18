/*-----------------------------------------------------------------------

File  : cio_initio.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Trivial initialization code.

  Copyright 2005 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Mar 17 11:27:30 UYT 2005
    New

-----------------------------------------------------------------------*/

#include "cio_initio.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


char* TPTP_dir = NULL;

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
// Function: InitIO()
//
//   Initialize I/O. Bundles a number of other initializations in one
//   call.
//
// Global Variables: TPTP_dir
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void InitIO(char* progname)
{
   char* tmp;

   InitOutput();
   InitError(progname);

   tmp = getenv("TPTP");
   if(tmp)
   {
      DStr_p tmpstr = DStrAlloc();

      DStrAppendStr(tmpstr, tmp);
      if(DStrLen(tmpstr) && DStrLastChar(tmpstr)!='/')
      {
         DStrAppendChar(tmpstr, '/');
      }
      TPTP_dir = DStrCopy(tmpstr);

      DStrFree(tmpstr);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ExitIO()
//
//   Clear up (variables)
//
// Global Variables: TPTP_dir
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


void ExitIO(void)
{
   if(TPTP_dir)
   {
      FREE(TPTP_dir);
      TPTP_dir = NULL;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


