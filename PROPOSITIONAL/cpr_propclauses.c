/*-----------------------------------------------------------------------

File  : cpr_propclauses.c

Author: Stephan Schulz

Contents

  Basic functions for propositional clauses.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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

/*-----------------------------------------------------------------------
//
// Function: p_atom_compare()
//
//   Compare the two propositional atoms pointed to. Atoms are
//   compared by alsolute value, then by sign (smaller is smaller,
//   positive is smaller).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int p_atom_compare(const void* atom1, const void* atom2)
{
   const PLiteralCode *a1 = (const PLiteralCode*)atom1;
   const PLiteralCode *a2 = (const PLiteralCode*)atom2;

   PLiteralCode abs_a1, abs_a2;

   if(a1==a2)
   {
      return 0;
   }
   abs_a1 = ABS(*a1);
   abs_a2 = ABS(*a1);
   if(abs_a1 < abs_a2)
   {
      return -1;
   }
   if(abs_a1 > abs_a2)
   {
      return 1;
   }
   if(*a1>0)
   {
      return -1;
   }
   return 1;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: DPLLClauseFree()
//
//   Free a DPLLClause.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void DPLLClauseFree(DPLLClause_p junk)
{
   SizeFree(junk->literals, junk->mem_size);
   DPLLClauseCellFree(junk);
}

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
   PLiteralCode    atom;

   handle->lit_no    = lit_no;
   handle->active_no = lit_no;
   handle->mem_size  = lit_no*sizeof(PLiteralCode);
   handle->literals  = SizeMalloc(handle->mem_size);

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
// Function: DPLLClauseNormalize()
//
//   Destructively normalize a clause: Literals are sorted by atom
//   encoding (positive comes before negative if both exist). Doubly
//   occuring literals are removed. Return value is true if clause is
//   tautological, false otherwise. Does not reduce size of literal
//   array, as I don't expect much reduction in the number of atoms
//   here.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool DPLLClauseNormalize(DPLLClause_p clause)
{
   unsigned long to,from;
   bool tautology = false;

   if(clause->lit_no <= 1)
   {
      return false;
   }
   assert(clause->lit_no == clause->active_no);
   qsort(clause->literals, clause->lit_no, sizeof(PLiteralCode),
    p_atom_compare);
   to = 0;
   from = 1;
   while(from<clause->lit_no)
   {
      if(clause->literals[from] != clause->literals[to])
      {
    if(clause->literals[from] == -(clause->literals[to]))
    {
       tautology=true;
    }
    to++;
    clause->literals[to]=clause->literals[from];
      }
      from++;
   }
   clause->lit_no = clause->active_no = to+1;
   return tautology;
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
   unsigned int i;

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
   unsigned int i;
   for(i=0; i<clause->lit_no;i++)
   {
      fprintf(out,"%ld ", clause->literals[i]);
   }
   fprintf(out, "0\n");
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


