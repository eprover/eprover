/*-----------------------------------------------------------------------

File  : cle_kbdesc.c

Author: Stephan Schulz

Contents

  Dealing with kb-descriptions.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul 27 16:07:14 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cle_kbdesc.h"
#include "e_version.h"



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
// Function: KBDescAlloc()
//
//   Return an initialized KBDesc-Cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

KBDesc_p KBDescAlloc(char* version, double neg_prop, long
           neg_examples)
{
   KBDesc_p handle = KBDescCellAlloc();

   handle->version           = SecureStrdup(version);
   handle->neg_proportion    = neg_prop;
   handle->fail_neg_examples = neg_examples;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: KBDescFree()
//
//   Free a KBDesc.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void KBDescFree(KBDesc_p desc)
{
   FREE(desc->version);
   KBDescCellFree(desc);
}

/*-----------------------------------------------------------------------
//
// Function: KBDescPrint()
//
//   Print a kb-description.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void KBDescPrint(FILE* out, KBDesc_p desc)
{
   fprintf(out,
      COMCHAR" E theorem prover knowledge base description\n"
      "Version     : \"%s\"\n"
      "NegProp     : %8f  "COMCHAR" "
      "Negative example proportion (successful proof search)\n"
      "FailExamples: %8ld  "COMCHAR" "
      "Number of clauses from a failed proof search\n",
      desc->version,
      desc->neg_proportion,
      desc->fail_neg_examples);
}

/*-----------------------------------------------------------------------
//
// Function: KBDescParse()
//
//   Parse a KB0Description.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

KBDesc_p KBDescParse(Scanner_p in)
{
   KBDesc_p handle = KBDescCellAlloc();

   AcceptInpId(in, "Version");
   AcceptInpTok(in, Colon);
   CheckInpTok(in, String);
   handle->version = DStrCopy(AktToken(in)->literal);

   if(strcmp(handle->version, KB_VERSION) > 0)
   {
      Error("Knowledge base is younger than your tool set. Please"
       " update from" E_URL, USAGE_ERROR);
   }
   NextToken(in);
   AcceptInpId(in, "NegProp");
   AcceptInpTok(in, Colon);
   handle->neg_proportion = ParseFloat(in);
   AcceptInpId(in, "FailExamples");
   AcceptInpTok(in, Colon);
   handle->fail_neg_examples = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: KBFileName()
//
//   Build a kb-file name in name and return a pointer to it.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

char* KBFileName(DStr_p name, char *basename, char* file)
{
   DStrReset(name);
   DStrAppendStr(name, basename);
   DStrAppendStr(name, "/");
   DStrAppendStr(name, file);

   return DStrView(name);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
