/*-----------------------------------------------------------------------

File  : cpr_propsig.h

Author: Stephan Schulz

Contents

  Definitions for dealing with signatures for propositional variables
  - essentially juat associating a name with an internal number and
  vice versa.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr 24 16:19:17 CEST 2003
    New (partially from cte_signature.h>

-----------------------------------------------------------------------*/

#ifndef CPR_PROPSIG

#define CPR_PROPSIG

#include <clb_stringtrees.h>
#include <clb_pdarrays.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct propsigcell
{
   PStack_p  enc_to_name;
   StrTree_p name_to_enc;
}PropSigCell, *PropSig_p;


typedef long PLiteralCode;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PropSigCellAlloc() (PropSigCell*)SizeMalloc(sizeof(PropSigCell))
#define PropSigCellFree(junk)            SizeFree(junk, sizeof(PropSigCell))
#define PLiteralNoLit 0
#define PAtomP(code) (code>0)


PropSig_p PropSigAlloc(void);
void      PropSigFree(PropSig_p junk);

PLiteralCode PropSigGetAtomEnc(PropSig_p psig, char* name);
PLiteralCode PropSigInsertAtom(PropSig_p psig, char* name);
char*     PropSigGetAtomName(PropSig_p psig, PLiteralCode atom);
#define   PropSigAtomNumber(psig) (PStackGetSP((psig)->enc_to_name))
void      PropSigPrint(FILE* out, PropSig_p sig);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





