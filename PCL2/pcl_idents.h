/*-----------------------------------------------------------------------

File  : pcl_idents.h

Author: Stephan Schulz

Contents

  Identifiers for PCL2 - lists of posititive numbers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 22 19:32:20 MET 2000
    New

-----------------------------------------------------------------------*/

#ifndef PCL_IDENTS

#define PCL_IDENTS

#include <clb_pdarrays.h>
#include <cio_scanner.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* PCL-Idents are represented as -1-Terminated PDArrays */

#define NO_PCL_ID_ELEMENT -1

typedef PDArrayCell PCLIdCell;
typedef PDArray_p   PCLId_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PCLIdCellAlloc() (PclIdCell*)SizeMalloc(sizeof(PclIdCell))
#define PCLIdCellFree(junk)         SizeFree(junk, sizeof(PclIdCell))

#define PCLIdAlloc()    PDIntArrayAlloc(2, 2)
#define PCLIdFree(junk) PDArrayFree(junk)

PCLId_p PCLIdParse(Scanner_p in);
void    PCLIdPrintFormatted(FILE* out, PCLId_p id, bool formatted);
#define PCLIdPrint(out, id) PCLIdPrintFormatted((out),(id),false)
void    PCLIdPrintTSTP(FILE* out, PCLId_p id);
int     PCLIdCompare(PCLId_p id1, PCLId_p id2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





