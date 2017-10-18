/*-----------------------------------------------------------------------

File  : cto_cmpcache.h

Author: Stephan Schulz

Contents

  Cache structure for the local caching of ordering results for LPO
  (and potentially RPO and other mainly recursive orderings).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Dec 25 00:50:42 MET 1999
    New

-----------------------------------------------------------------------*/

#ifndef CTO_CMPCACHE

#define CTO_CMPCACHE

#include <clb_partial_orderings.h>
#include <clb_quadtrees.h>
#include <cte_termbanks.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Compare results are organized as a quadtree, indexed by terms and
   deref-counters. Keys are always ordered (smaller key first) to
   allow retrieval of symmetric comparisons. We do not use a hash
   table to keep the fixed overhead for easy comparisons small. */

typedef QuadTree_p CmpCache_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define CmpCacheInit(cache)  assert(!(cache))
#define CmpCacheClear(cache) QuadTreeFree(cache);(cache)=NULL

CompareResult CmpCacheFind(CmpCache_p *cache, Term_p t1, DerefType d1,
            Term_p t2, DerefType d2);
bool CmpCacheInsert(CmpCache_p *cache, Term_p t1, DerefType d1, Term_p
          t2, DerefType d2, CompareResult insert);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





