/*-----------------------------------------------------------------------

File  : cle_kbdesc.c

Author: Stephan Schulz

Contents
 
  Dealing with kb-descriptions.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul 27 16:07:14 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cle_kbdesc.h"



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
   handle->select_eval       = true;
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
	   "# E theorem prover knowledge base description\n"
	   "Version     : \"%s\"\n"
	   "NegProp     : %8f  # "
	   "Negative example proportion (successful proof search)\n"
	   "FailExamples: %8ld  # "
	   "Number of clauses from a failed proof search\n"
	   "Select on   : %s\n",
	   desc->version,
	   desc->neg_proportion,
	   desc->fail_neg_examples,
	   desc->select_eval? "evaluation":"change");
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
   AcceptInpId(in, "Select");
   AcceptInpId(in, "on");
   AcceptInpTok(in, Colon);
   CheckInpId(in, "evaluation|change");
   if(TestInpId(in, "evaluation"))
   {
      handle->select_eval = true;
   }
   else
   {
      handle->select_eval = false;
   }
   NextToken(in);

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


