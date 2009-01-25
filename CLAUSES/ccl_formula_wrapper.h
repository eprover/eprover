/*-----------------------------------------------------------------------

File  : ccl_formula_wrapper.h

Author: Stephan Schulz

Contents
 
  Data type wrapping formulas, with all the stuff that really only
  applies to input or top-level formulae, not to recursive
  subformulae. Also has formula sets (well, wrapped formula sets). 

  Copyright 1998-2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Nov 12 17:17:44 GMT 2003
    New

-----------------------------------------------------------------------*/

#ifndef CCL_FORMULA_WRAPPER

#define CCL_FORMULA_WRAPPER

#include <ccl_tformulae.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   WPIgnoreProps       = 0,        /* For masking properties out */
   WPInitial           = 1,        /* Input formula */
   WPInputFormula      = CPInputClause, /* _Really_ initial in TSTP sense */
   WPType1          = CPType1, /* 128 */
   WPType2          = CPType2, 
   WPType3          = CPType3,
   WPTypeMask       = CPTypeMask,
   WPTypeUnknown    = 0,                /* Also used as wildcard */
   WPTypeAxiom      = CPTypeAxiom,      /* Formula is Axiom */
   WPTypeHypothesis = CPTypeHypothesis, /* Formula is Hypothesis */
   WPTypeConjecture = CPTypeConjecture, /* Formula is Conjecture */
   WPTypeLemma      = CPTypeLemma,      /* Formula is Lemma */
   WPTypeNegConjecture = CPTypeNegConjecture /* Formula is NegConjecture */
}WFormulaProperties;


typedef struct wformula_cell
{ 
   WFormulaProperties    properties;
   long                  ident;
   TB_p                  terms;
   ClauseInfo_p          info;
   TFormula_p            tformula;
   struct formula_set_cell* set;      /* Is the formula in a set? */
   struct wformula_cell* pred;        /* For fomula sets = doubly  */
   struct wformula_cell* succ;        /* linked lists */
}WFormulaCell, *WFormula_p;


typedef struct formula_set_cell
{
   WFormula_p anchor;
   long       members;
}FormulaSetCell, *FormulaSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern long FormulaIdentCounter;
extern bool FormulaTermEncoding;
extern long FormulaDefLimit;

#define FormulaSetProp(form, prop) SetProp((form), (prop))
#define FormulaDelProp(form, prop) DelProp((form), (prop))
#define FormulaGiveProps(form, prop) GiveProps((form), (prop))

/* Are _all_ properties in prop set in form? */
#define FormulaQueryProp(form, prop) QueryProp((form), (prop))
/* Are any properties in prop set in form? */
#define FormulaIsAnyPropSet(form, prop) IsAnyPropSet((form), (prop))
#define FormulaSetType(form, type) \
    FormulaDelProp((form),WPTypeMask);\
    FormulaSetProp((form), (type))
#define FormulaQueryType(form) ((form)->properties&CPTypeMask)

#define WFormulaCellAlloc()    (WFormulaCell*)SizeMalloc(sizeof(WFormulaCell))
#define WFormulaCellFree(junk) SizeFree(junk, sizeof(WFormulaCell))

WFormula_p DefaultWFormulaAlloc();
WFormula_p WTFormulaAlloc(TB_p terms, TFormula_p formula);
void       WFormulaFree(WFormula_p form);
void       WFormulaGCMarkCells(WFormula_p form);
void       WFormulaMarkPolarity(WFormula_p form);

WFormula_p WFormulaTPTPParse(Scanner_p in, TB_p terms);
void       WFormulaTPTPPrint(FILE* out, WFormula_p form, bool fullterms);

WFormula_p WFormulaTSTPParse(Scanner_p in, TB_p terms);
void       WFormulaTSTPPrint(FILE* out, WFormula_p form, bool fullterms,
			     bool complete);
WFormula_p WFormulaParse(Scanner_p in, TB_p terms);
void       WFormulaPrint(FILE* out, WFormula_p form, bool fullterms);

#define FormulaSetCellAlloc()    (FormulaSetCell*)SizeMalloc(sizeof(FormulaSetCell))
#define FormulaSetCellFree(junk) SizeFree(junk, sizeof(FormulaSetCell))

FormulaSet_p FormulaSetAlloc();
void         FormulaSetFreeFormulas(FormulaSet_p set);
void         FormulaSetFree(FormulaSet_p set);
void         FormulaSetGCMarkCells(FormulaSet_p set);
void         FormulaSetMarkPolarity(FormulaSet_p set);
void         FormulaSetInsert(FormulaSet_p set, WFormula_p newform);
long         FormulaSetInsertSet(FormulaSet_p set, FormulaSet_p from);
WFormula_p   FormulaSetExtractEntry(WFormula_p form);
#define      FormulaSetEmpty(set)\
             ((set)->anchor->succ == (set)->anchor)
WFormula_p   FormulaSetExtractFirst(FormulaSet_p set);
void         FormulaSetDeleteEntry(WFormula_p form);

void         FormulaSetPrint(FILE* out, FormulaSet_p set, 
                             bool fullterms);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





