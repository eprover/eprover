/*-----------------------------------------------------------------------

  File  : che_hcbadmin.h

  Author: Stephan Schulz

  Contents

  Functions for administrating and parsing sets of heuristics.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Dec  8 22:27:02 MET 1998

  -----------------------------------------------------------------------*/

#ifndef CHE_HCB_ADMIN

#define CHE_HCB_ADMIN

#include <che_hcb.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct hcbadmincell
{
   PStack_p names;
   PStack_p hcb_set;
}HCBAdminCell, *HCBAdmin_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define HCBAdminCellAlloc() (HCBAdminCell*)SizeMalloc(sizeof(HCBAdminCell))
#define HCBAdminCellFree(junk)         SizeFree(junk, sizeof(HCBAdminCell))

HCBAdmin_p HCBAdminAlloc(void);
void       HCBAdminFree(HCBAdmin_p junk);
long       HCBAdminAddHCB(HCBAdmin_p set, char* name, HCB_p hcb);
HCB_p      HCBAdminFindHCB(HCBAdmin_p set, char* name);

HCB_p      HeuristicParse(Scanner_p in, WFCBAdmin_p wfcbs, OCB_p ocb,
                          ProofState_p state);
long       HeuristicDefParse(HCBAdmin_p set, Scanner_p in, WFCBAdmin_p
                             wfcbs, OCB_p ocb, ProofState_p state);
long       HeuristicDefListParse(HCBAdmin_p set, Scanner_p in,
                                 WFCBAdmin_p wfcbs, OCB_p ocb,
                                 ProofState_p state);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
