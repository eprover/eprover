/*-----------------------------------------------------------------------

  File  : che_heuristics.h

  Author: Stephan Schulz

  Contents

  High-Level interface functions to the heuristics module.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Jun  8 02:14:51 MET DST 1998

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
   HEU_AUTO_MODE_DEV,
   HEU_AUTO_SCHED0,
   HEU_AUTO_SCHED1,
   HEU_AUTO_SCHED2,
   HEU_AUTO_SCHED3,
   HEU_AUTO_SCHED4,
   HEU_AUTO_SCHED5,
   HEU_AUTO_SCHED6,
   HEU_AUTO_SCHED7,
   HEU_AUTO_SCHED8,
   HEU_AUTO_SCHED9
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

HCB_p HCBCreate(char* name, HCBARGUMENTS);
HCB_p GetHeuristic(char* source, HCBARGUMENTS);

HCB_p HCBAutoModeCreate(HCBARGUMENTS);
HCB_p HCBCASCAutoModeCreate(HCBARGUMENTS);
HCB_p HCBDevAutoModeCreate(HCBARGUMENTS);

HCB_p HCBAutoSched0Create(HCBARGUMENTS);
HCB_p HCBAutoSched1Create(HCBARGUMENTS);
HCB_p HCBAutoSched2Create(HCBARGUMENTS);
HCB_p HCBAutoSched3Create(HCBARGUMENTS);
HCB_p HCBAutoSched4Create(HCBARGUMENTS);
HCB_p HCBAutoSched5Create(HCBARGUMENTS);
HCB_p HCBAutoSched6Create(HCBARGUMENTS);
HCB_p HCBAutoSched7Create(HCBARGUMENTS);
HCB_p HCBAutoSched8Create(HCBARGUMENTS);
HCB_p HCBAutoSched9Create(HCBARGUMENTS);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
