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

#include <ccl_formulae.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   WPIgnoreProps    = 0,        /* For masking properties out */
   WPInitial        = 1,        /* Input formula */
   WPTypeAxiom      = 128,      /* Value for consistency with
                                 * ClauseProps, we also use it for as
                                 * a catch-all type for TPTP/TSTP
                                 * types not explicitely checked
                                 * for. */
   WPTypeHypothesis = 256,      
   WPTypeConjecture = 512,
   WPTypeMask       = WPTypeAxiom|WPTypeHypothesis|WPTypeConjecture
}WFormulaProperties;


typedef struct wformula_cell
{ 
   WFormulaProperties    properties;
   long                  ident;
   char                  *ext_ident;
   Formula_p             formula;
   struct formula_set_cell* set;         /* Is the formula in a set? */
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
WFormula_p WFormulaAlloc(Formula_p formula);
void       WFormulaFree(WFormula_p form);

WFormula_p WFormulaTPTPParse(Scanner_p in, TB_p terms);
void       WFormulaTPTPPrint(FILE* out, WFormula_p form, bool fullterms);

WFormula_p WFormulaTSTPParse(Scanner_p in, TB_p terms);
void       WFormulaTSTPPrint(FILE* out, WFormula_p form, bool fullterms,
			     bool complete);


#define FormulaSetCellAlloc()    (FormulaSetCell*)SizeMalloc(sizeof(FormulaSetCell))
#define FormulaSetCellFree(junk) SizeFree(junk, sizeof(FormulaSetCell))

FormulaSet_p FormulaSetAlloc();
void         FormulaSetFree(FormulaSet_p set);
void         FormulaSetInsert(FormulaSet_p set, WFormula_p new);
WFormula_p   FormulaSetExtractEntry(WFormula_p form);
#define      FormulaSetEmpty(set)\
             ((set)->anchor->succ == (set)->anchor)
WFormula_p   FormulaSetExtractFirst(FormulaSet_p set);
void         FormulaSetDeleteEntry(WFormula_p form);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





