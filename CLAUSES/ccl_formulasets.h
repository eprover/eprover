/*-----------------------------------------------------------------------

File  : ccl_formulaset.h

Author: Stephan Schulz

Contents

  Data type for (wrapped) formula sets.

Copyright 1998-2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jun 11 16:24:27 CEST 2009
    New (factored out from ccl_wrapped_formulas.h)

-----------------------------------------------------------------------*/

#ifndef CCL_FORMULASETS

#define CCL_FORMULASETS

#include <clb_plist.h>
#include <ccl_formula_wrapper.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct formula_set_cell
{
   WFormula_p anchor;
   long       members;
   DStr_p     identifier;
}FormulaSetCell, *FormulaSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define FormulaSetCellAlloc()    (FormulaSetCell*)SizeMalloc(sizeof(FormulaSetCell))
#define FormulaSetCellFree(junk) SizeFree(junk, sizeof(FormulaSetCell))

FormulaSet_p FormulaSetAlloc(void);
void         FormulaSetFreeFormulas(FormulaSet_p set);
void         FormulaSetFree(FormulaSet_p set);
#define      FormulaSetCardinality(set) ((set)->members)
#define      FormulaSetEmpty(set)\
             ((set)->anchor->succ == (set)->anchor)
long         FormulaSetStackCardinality(PStack_p stack);
void         FormulaSetGCMarkCells(FormulaSet_p set);
void         FormulaSetMarkPolarity(FormulaSet_p set);
void         FormulaSetInsert(FormulaSet_p set, WFormula_p newform);
long         FormulaSetInsertSet(FormulaSet_p set, FormulaSet_p from);
WFormula_p   FormulaSetExtractEntry(WFormula_p form);
WFormula_p   FormulaSetExtractFirst(FormulaSet_p set);
#define      FormulaSetMoveFormula(set, form) \
             FormulaSetExtractEntry(form);FormulaSetInsert((set), (form))

int          FormulaConjectureOrder(FormulaSet_p set);

void         FormulaSetDeleteEntry(WFormula_p form);
bool         FormulaSetIsUntyped(FormulaSet_p set);

void         FormulaSetPrint(FILE* out, FormulaSet_p set,
                             bool fullterms);
void         FormulaSetPrettyPrintTSTP(FILE* out, FormulaSet_p set, bool fullterms);
bool         FormulaSetHasInterpretedSymbol(FormulaSet_p set);
long         FormulaSetSplitConjectures(FormulaSet_p set,
                                        PList_p conjectures,
                                        PList_p rest);

long long    FormulaSetStandardWeight(FormulaSet_p set);
long         FormulaSetCountConjectures(FormulaSet_p set, long* hypos);
void         FormulaStackCondSetType(PStack_p stack, FormulaProperties type);
long         FormulaSetCollectFCode(FormulaSet_p set, FunCode f_code,
                                    PStack_p result);

void FormulaSetAppEncode(FILE* out, FormulaSet_p set);
void FormulaSetDefinitionStatistics(FormulaSet_p orig, FormulaSet_p arch,
                                    TB_p bank, int* num_defs,
                                    double* percentage_form_defs,
                                    int* num_lams, bool* app_var_lits);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
