/*-----------------------------------------------------------------------

File  : cpr_dpllformula.c

Author: Stephan Schulz

Contents

  Implementation of code for a dpll formula data type.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri May  2 19:01:54 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "cpr_dpllformula.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: dpll_form_add_atom_space()
//
//   Create more space for atoms in an DPLLFormula.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void dpll_form_add_atom_space(DPLLFormula_p form)
{
   long old_limit, new_limit,i;

   old_limit = form->atom_no;
   if(old_limit)
   {
      new_limit=old_limit*ATOM_GROWTH_FACTOR;
   }
   else
   {
      new_limit = DEFAULT_ATOM_NUMBER;
   }

   form->atoms = SecureRealloc(form->atoms, new_limit*sizeof(AtomCell));
   for(i=old_limit; i<new_limit; i++)
   {
      form->atoms[i].pos_occur = 0;
      form->atoms[i].neg_occur = 0;
      form->atoms[i].pos_active= NULL;
      form->atoms[i].neg_active= NULL;
   }
   form->atom_no = new_limit;
}


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: DPLLFormulaAlloc()
//
//   Allocate an empty and initialized DPLLFormula().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

DPLLFormula_p DPLLFormulaAlloc(void)
{
   DPLLFormula_p form = DPLLFormulaCellAlloc();

   form->sig = PropSigAlloc();
   form->clauses = PStackAlloc();
   form->atom_no = 0;
   form->atoms = NULL;
   return form;
}


/*-----------------------------------------------------------------------
//
// Function: DPLLFormulaFree()
//
//   Free all of the DPLLFormula().
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void DPLLFormulaFree(DPLLFormula_p junk)
{
   int i;

   for(i=0; i<junk->atom_no; i++)
   {
      PTreeFree(junk->atoms[i].pos_active);
      PTreeFree(junk->atoms[i].neg_active);
   }
   FREE(junk->atoms);
   while(!PStackEmpty(junk->clauses))
   {
      DPLLClauseFree(PStackPopP(junk->clauses));
   }
   PStackFree(junk->clauses);
   PropSigFree(junk->sig);

   DPLLFormulaCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: DPLLFormulaPrint()
//
//   Print a DPLLFormula.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DPLLFormulaPrint(FILE* out,DPLLFormula_p form, DPLLOutputFormat format,
            bool print_atoms)
{
   long i;

   if(print_atoms)
   {
      PropSigPrint(out, form->sig);
      for(i=0; i<form->atom_no; i++)
      {
    if(form->atoms[i].pos_occur || form->atoms[i].neg_occur)
    {
       fprintf(out, COMCHAR" %4ld: %4ld %4ld\n", i,
          form->atoms[i].pos_occur,
          form->atoms[i].pos_occur);
    }
      }
   }
   for(i=0; i<PStackGetSP(form->clauses);i++)
   {
      switch(format)
      {
      case DPLLOutLOP:
    DPLLClausePrintLOP(out, form->sig,
             PStackElementP(form->clauses,i));
    fputc('\n',out);
    break;
      case DPLLOutDimacs:
    DPLLClausePrintDimacs(out, PStackElementP(form->clauses,i));
    break;
      default:
       assert(false && "Not a valid DPLLOutputFormat");
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: DPLLRegisterClauseLiteral()
//
//   Register a clause at an atom.
//
// Global Variables: -
//
// Side Effects    : Changes formula.
//
/----------------------------------------------------------------------*/

void DPLLRegisterClauseLiteral(DPLLFormula_p form,
                DPLLClause_p clause,
                PLiteralCode lit)
{
   PLiteralCode atom = ABS(lit);
   bool res;

   while(form->atom_no<=lit)
   {
      dpll_form_add_atom_space(form);
   }
   if(lit>0)
   {
      form->atoms[atom].pos_occur++;
      res = PTreeStore(&(form->atoms[atom].pos_active), clause);
      UNUSED(res); assert(res && "Duplicate entry of a clause!");
   }
   else
   {
      form->atoms[atom].neg_occur++;
      res = PTreeStore(&(form->atoms[atom].neg_active), clause);
      UNUSED(res); assert(res && "Duplicate entry of a clause!");
   }
}



/*-----------------------------------------------------------------------
//
// Function: DPLLFormulaInsertClause()
//
//   Insert a new clause into a formula. The clause is expected to be
//   non-tautological and contain no redundant literals. Moreover, for
//   the sake of printing LOP, the atoms should be registered in
//   form->sig.
//
// Global Variables: -
//
// Side Effects    : Changes formula.
//
/----------------------------------------------------------------------*/

void DPLLFormulaInsertClause(DPLLFormula_p form, DPLLClause_p clause)
{
   unsigned int i;

   PStackPushP(form->clauses, clause);
   for(i=0; i<clause->lit_no; i++)
   {
      DPLLRegisterClauseLiteral(form, clause, clause->literals[i]);
   }
}


/*-----------------------------------------------------------------------
//
// Function: DPLLFormulaParseLOP()
//
//   Parse a set of LOP clauses into a DPLLFormula.
//
// Global Variables: -
//
// Side Effects    : Changes form, I/O, memory operations
//
/----------------------------------------------------------------------*/

void DPLLFormulaParseLOP(Scanner_p in, Sig_p sig, DPLLFormula_p form)
{
   Clause_p clause;
   DPLLClause_p pclause;
   TB_p terms = TBAlloc(sig);

   while(ClauseStartsMaybe(in))
   {
      clause = ClauseParse(in, terms);
      pclause = DPLLClauseFromClause(form->sig, clause);
      ClauseFree(clause);
      fprintf(GlobalOut, "New clause: ");
      DPLLClausePrintLOP(GlobalOut, form->sig, pclause);
      if(DPLLClauseNormalize(pclause))
      {
    fprintf(GlobalOut, "...discarded (tautology)\n");
    DPLLClauseFree(pclause);
    continue;
      }
      DPLLFormulaInsertClause(form, pclause);
      fprintf(GlobalOut, "...accepted\n");
   }
   terms->sig = NULL;
   TBFree(terms);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
