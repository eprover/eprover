/*-----------------------------------------------------------------------

  File  : cio_filevars.h

  Author: Stephan Schulz

  Contents

  Functions for managing file-stored "variable = value;" pairs.

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Thu Apr  8 16:00:49 MET DST 1999

-----------------------------------------------------------------------*/


#include "cio_filevars.h"


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
// Function: FileVarsAlloc()
//
//   Allocate an empty, initialized filevars cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FileVars_p FileVarsAlloc(void)
{
   FileVars_p handle = FileVarsCellAlloc();

   handle->names = PStackAlloc();
   handle->vars  = NULL;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FileVarsFree()
//
//   Free a file vars cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void FileVarsFree(FileVars_p handle)
{
   PStack_p  stack;
   StrTree_p cell;
   char*     name;

   while(!PStackEmpty(handle->names))
   {
      name = PStackPopP(handle->names);
      FREE(name);
   }
   PStackFree(handle->names);
   stack = StrTreeTraverseInit(handle->vars);
   while((cell = StrTreeTraverseNext(stack)))
   {
      FREE(cell->val1.p_val);
   }
   StrTreeFree(handle->vars);
   FileVarsCellFree(handle);
}


/*-----------------------------------------------------------------------
//
// Function: FileVarsParse()
//
//   Parse a set of file var definitions. Return number of variables
//   read. New definitions overwrite old ones!
//
// Global Variables: -
//
// Side Effects    : Reads input, memory operations, may terminate
//                   with error message.
//
/----------------------------------------------------------------------*/

long FileVarsParse(Scanner_p in, FileVars_p vars)
{
   char*     name;
   StrTree_p cell, test;
   long      res = 0;
   DStr_p    value = DStrAlloc();

   assert(!PStackEmpty(vars->names));
   assert(strcmp(PStackTopP(vars->names), DStrView(Source(in))) == 0);

   while(!TestInpTok(in, NoToken))
   {
      CheckInpTok(in, Identifier);
      name = DStrCopy(AktToken(in)->literal);
      cell = StrTreeFind(&(vars->vars), name);
      if(cell)
      {
         FREE(cell->val1.p_val);
      }
      else
      {
         cell = StrTreeCellAllocEmpty();
         cell->key = name;
         cell->val2.p_val = PStackTopP(vars->names);
         test = StrTreeInsert(&(vars->vars), cell);
         UNUSED(test); assert(test == NULL);
      }
      NextToken(in);
      AcceptInpTok(in, EqualSign);

      DStrReset(value);
      while(!TestInpTok(in, Semicolon))
      {
         DStrAppendDStr(value, AktToken(in)->literal);
         NextToken(in);
      }
      AcceptInpTok(in, Semicolon);
      cell->val1.p_val = DStrCopy(value);
      res++;
   }
   DStrFree(value);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FileVarsReadFromFile()
//
//   Read a set of file vars from a file (as opposed to an arbitrary
//   scanner as above).
//
// Global Variables: -
//
// Side Effects    : As FileVarsParse()
//
/----------------------------------------------------------------------*/

long FileVarsReadFromFile(char* file, FileVars_p vars)
{
   long res;
   Scanner_p in;

   in = CreateScanner(StreamTypeFile, file, true, NULL, true);
   res = FileVarsParse(in, vars);
   DestroyScanner(in);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FileVarsGetBool()
//
//   Try to get a boolean value associated with a name. If it exist,
//   set *var to the result and return true, otherwise leave *var
//   untouched and return false. If value is not boolean, exit with
//   error.
//
// Global Variables: -
//
// Side Effects    : May cause error and exit
//
/----------------------------------------------------------------------*/

bool FileVarsGetBool(FileVars_p vars, char* name, bool *value)
{
   StrTree_p cell = StrTreeFind(&(vars->vars), name);

   if(!cell)
   {
      return false;
   }
   if(strcmp(cell->val1.p_val, "true"))
   {
      *value = true;
   }
   else if(strcmp(cell->val1.p_val, "false"))
   {
      *value = false;
   }
   else
   {
      Error("Boolean value requested for file variable %s read "
            "from \"%s\", but no Boolean value present",
            INPUT_SEMANTIC_ERROR, name, cell->val2.p_val);
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function:  FileVarsGetInt()
//
//   Try to get an integer value associated with a name. If it exist,
//   set *var to the result and return true, otherwise leave *var
//   untouched and return false. If value is not integer, exit with
//   error.
//
// Global Variables: -
//
// Side Effects    : May cause error and exit
//
/----------------------------------------------------------------------*/

bool FileVarsGetInt(FileVars_p vars, char* name,  long *value)
{
   StrTree_p cell = StrTreeFind(&(vars->vars), name);
   char      *eoarg;

   if(!cell)
   {
      return false;
   }

   *value = strtol(cell->val1.p_val, &eoarg, 10);

   if(errno || *eoarg)
   {
      if(errno)
      {
         TmpErrno = errno;
         SysError("Integer value requested for file variable %s read "
                  "from \"%s\", but no Integer value present",
                  INPUT_SEMANTIC_ERROR, name, cell->val2.p_val);
      }
      else if(*eoarg)
      {
         Error("Integer value requested for file variable %s read "
               "from \"%s\", but no Integer value present",
               INPUT_SEMANTIC_ERROR, name, cell->val2.p_val);
      }
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: FileVarsGetStr()
//
//   Try to get any value associated with a name. If it exist,
//   set *var to the result and return true, otherwise leave *var
//   untouched and return false. *var will only live as long as vars!
//
// Global Variables: -
//
// Side Effects    : May cause error and exit
//
/----------------------------------------------------------------------*/

bool FileVarsGetStr(FileVars_p vars, char* name,  char **value)
{
   StrTree_p cell = StrTreeFind(&(vars->vars), name);

   if(!cell)
   {
      return false;
   }
   *value = cell->val1.p_val;

   return true;
}


/*-----------------------------------------------------------------------
//
// Function: FileVarsGetIdentifier()
//
//   Try to get an Identifier value associated with a name. If it
//   exist, set *var to the result and return true, otherwise leave
//   *var untouched and return false. If value is not integer, exit
//   with error. *var will only live as long as vars!
//
// Global Variables: -
//
// Side Effects    : May cause error and exit
//
/----------------------------------------------------------------------*/

bool FileVarsGetIdentifier(FileVars_p vars, char* name,  char **value)
{
   StrTree_p cell = StrTreeFind(&(vars->vars), name);
   Scanner_p in;

   if(!cell)
   {
      return false;
   }

   in = CreateScanner(StreamTypeInternalString, cell->val1.p_val,
                      true, NULL, true);
   if(!TestInpTok(in, Identifier))
   {
      Error("Identifier value requested for file variable %s read "
            "from \"%s\", but no identifier present",
            INPUT_SEMANTIC_ERROR, name, cell->val2.p_val);
   }
   DestroyScanner(in);
   *value = cell->val1.p_val;

   return true;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
