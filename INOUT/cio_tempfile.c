/*-----------------------------------------------------------------------

File  : cio_tempfile.c

Author: Stephan Schulz

Contents

  Functions for temporary files.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul 24 02:26:32 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cio_tempfile.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static StrTree_p temp_file_store = NULL;

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
// Function: TempFileCleanup()
//
//   Remove all temporary files.
//
// Global Variables: temp_file_store
//
// Side Effects    : Deletes files
//
/----------------------------------------------------------------------*/

void TempFileCleanup(void)
{
   while(temp_file_store)
   {
      VERBOUTARG("Removing termorary file ", temp_file_store->key);
      if(unlink(temp_file_store->key))
      {
         sprintf(ErrStr, "Could not remove temporary file %s",
                 temp_file_store->key);
         Warning(ErrStr);
      }
      StrTreeDeleteEntry(&temp_file_store, temp_file_store->key);
   }
}

/*-----------------------------------------------------------------------
//
// Function: TempFileRegister()
//
//   Register a file as temporary and to remove at exit.
//
// Global Variables: temp_file_store
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TempFileRegister(char *name)
{
   StrTree_p res;
   IntOrP tmp;

   tmp.p_val = NULL;
   res = StrTreeStore(&temp_file_store, name, tmp, tmp);
   UNUSED(res); assert(res);
}


/*-----------------------------------------------------------------------
//
// Function: TempFileName()
//
//   Allocate and register a new temporary file name. The caller has
//   to free the name!
//
// Global Variables: temp_file_store
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

char* TempFileName(void)
{
   int fd;
   char *res, *tmp;
   DStr_p name = DStrAlloc();

   tmp = getenv("TMPDIR");
   if(tmp)
   {
      DStrAppendStr(name, tmp);
   }
   else
   {
      DStrAppendStr(name, "/tmp");
   }
   if(DStrLen(name) && DStrLastChar(name)!='/')
   {
      DStrAppendChar(name, '/');
   }
   DStrAppendStr(name, "epr_XXXXXX");

   fd = mkstemp(DStrView(name));

   if(fd==-1)
   {
      TmpErrno = errno;
      SysError("Could not create valid temporary file name %s (check $TMPDIR)",
               FILE_ERROR, DStrView(name));
   }
   close(fd);
   res = SecureStrdup(DStrView(name));
   DStrFree(name);
   TempFileRegister(res);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TempFileCreate()
//
//   Create a temporary file storing the data from source. Return name
//   of the created file.
//
// Global Variables: -
//
// Side Effects    : Writes file, reads input, allocates new name!
//
/----------------------------------------------------------------------*/

char* TempFileCreate(FILE* source)
{
   char* name;
   FILE* out;
   int c;

   name = TempFileName();
   out = OutOpen(name);

   while((c = getc(source))!= EOF)
   {
      putc(c, out);
   }
   OutClose(out);
   return name;
}


/*-----------------------------------------------------------------------
//
// Function: TempFileRemove()
//
//   Remove a temporary file.
//
// Global Variables: -
//
// Side Effects    : Removes file.
//
/----------------------------------------------------------------------*/

void TempFileRemove(char* name)
{
   bool res;

   if(unlink(name)!=0)
   {
      TmpErrno = errno;
      SysError("Could not remove temporary file", SYS_ERROR);
   }
   res = StrTreeDeleteEntry(&temp_file_store, name);
   UNUSED(res); assert(res);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
