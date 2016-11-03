/*-----------------------------------------------------------------------

File  : can_clausestats.h

Author: Stephan Schulz

Contents

  Simple data type to store information about clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 31 14:22:23 MET DST 1999
    New

-----------------------------------------------------------------------*/


#ifndef CAN_CLAUSESTATS

#define CAN_CLAUSESTATS

#include <clb_numtrees.h>
#include <cio_basicparser.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct clausestatscell
{
   long proof_distance;
   long simplify_used;
   long simplify_unused;
   long generate_used;
   long generate_unused;
   long subsumed;
}ClauseStatsCell, *ClauseStats_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define ClauseStatsCellAlloc()\
        (ClauseStatsCell*)SizeMalloc(sizeof(ClauseStatsCell))
#define ClauseStatsCellFree(junk)\
        SizeFree(junk, sizeof(ClauseStatsCell))

ClauseStats_p ClauseStatsCopy(ClauseStats_p stats);

void          ClauseStatsParseInto(Scanner_p in, ClauseStats_p cell);
ClauseStats_p ClauseStatsParse(Scanner_p in);
void          ClauseStatsPrint(FILE *out, ClauseStats_p cell);
void          ClauseStatsPrintNormalized(FILE *out, ClauseStats_p
                cell, long created, long
                processed);

long          ClauseSetInfoParse(Scanner_p in, NumTree_p *tree);
long          ClauseSetInfoPrint(FILE* out, NumTree_p tree);
void          ClauseStatTreeFree(NumTree_p tree);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





