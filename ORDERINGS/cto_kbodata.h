/*-----------------------------------------------------------------------

File  : cto_kbodata.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Data structure for storing information about KBO for the linear time 
  KBO implementation described in [Loechner:JAR-2006]. This is a
  mixture of KBO4, KBO5 and KBO6 - because E's data structures are
  variable-normalized, the initialization/reset optimizations cannot,
  in most cases pay off and are omitted.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Feb 13 19:15:01 EET 2010
    New

-----------------------------------------------------------------------*/

#ifndef CTO_KBODATA

#define CTO_KBODATA

#include <clb_partial_orderings.h>
#include <clb_pdarrays.h>
#include <cte_termtypes.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct kbo_lin_cell
{
   CompareResult   res;
   int             wb;
   int             pos_bal;
   int             neg_bal;
   FunCode         max_var;
   PDArray_p       vb;
}KBOLinCell, *KBOLin_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define KBOLinCellAlloc()    (KBOLinCell*)SizeMalloc(sizeof(KBOLinCell))
#define KBOLinCellFree(junk) SizeFree(junk, sizeof(KBOLinCell))

KBOLin_p KBOLinAlloc(void);
void     KBOLinFree(KBOLin_p junk);
void     __inline__ KBOLinReset(KBOLin_p kbobal);


void __inline__ KBOLinReset(KBOLin_p kbobal)
{
   for(size_t i=0; i<=kbobal->max_var; i++)
   {
      PDArrayElementClear(kbobal->vb, i);
   }
   kbobal->res     = to_equal;
   kbobal->wb      = 0;
   kbobal->pos_bal = 0;
   kbobal->neg_bal = 0;
   kbobal->max_var = 0;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





