/*-----------------------------------------------------------------------

File  : cpr_varset.h

Author: Stephan Schulz

Contents

  Data type for (multi-)sets of propositional variables, currently
  organized as doubly linked lists.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue May 13 21:37:34 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CPR_VARSET

#define CPR_VARSET

#include <cpr_propsig.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct atomset_cell
{
   PLiteralCode        atom;
   struct atomset_cell *prev;
   struct atomset_cell *succ;
}AtomSetCell, *AtomSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define AtomSetCellAlloc() (AtomSetCell*)SizeMalloc(sizeof(AtomSetCell))
#define AtomSetCellFree(junk)            SizeFree(junk, sizeof(AtomSetCell))

AtomSet_p    AtomSetAlloc(void);
void         AtomSetFree(AtomSet_p set);
#define      AtomSetEmpty(set) ((set)->prev == (set))
PLiteralCode AtomSetExtract(AtomSet_p var);
void         AtomSetInsert(AtomSet_p set, PLiteralCode atom);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





