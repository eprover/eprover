/*-----------------------------------------------------------------------

File  : clb_plocalstacks.c

Author: Martin Möhrmann

Contents

  Stack implementation with macros that use local variables.

  Copyright 2016 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jun 4 20:30:20 2016

  -----------------------------------------------------------------------*/

#include <clb_memory.h>

/*-----------------------------------------------------------------------
//
// Function: PLocalStackGrow()
//
//   Grow stack to have room for at least space new items.
//
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

__attribute__ ((noinline)) size_t PLocalStackGrow(void** *data, size_t size, size_t space)
{
   size_t old_size = size;
   while(size <= (old_size+space))
   {
      size *= 2;
   }
   void* tmp = SizeMalloc(size * sizeof(void*));
   memcpy(tmp, *data, old_size * sizeof(void*));
   SizeFree(*data, old_size * sizeof(void*));
   *data = tmp;
   return size;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
