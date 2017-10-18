/*-----------------------------------------------------------------------

File  : clb_regmem.h

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

<1> Mon Feb 13 01:47:54 CET 2012
    New

-----------------------------------------------------------------------*/

#ifndef CLB_REGMEM

#define CLB_REGMEM


#include <clb_ptrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/





/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void* RegMemAlloc(size_t size);
void* RegMemRealloc(void* mem, size_t size);
void  RegMemFree(void* mem);
void* RegMemProvide(void* mem, size_t *oldsize, size_t newsize);
void  RegMemCleanUp(void);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





