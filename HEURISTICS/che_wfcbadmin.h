/*-----------------------------------------------------------------------

File  : che_wfcbadmin.h

Author: Stephan Schulz

Contents

  Functions for administrating and parsing sets of weight functions.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Dec  8 22:27:02 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_WFCB_ADMIN

#define CHE_WFCB_ADMIN

#include <che_clauseweight.h>
#include <che_dagweight.h>
#include <che_varweights.h>
#include <che_funweights.h>
#include <che_orientweight.h>
#include <che_simweight.h>
#include <che_fifo.h>
#include <che_lifo.h>
#include <che_random.h>
#include <che_learning.h>
#include <che_termweight.h>
#include <che_tfidfweight.h>
#include <che_levweight.h>
#include <che_treeweight.h>
#include <che_prefixweight.h>
#include <che_strucweight.h>
#include <che_diversityweight.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct wfcbadmincell
{
   PStack_p names;
   PStack_p wfcb_set;
   long     anon_counter;
}WFCBAdminCell, *WFCBAdmin_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define WFCBAdminCellAlloc() (WFCBAdminCell*)SizeMalloc(sizeof(WFCBAdminCell))
#define WFCBAdminCellFree(junk)         SizeFree(junk, sizeof(WFCBAdminCell))

extern char* WeightFunParseFunNames[];

WFCBAdmin_p WFCBAdminAlloc(void);
void        WFCBAdminFree(WFCBAdmin_p junk);
long        WFCBAdminAddWFCB(WFCBAdmin_p set, char* name, WFCB_p wfcb);
WFCB_p      WFCBAdminFindWFCB(WFCBAdmin_p set, char* name);

WeightFunParseFun GetWeightFunParseFun(char* name);
WFCB_p      WeightFunParse(Scanner_p in, OCB_p ocb, ProofState_p
             state);
char*       WeightFunDefParse(WFCBAdmin_p set, Scanner_p in, OCB_p
                ocb, ProofState_p state);
long         WeightFunDefListParse(WFCBAdmin_p set, Scanner_p in,
               OCB_p ocb, ProofState_p state);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
