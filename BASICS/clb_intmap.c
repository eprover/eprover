/*-----------------------------------------------------------------------

File  : clb_intmap.c

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Functions implementing the multi-representation N_0->void* mapping
  data type.

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Dec 27 17:34:48 CET 2004
    New

-----------------------------------------------------------------------*/

#include "clb_intmap.h"



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
// Function: 
//
//   
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

IntMap_p IntMapAlloc();
void     IntMapFree(IntMap_p map);
long     IntMapRecFree(IntMap_p map, IntMapFreeFunc free_func);


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


