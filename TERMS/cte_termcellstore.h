/*-----------------------------------------------------------------------

File  : cte_termcellstore.h

Author: Stephan Schulz

Contents

  Abstract interface for storing term cells, implemented by a
  combination of a hashed array and term cell trees. The hash
  combines f_code and the addresses of the first two arguments. The
  array starts small and doubles whenever the average tree would
  exceed TERM_STORE_MAX_LOAD cells.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Oct  5 01:09:50 MEST 1998
    New
<2> Thu Apr 11 10:08:26 CEST 2002
    Support for mark-and-sweep garbage collection (the sweep pass) for
    term cells
<3> Wed Nov 18 2026
    Growing hash array instead of a fixed one with 32768 entries.

-----------------------------------------------------------------------*/

#ifndef CTE_TERMCELLSTORE

#define CTE_TERMCELLSTORE

#include <cte_termtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Initial number of hash buckets (a power of 2), and the average
   number of cells per bucket at which the array is doubled. */
#define TERM_STORE_INIT_SIZE 1024
#define TERM_STORE_MAX_LOAD  1

typedef struct termcellstore
{
   long    entries;
   long    arg_count;
   long    size;     /* Number of buckets, a power of 2 */
   long    mask;     /* size-1 */
   Term_p* store;    /* Array of size term cell trees */
}TermCellStoreCell, *TermCellStore_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* Note (Nov 2026): this hash spreads cells unevenly on large unit
   equality problems (about twice the ideal average bucket size). A
   multiplicative (Fibonacci) hash gave an ideal spread but no
   measurable speedup (within +-2%), probably because the trees
   adapt and this hash keeps related cells in nearby buckets. */
#define tcs_arity0hash(term) ((term)->f_code)
#define tcs_arity1hash(term) (tcs_arity0hash(term)^(((intptr_t)(term)->args[0])>>3))
#define tcs_aritynhash(term) (tcs_arity1hash(term)^(((intptr_t)(term)->args[1])>>4))

#define TermCellHash(term, mask)\
((((term)->arity == 0)?\
  tcs_arity0hash(term):\
  (((term)->arity == 1)?tcs_arity1hash(term):tcs_aritynhash(term)))\
   &(mask))

#define TermCellStoreSize(store) ((store)->size)

void    TermCellStoreInit(TermCellStore_p store);
void    TermCellStoreExit(TermCellStore_p store);

Term_p  TermCellStoreFind(TermCellStore_p store, Term_p term);
Term_p  TermCellStoreInsert(TermCellStore_p store, Term_p term);
Term_p  TermCellStoreExtract(TermCellStore_p store, Term_p term);
bool    TermCellStoreDelete(TermCellStore_p store, Term_p term);

void    TermCellStoreSetProp(TermCellStore_p store, TermProperties
              props);
void    TermCellStoreDelProp(TermCellStore_p store, TermProperties
              props);

#define TermCellStoreNodes(store) ((store)->entries)
long    TermCellStoreCountNodes(TermCellStore_p store);

long    TermCellStoreGCSweep(TermCellStore_p store, TermProperties gc_state );

void    TermCellStorePrintDistrib(FILE* out, TermCellStore_p store);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





