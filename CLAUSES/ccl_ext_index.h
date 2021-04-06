/*-----------------------------------------------------------------------

File  : ccl_overlap_index.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A simple index mapping symbols to ClauseTPos trees.
  See .c file for details on functionality.

Changes

<1> Thu Jun  3 11:30:36 CEST 2010
    New

-----------------------------------------------------------------------*/

#include <ccl_clauses.h>
#include <clb_intmap.h>

#ifndef CCL_EXT_DEC_IDX

#define CCL_EXT_DEC_IDX


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef IntMap_p ExtIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ExtDecAlloc()   IntMapAlloc()



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





