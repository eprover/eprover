/*-----------------------------------------------------------------------

File  : ccl_formulae.h

Author: Stephan Schulz

Contents
 
  Declarations and definitions for full first-order formulae.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Nov  5 09:37:19 GMT 2003
    New

-----------------------------------------------------------------------*/

#ifndef CCL_FORMULAE

#define CCL_FORMULAE

#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Operand for FOF formulae. There is some reason for this scheme:
   Quantors have bit 1 set, binary operators have bit 2 set (counting
   starts at 0! */

typedef enum 
{
   OpNoOp   = 0,
   OpIsLit  = OpNoOp,
   OpUNot   = 1,
   OpQEx    = 2,
   OpQAll   = 3,
   OpBAnd   = 4,
   OpBOr    = 5,
   OpBImpl  = 6,
   OpBEquiv = 7,
   OpBNand  = 12,
   OpBNor   = 13,
   OpBNImpl = 14,
   OpBXor  =  15,
}FOFOperatorType;


typedef struct formula_cell
{
   FOFOperatorType op;
   union /* Bound variable for quantifier, pointer to true literal for
            leaf nodes in the formula tree. I could fold this into
            one of the args, but it saves only one word, and I don't
            expect to need to many formula cells anyways. */
   {
      Term_p           var;
      Eqn_p            literal;
   }special;
   struct formula_cell *arg1;
   struct formula_cell *arg2;
   short               polarity;
   long                ref_count;
}FormulaCell, *Formula_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define OpIsUnary(op)   ((op)==OpUNot)
#define OpIsBinary(op)  ((op) & 4)
#define OpIsQuantor(op) ((op) &2)

#define FormulaCellAlloc()    (FormulaCell*)SizeMalloc(sizeof(FormulaCell))
#define FormulaCellFree(junk) SizeFree(junk, sizeof(FormulaCell))
#define FormulaGetRef(form) ((form)?(form)->ref_count++,(form):NULL)
#define FormulaRelRef(form) ((form)?:(form)->ref_count--,NULL:NULL)


#ifdef CONSTANT_MEM_ESTIMATE
#define FORMULACELL_MEM 16
#else
#define FORMULACELL_MEM MEMSIZE(FormulaCell)
#endif

#define   FormulaHasSubForm1(form) ((form)->op!=OpNoOp)
#define   FormulaHasSubForm2(form) OpIsBinary((form)->op)
#define   FormulaIsBinary(form)       OpIsBinary((form)->op)
#define   FormulaIsUnary(form)        OpIsUnary((form)->op)
#define   FormulaIsQuantified(form)      OpIsQuantor((form)->op)
#define   FormulaIsLiteral(form)   ((form)->op==OpNoOp)


void      FormulaFree(Formula_p form);
Formula_p FormulaOpAlloc(FOFOperatorType op, Formula_p arg1, Formula_p arg2);
Formula_p FormulaLitAlloc(Eqn_p literal);
Formula_p FormulaQuantorAlloc(FOFOperatorType quantor, Term_p var,Formula_p arg1);
void      FormulaTPTPPrint(FILE* out, Formula_p form, bool fullterms);
Formula_p FormulaTPTPParse(Scanner_p in, TB_p terms);

Formula_p FormulaCopy(Formula_p formula, TB_p bank);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





