/*-----------------------------------------------------------------------

File  : cio_fileops.h

Author: Stephan Schulz

Contents

  Simple operations on files, with error-checking.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
   int statres;
   struct stat stat_buf;

   if(name && strcmp(name,"-")!= 0)
   {
      VERBOUTARG2("Trying file ", name);
      in = fopen(name, "r");

      statres = stat (name, &stat_buf);
      if(statres != 0)
      {
         in = NULL;
         if(fail)
         {
            TmpErrno = errno;
            SysError("Cannot stat file %s", FILE_ERROR, name);
         }
      }
      else if(!S_ISREG (stat_buf.st_mode))
      {
         in = NULL;
         if(fail)
         {
            Error("%s it is not a regular file", FILE_ERROR, name);
         }
      }

      if(fail && !in)
      {
         TmpErrno = errno;
         SysError("Cannot open file %s for reading", FILE_ERROR , name);
      }
      if(fail)
      {
         VERBOUTARG("Input file is ", name);
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
// Function: FileLoad()
//
//   Load the content of the named file and append it to dest. Returns
//   number of characters read.
//
// Global Variables: -
//
// Side Effects    : Memory, IO
//
/----------------------------------------------------------------------*/

long  FileLoad(char* name, DStr_p dest)
{
   FILE* in;
   long count=0;
   int c;

   in = InputOpen(name, true);

   while((c = getc(in))!= EOF)
   {
      count++;
      DStrAppendChar(dest, c);
   }
   InputClose(in);

   return count;
}



/*-----------------------------------------------------------------------
//
// Function: ConcatFiles()
//
//   Concatenate all file in (NULL-terminated) array sources into
//   target. "-" is stdin, as always. Return number of files
//   concated. This could be much optimized. Let me know if it ever
//   shows up in a profile...
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
// Function: CopyFile()
//
//   Copy source to target (the lazy way ;-). Notice argument order
//   (compatible with = and strcpy(), not with cp!)
//
// Global Variables: -
//
// Side Effects    : Via ConcatFiles()
//
/----------------------------------------------------------------------*/

long CopyFile(char* target, char* source)
{
   char* tmp[2];

   tmp[0] = source;
   tmp[1] = NULL;

   return ConcatFiles(target, tmp);
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


/*-----------------------------------------------------------------------
//
// Function: FileNameDirName()
//
//   Given a path name, return the directory portion (i.e. the part
//   from the first character to the last / character (including
//   it). Return "" if no directory part exists. It is the users
//   responsibility to FREE the memory returned.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* FileNameDirName(char* name)
{
   char *res;
   int i, endpos = 0;

   assert(name);

   for(i=0; name[i]; i++)
   {
      if(name[i] == '/')
      {
         endpos = i+1;
      }
   }
   res = SecureStrndup(name, endpos);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FileFindBaseName()
//
//   Return a pointer to the first character of the last file name
//   component of name.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char *FileFindBaseName(char* name)
{
   char *res = name;

   for( ; *name; name++)
   {
      if(*name == '/')
      {
    res = name+1;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FileNameBaseName()
//
// Given a path, return a copy of the base name part of it, i.e. the
// string starting at the last / (if any). In contrast to the UNIX
// command 'basename', it will return the empty string for a string
// ending in "/".
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* FileNameBaseName(char* name)
{
   char *res, *endpos = name;

   endpos = FileFindBaseName(name);
   res = SecureStrdup(endpos);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FileNameStrip()
//
//   Given a path, return a copy of the core name - i.e. the basename
//   without a possible suffix.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* FileNameStrip(char* name)
{
   char *res, *endpos = name;
   int len = 0, i;


   endpos = FileFindBaseName(name);
   for(i=0; endpos[i]; i++)
   {
      if(endpos[i] == '.')
      {
         len = i;
      }
   }
   if(!len)
   {
      len = i;
   }
   res = SecureStrndup(endpos, len);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FileExists()
//
//   Return true if file exists and can be opened for reading, false
//   otherwise. This is not race-safe!
//
// Global Variables: -
//
// Side Effects    : Tries to open the file.
//
/----------------------------------------------------------------------*/

bool FileExists(char* name)
{
   FILE* fp;

   fp = fopen(name, "r");
   if(fp)
   {
      fclose(fp);
      return true;
   }
   return false;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
