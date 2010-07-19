/*-----------------------------------------------------------------------

File  : che_heuristics.h

Author: Stephan Schulz

Contents
 
  High-Level interface functions to the heuristics module.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 02:14:51 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_HEURISTICS

#define CHE_HEURISTICS

#include <che_proofcontrol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* All standard heuristics are now parsed from DefaultHeuristics in
   che_proofcontrol.c. Only non-standard heuristics get an entry
   here. */

typedef enum
{
   HEU_NO_HEURISTIC = 0,
   HEU_AUTO_MODE,
   HEU_AUTO_MODE_CASC,
   HEU_AUTO_MODE_DEV
}Heuristic;

typedef struct heuristic_assoc_cell
{
   Heuristic    heuristic;
   char*        name;
   HCBCreateFun heuristic_create;
}HeuristicAssocCell, HeuristicAssoc_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern  HeuristicAssocCell HeuristicsTable[];

HCB_p HCBAutoModeCreate(HCBARGUMENTS);
HCB_p HCBCASCAutoModeCreate(HCBARGUMENTS);
HCB_p HCBDevAutoModeCreate(HCBARGUMENTS);

HCB_p HCBCreate(char* name, HCBARGUMENTS);
HCB_p GetHeuristic(char* source, HCBARGUMENTS);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





