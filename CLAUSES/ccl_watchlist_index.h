/*-----------------------------------------------------------------------

File  : ccl_watchlist_index.h

Author: Constantin Ruhdorfer

Contents

  Interface for indexing watchlist clauses.

Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#ifndef CCL_WATCHLIST_INDEX

#define CCL_WATCHLIST_INDEX

#include <ccl_fcvindexing.h>
#include <ccl_pdtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct watchlist_index
{
   PDTree_p  demod_index; /* If used for demodulators */
   FVIAnchor_p fvindex; /* Used for non-unit subsumption */
}WatchListIndex, *WatchListIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

#endif