/*-----------------------------------------------------------------------

File  : clb_regmem.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A module supporting dynamic memory for local static variables that
  is still freed quasi-automatically (via a call to a cleanup
  function) when the program terminates. This is useful if there is a
  need for dynamically growing (or shrinking) persistent memory "owned"
  by a function. I still want this cleaned up at the end to keep the
  usefulness of the memory counters in detecting (and hence avoiding)
  memory leaks.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 13 01:48:00 CET 2012
    New

-----------------------------------------------------------------------*/

#include "clb_regmem.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static PTree_p reg_mem_tree = NULL;

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
// Function: RegMemAlloc()
//
//   Allocate a registered memory area.
//
// Global Variables: -
//
// Side Effects    : Memory operations (duh!)
//
/----------------------------------------------------------------------*/

void* RegMemAlloc(size_t size)
{
   void *res = SecureMalloc(size);

   PTreeStore(&reg_mem_tree, res);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: RegMemRealloc()
//
//   Realloc a registererd memory area.
//
// Global Variables: -
//
// Side Effects    : Memory operations (duh!)
//
/----------------------------------------------------------------------*/

void* RegMemRealloc(void* mem, size_t size)
{
   void *res;

   PTreeDeleteEntry(&reg_mem_tree, mem);
   res = SecureRealloc(mem,size);
   PTreeStore(&reg_mem_tree, res);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: RegMemFree()
//
//   Free a registered memory area.
//
// Global Variables: -
//
// Side Effects    : Memory operations (duh!)
//
/----------------------------------------------------------------------*/

void  RegMemFree(void* mem)
{
   PTreeDeleteEntry(&reg_mem_tree, mem);
   FREE(mem);
}


/*-----------------------------------------------------------------------
//
// Function: RegMemProvide()
//
//   Return pointer to a memory section that is large enough to store
//   newsize bytes, with the first *oldsize bytes being initialized to
//   the value at *mem, and the rest being initialized to '0'. If
//   newsize <= oldsize, this is a NOP. If new memory is allocated,
//   oldsize will be updated to reflect the new size (which most
//   likely is larger than newsize).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void* RegMemProvide(void* mem, size_t *oldsize, size_t newsize)
{
   size_t newlimit;

   if(*oldsize >= newsize)
   {
      return mem;
   }
   newlimit = MAX(*oldsize, (size_t)1);
   while(newlimit < newsize)
   {
      newlimit *= 2;
   }
   mem = RegMemRealloc(mem, newlimit);
   memset(mem+*oldsize, 0, newlimit-*oldsize);
   *oldsize = newlimit;

   return mem;
}


/*-----------------------------------------------------------------------
//
// Function: RegMemCleanUp()
//
//   Free all registered memory areas.
//
// Global Variables: -
//
// Side Effects    : Memory operations (duh!)
//
/----------------------------------------------------------------------*/

void  RegMemCleanUp(void)
{
   void* mem;

   while(reg_mem_tree)
   {
      mem = PTreeExtractRootKey(&reg_mem_tree);
      FREE(mem);
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


