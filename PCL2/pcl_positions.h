/*-----------------------------------------------------------------------

File  : pcl_positions.h

Author: Stephan Schulz

Contents

  Positions in PCL2 clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 22 19:32:20 MET 2000
    New

-----------------------------------------------------------------------*/

#ifndef PCL_POSITIONS

#define PCL_POSITIONS

#include <ccl_eqn.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Position is literal, side, position in term. Except for the
   position, everything is optional. Empty term position is expressed
   by both termposlen==0 and termpos==NULL, unless I decide otherwise
   ;-) */

typedef struct pcl2poscell
{
   long      literal;
   EqnSide   side;
   long      termposlen; /* Might take a stack here, but stacks will */
   PDArray_p termpos;    /* be, on average, much to large */
}PCL2PosCell, *PCL2Pos_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PCL2PosCellAlloc() (PCL2PosCell*)SizeMalloc(sizeof(PCL2PosCell))
#define PCL2PosCellFree(junk)         SizeFree(junk, sizeof(PCL2PosCell))

PCL2Pos_p PCL2PosAlloc(void);
void      PCL2PosFree(PCL2Pos_p pos);

PCL2Pos_p PCL2PosParse(Scanner_p in);
void      PCL2PosPrint(FILE* out, PCL2Pos_p pos);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

