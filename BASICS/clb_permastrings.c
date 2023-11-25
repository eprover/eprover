/*-----------------------------------------------------------------------

  File  : clb_permastrings.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  A simple registry maintaining permanent copies of strings until the
  registry is explicitly cleared.

  Copyright 2023 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Fri Nov 24 15:01:19 CET 2023

  -----------------------------------------------------------------------*/

#include "clb_permastrings.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static StrTree_p perma_anchor = NULL;


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
// Function: PermaString()
//
//   Register a string. Will return a pointer to a permanent (possibly
//   shared) copy of the string that is valid until PermaStringsFree()
//   is called.
//
// Global Variables: perma_anchor
//
// Side Effects    : Memory operations, reorganises stored tree.
//
/----------------------------------------------------------------------*/

char* PermaString(char* str)
{
   StrTree_p handle, old;
   char *res;

   if(!str)
   {
      return NULL;
   }
   handle = StrTreeCellAlloc();
   handle->key = SecureStrdup(str);
   assert(handle->key != str);
   handle->val1.i_val = 0;
   handle->val2.i_val = 0;

   old = StrTreeInsert(&perma_anchor, handle);

      if(!old)
   {
      res = handle->key;
      assert(res!=str);
   }
   else
   {
      FREE(handle->key);
      StrTreeCellFree(handle);
      res = old->key;
      assert(res!=str);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PermaStringStore()
//
//   As PermaString, but will FREE the original.
//
// Global Variables: perma_anchor
//
// Side Effects    : Memory operations, reorganises stored tree.
//
/----------------------------------------------------------------------*/

char* PermaStringStore(char* str)
{
   if(!str)
   {
      return str;
   }
   char* res = PermaString(str);
   FREE(str);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PermaStringsFree()
//
//   Free all permastrings (and their admin data structure).
//
// Global Variables: perma_anchor
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void  PermaStringsFree(void)
{
   StrTreeFree(perma_anchor);
   perma_anchor = NULL;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
