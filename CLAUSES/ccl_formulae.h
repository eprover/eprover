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

/* Operand for FOF formulae. */

typedef enum 
{
   OpNoOp   = 0,
   OpIsLit  = OpNoOp,
   OpUNot,
   OpQEx,
   OpQAll,
   OpBAnd,
   OpBOr,
   OpBImpl,
   OpBEquiv,
   OpBNand,
   OpBNor,
   OpBNImpl,
   OpBXor,
}FOFOperatorType;


/* We are going with opportunistic sharing here, i.e. we do not look
   for identical subformula, but whenever we need an uninstantiated
   copy, we just hand out a new reference. */

typedef struct formula_cell
{
   FOFOperatorType op;
   union /* Bound variable for quantifier, pointer to real literal for
            leaf nodes in the formula tree. I could fold this into
            one of the args, but it saves only one word, and I don't
            expect to need too many formula cells anyways. */
   {
      Term_p           var;
      Eqn_p            literal;
   }special;
   struct formula_cell *arg1;
   struct formula_cell *arg2;
   long                ref_count;
}FormulaCell, *Formula_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define OpIsUnary(op)   ((op)==OpUNot)
#define OpIsBinary(op)  ((op)>= OpBAnd)
#define OpIsQuantor(op) (((op)==OpQEx) || ((op)==OpQAll))

#define FormulaCellAlloc()    (FormulaCell*)SizeMalloc(sizeof(FormulaCell))
#define FormulaCellFree(junk) SizeFree(junk, sizeof(FormulaCell))
#define FormulaGetRef(form)   ((form)?(form)->ref_count++,(form):NULL)
#define FormulaRelRef(form)   ((form)?(form)->ref_count--,(form):NULL)


#ifdef CONSTANT_MEM_ESTIMATE
#define FORMULACELL_MEM 16
#else
#define FORMULACELL_MEM MEMSIZE(FormulaCell)
#endif

#define   FormulaHasSubForm1(form)  ((form)->op!=OpNoOp)
#define   FormulaHasSubForm2(form)  OpIsBinary((form)->op)
#define   FormulaIsBinary(form)     OpIsBinary((form)->op)
#define   FormulaIsUnary(form)      OpIsUnary((form)->op)
#define   FormulaIsQuantified(form) OpIsQuantor((form)->op)
#define   FormulaIsLiteral(form)    ((form)->op==OpNoOp)
#define   FormulaIsPropConst(form, positive) \
          FormulaIsLiteral(form) &&\
          ((positive)?EqnIsPropTrue((form)->special.literal)\
                     :EqnIsPropFalse((form)->special.literal))

#define   FormulaIsPropTrue(form)   (FormulaIsLiteral(form) && \
                                    EqnIsPropTrue((form)->special.literal))
#define   FormulaIsPropFalse(form)  (FormulaIsLiteral(form) && \
                                    EqnIsPropFalse((form)->special.literal))
                                     
Formula_p FormulaAlloc(void);
void      FormulaFree(Formula_p form);
Formula_p FormulaOpAlloc(FOFOperatorType op, Formula_p arg1, Formula_p arg2);
Formula_p FormulaLitAlloc(Eqn_p literal);
Formula_p FormulaPropConstantAlloc(TB_p terms, bool positive);
Formula_p FormulaQuantorAlloc(FOFOperatorType quantor, Term_p var,Formula_p arg1);
void      FormulaTPTPPrint(FILE* out, Formula_p form, bool fullterms);
Formula_p FormulaTPTPParse(Scanner_p in, TB_p terms);

bool      FormulaEqual(Formula_p form1, Formula_p form2);
bool      FormulaVarIsFree(Formula_p form, Term_p var);
Formula_p FormulaCopy(Formula_p form, TB_p terms);
void      FormulaCollectFreeVars(Formula_p form, PTree_p *vars);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





