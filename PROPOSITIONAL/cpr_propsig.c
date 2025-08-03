/*-----------------------------------------------------------------------

File  : cpr_propsig.c

Author: Stephan Schulz

Contents

  Code for "signatures" for propositional atoms (essentially just a
  bidirectional mapping external name <-> internal encoding).

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr 24 16:28:42 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "cpr_propsig.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PropSigAlloc()
//
//   Allocate an empty, initialized propositional signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PropSig_p PropSigAlloc(void)
{
   PropSig_p handle = PropSigCellAlloc();

   handle->enc_to_name = PStackAlloc();
   PStackPushP(handle->enc_to_name, NULL); /* Don't use literal 0 */
   handle->name_to_enc = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PropSigFree()
//
//   Free a propositional signature and all associated memory.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PropSigFree(PropSig_p junk)
{
   PStackFree(junk->enc_to_name);
   StrTreeFree(junk->name_to_enc);
   PropSigCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: PropSigGetAtomEnc()
//
//   Given a name, return the encoding. Return PAtomNoAtom if name is
//   unknown.
//
// Global Variables: -
//
// Side Effects    : - (Reorganzizes name_to_enc)
//
/----------------------------------------------------------------------*/

PLiteralCode PropSigGetAtomEnc(PropSig_p psig, char* name)
{
   StrTree_p handle;

   handle = StrTreeFind(&(psig->name_to_enc),name);
   if(handle)
   {
      return handle->val1.i_val;
   }
   return PLiteralNoLit;
}


/*-----------------------------------------------------------------------
//
// Function: PropSigInsertAtom()
//
//   Insert a new atom into a psig. Return encoding. If atom already
//   exists, do nothing but returning the encoding.
//
// Global Variables: -
//
// Side Effects    : Changes psig.
//
/----------------------------------------------------------------------*/

PLiteralCode PropSigInsertAtom(PropSig_p psig, char* name)
{
   PLiteralCode enc;
   StrTree_p handle;

   enc = PropSigGetAtomEnc(psig,name);
   if(enc)
   {
      return enc;
   }
   handle = StrTreeCellAlloc();
   handle->key = SecureStrdup(name);
   enc = PropSigAtomNumber(psig);
   PStackPushP(psig->enc_to_name,handle->key);
   handle->val1.i_val = enc;
   handle = StrTreeInsert(&(psig->name_to_enc), handle);
   assert(!handle);

   return enc;
}

/*-----------------------------------------------------------------------
//
// Function: PropSigGetAtomName()
//
//   Return a pointer name of an atom. Fail on assertion if no valid
//   atom is passed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


char* PropSigGetAtomName(PropSig_p psig, PLiteralCode atom)
{
   /* printf("Atom: %ld\n",atom); */
   assert(psig);
   assert(PAtomP(atom));
   assert(atom<PropSigAtomNumber(psig));
   return PStackElementP(psig->enc_to_name, atom);
}


/*-----------------------------------------------------------------------
//
// Function: PSigPrint()
//
//   Print a PSig (mainly for debugging)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PropSigPrint(FILE* out, PropSig_p psig)
{
   long i;

   fprintf(out, COMCHAR" Propositional signature:\n");
   fprintf(out, COMCHAR" ------------------------\n");
   for(i=1; i<PropSigAtomNumber(psig); i++)
   {
      fprintf(out, COMCHAR" %6ld : %s\n", i, PropSigGetAtomName(psig,i));
   }
   fprintf(out, "\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
