/*-----------------------------------------------------------------------

File  : cio_fileops.h

Author: Stephan Schulz

Contents

  Simple operations on files, with error-checking. 

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 28 12:48:11 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cio_fileops.h"



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
// Function: FileErrorPrint()
//
//   Print an errpr message about failed file opening.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void FileOpenErrorPrint(char* name)
{
   TmpErrno = errno; /* Save error number, the following call to
                        sprintf() can theoretically alter  the
                        value !*/
   sprintf(ErrStr, "Cannot open file %s for reading", name);
   SysError(ErrStr, FILE_ERROR);
}


/*-----------------------------------------------------------------------
//
// Function: InputOpen()
//
//   Open an input file for reading. NULL and "-" are stdin. If fail
//   is true, terminate with error, otherwise pass  error down.
//
// Global Variables: -
//
// Side Effects    : No significant ones (I hope)
//
/----------------------------------------------------------------------*/

FILE* InputOpen(char *name, bool fail)
{
   FILE* in;
   
   if(name && strcmp(name,"-")!= 0)
   {
      VERBOUTARG("Input file is ", name);
      
      in = fopen(name, "r");

      if(fail && !in)
      {
         FileOpenErrorPrint(name);
      }
   }
   else
   {
      VERBOUT("Input is coming from <stdin>\n");
      in = stdin;
   }
   return in;
}

/*-----------------------------------------------------------------------
//
// Function: InputClose()
//
//   Close an input file.
//
// Global Variables: -
//
// Side Effects    : As above ;-)
//
/----------------------------------------------------------------------*/

void  InputClose(FILE* file)
{
   VERBOUT("Closing input\n");
   if(file != stdin)
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
// Function: ConcatFiles()
//
//   Concatenate all file in (NULL-terminated) array sources into
//   target. "-" is stdin, as always. Return number of files
//   concated. 
//
// Global Variables: -
//
// Side Effects    : Writes a new file ;-)
//
/----------------------------------------------------------------------*/

long ConcatFiles(char* target, char** sources)
{
   FILE *in, *out;
   int i;
   int c;

   out = OutOpen(target);

   for(i=0; sources[i]; i++)
   {
      in = InputOpen(sources[i], true);
      while((c = getc(in))!= EOF)
      {
	 putc(c, out);
      }
      InputClose(in);
   }
   OutClose(out);   

   return i;
}




/*-----------------------------------------------------------------------
//
// Function: FileRemove()
//
//   Remove a arbitrary file.
//
// Global Variables: -
//
// Side Effects    : Removes file.
//
/----------------------------------------------------------------------*/

void FileRemove(char* name)
{
   VERBOUTARG("Removing ", name);
   if(unlink(name)!=0)
   {
      TmpErrno = errno;
      sprintf(ErrStr, "Cannot remove file %s", name);
      SysError(ErrStr, FILE_ERROR);      TmpErrno = errno;
      SysError("Could not remove temporary file", SYS_ERROR);
   }
}

/*-----------------------------------------------------------------------
//
// Function: FileBaseName()
//
//   Return a pointer to the first character of the last file name
//   component of name. 
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

char *FileBaseName(char* name)
{
   char *res = name, *step;

   for(step=name; *step; step++)
   {
      if(*step == '/')
      {
	 res = step+1;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FilePrint()
//
//   Print the contents of the named file to out.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void  FilePrint(FILE* out, char* name)
{
   FILE* in;
   int   c;
   
   in = InputOpen(name, true);
   while((c = getc(in))!=EOF)
   {
      putc(c, out);
   }
   InputClose(in);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


