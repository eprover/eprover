/*-----------------------------------------------------------------------

File  : cpr_propclauses.c

Author: Stephan Schulz

Contents

  Basic functions for propositional clauses. 

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Apr 24 20:55:39 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "cpr_propclauses.h"



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
// Function: DPLLClauseFromClause()
//
//   Convert a propositional (not ground!) E clause into a DPLL
//   clause. No simplification or checking is done (except for the
//   fact that the clause is indeed propositional)!
//
// Global Variables: -
//
// Side Effects    : Memory allocation, psig changes.
//
/----------------------------------------------------------------------*/

DPLLClause_p DPLLClauseFromClause(PropSig_p psig, Clause_p clause)
{
   long         lit_no = ClauseLiteralNumber(clause);
   DPLLClause_p handle = DPLLClauseCellAlloc();
   Eqn_p        eqn;
   long         i;
   PAtomCode    atom;

   handle->lit_no    = lit_no;
   handle->active_no = lit_no;
   handle->literals  = SizeMalloc(sizeof(PAtomCode));
   
   for(i=0,eqn=clause->literals;
       eqn;
       i++,eqn=eqn->next)
   {
      assert(i<lit_no);
      if(!EqnIsPropositional(eqn))
      {
	 Error("Only real propositional clauses can be converted"
	       " by DPLLClauseFromClause()!",
	       SYNTAX_ERROR);
      }
      atom = PropSigInsertAtom(psig,
			       SigFindName(eqn->bank->sig,eqn->lterm->f_code));
      if(EqnIsNegative(eqn))
      {
	 atom = -atom;
      }
      handle->literals[i] = atom;
   }
   assert(i==lit_no);
   

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: DPLLClausePrintLOP()
//
//   Print a propositional clause in LOP format.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DPLLClausePrintLOP(FILE* out, PropSig_p psig, DPLLClause_p clause)
{
   char* sep ="";
   int i;

   for(i=0; i<clause->lit_no;i++)
   {
      if(clause->literals[i]>0)
      {
	 fprintf(out, "%s%s", sep, 
		 PropSigGetAtomName(psig,clause->literals[i]));
	 sep=";";
      }
   }
   fprintf(out,"<-");
   sep="";
   for(i=0; i<clause->lit_no;i++)
   {
      if(clause->literals[i]<0)
      {
	 fprintf(out, "%s%s", sep, 
		 PropSigGetAtomName(psig,-clause->literals[i]));
	 sep=",";
      }
   }      
   fprintf(out,".");
}

/*-----------------------------------------------------------------------
//
// Function: DPLLClausePrintDimacs()
//
//   Print a DPLL clause in DIMACS format (note that DIMACS input
//   files require a header, individual clauses are not a complete
//   syntactic element! Also not that most provers reading DIMACS
//   require the sequence "0\n" as an end of clause marker and that
//   most cannot deal with the empty clause. I'm printing the empty
//   clause nonetheless, since this output is unlikely to be parsed by
//   another system and I don't want to support broken behaviour.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void DPLLClausePrintDimacs(FILE* out,DPLLClause_p clause)
{
   int i;
   for(i=0; i<i<clause->lit_no;i++)
   {
      fprintf(out,"%ld ", clause->literals[i]);
   }
   fprintf(out, "0\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


