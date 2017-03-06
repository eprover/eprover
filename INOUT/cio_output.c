/*-----------------------------------------------------------------------

File  : cio_output.c

Author: Stephan Schulz

Contents

  Simple functions for secure opening of output files with -
  convention and error checking. Much simpler than the input, because
  much less can go wrong with output...

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Nov 28 11:55:59 MET 1997
    New

-----------------------------------------------------------------------*/


#include <cio_output.h>



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


long  OutputLevel = 1;
FILE* GlobalOut;
int   GlobalOutFD = STDOUT_FILENO;

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
// Function: OpenGlobalOut()
//
//   Set GobalOut to a FILE* connected to file outname, set
//   GlobalOutFD accordingly.
//
// Global Variables: GlobalOut, GlobalOutFD
//
// Side Effects    : As above.
//
/----------------------------------------------------------------------*/

void OpenGlobalOut(char* outname)
{
   GlobalOut   = OutOpen(outname);
   GlobalOutFD = fileno(GlobalOut);
}



/*-----------------------------------------------------------------------
//
// Function: OutOpen()
//
//   Open a file for writing and return it, with error checking. "-"
//   and NULL are both taken to mean stdout.
//
// Global Variables: -
//
// Side Effects    : Opens file
//
/----------------------------------------------------------------------*/

FILE* OutOpen(char* name)
{
   FILE* out;

   if(name && strcmp(name,"-")!= 0)
   {
      VERBOUTARG("Output file is ", name);

      if(! (out = fopen(name,"w")))
      {
    TmpErrno = errno; /* Save error number, the following call to
             sprintf() can theoretically alter  the
             value !*/
    sprintf(ErrStr, "Cannot open file %s", name);
         SysError(ErrStr, FILE_ERROR);
      }
   }
   else
   {
      VERBOUT("Output is going to <stdout>\n");
      out = stdout;
   }
   clearerr(out);
   return out;
}


/*-----------------------------------------------------------------------
//
// Function: OutClose()
//
//   Close the file, checking for errors. If stdout, just flush
//   it. Error messages are bound to be short, but errors should only
//   result from program errors or extremely obscure circumstances.
//
// Global Variables: -
//
// Side Effects    : Closes file
//
/----------------------------------------------------------------------*/

void OutClose(FILE* file)
{
   fflush(file);
   if(ferror(file))
   {
      Error("Output stream to be closed reports error (probably "
            "broken pipe, file system full or quota exceeded)",
            FILE_ERROR);
   }

   VERBOUT("Closing output\n");
   if(file != stdout)
   {
      if(fclose(file) != 0)
      {
         TmpErrno = errno;
         SysError("Error while closing file", FILE_ERROR);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PrintDashedStatuses()
//
//   This is a weird simple thing needed far above. If stat1 and stat2
//   are NULL, print fallback. If either is non-NULL, print it. If both
//   are non-NULL, print them both separated by a dash.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PrintDashedStatuses(FILE* out, char* stat1, char* stat2, char* fallback)
{
   assert(fallback);

   if(stat1 && stat2)
   {
      fprintf(out, "%s-%s", stat1, stat2);
   }
   else if(stat1)
   {
      fputs(stat1, out);
   }
   else if(stat2)
   {
      fputs(stat2, out);
   }
   else
   {
      fputs(fallback, out);
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


