/*-----------------------------------------------------------------------

  File  : cle_patterns.h

  Author: Stephan Schulz

  Contents

  Data type (previous "norm subst") for describing terms, equations
  and clauses as patterns of same.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Apr  9 14:18:11 MET DST 1999
  New

  -----------------------------------------------------------------------*/

#ifndef CLE_PATTERNS

#define CLE_PATTERNS

#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Norm idents are allocated as NORM_SYMBOL_LIMIT*(arity+1) +number of
   ident. There can be no more than NORM_SYMBOL_LIMIT symbols of any
   arity, and no symbol with arity > ARITY. Moreover, we will run into
   trouble with learning if the total number of symbols is ever
   bigger than 32768
   NORM_ARITY_LIMIT*NORM_SYMBOL_LIMIT <= LONG_MAX */

#define NORM_ARITY_LIMIT  (16384/8)  /* Largest Arity allowed */
#define NORM_SYMBOL_LIMIT (65536*8)  /* Maximum number of different
                                        symbols of an arity allowed */
#define NORM_VAR_INIT     -536870912

typedef struct patternsubstcell
{
   PDArray_p used_idents; /* Idents already used for different arities */
   PDArray_p fun_subst;   /* Replacement for function symbols. 0 ->
                             unassigned. Pre-assigned symbols are
                             given values from
                             1 to NORM_SYMBOL_LIMIT */
   long      used_vars;
   PDArray_p var_subst;   /* Replacement for variables */
   PStack_p  backtrack;   /* To take back bindings */
   Sig_p     sig;         /* For alpha-ranks and potentially other
                             stuff  */
}PatternSubstCell, *PatternSubst_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DEFAULT_LITERAL_NO 8
#define PATTERN_SEARCH_BRANCHLIMIT 3

#define PatternNormCode(symbol, arity) ((NORM_SYMBOL_LIMIT*((arity)+1))+symbol)

#define PatternSubstCellAlloc()                                 \
   (PatternSubstCell*)SizeMalloc(sizeof(PatternSubstCell))
#define PatternSubstCellFree(junk)                      \
   SizeFree(junk, sizeof(PatternSubstCell))

PatternSubst_p PatternSubstAlloc(Sig_p sig);
PatternSubst_p PatternDefaultSubstAlloc(Sig_p sig);
void           PatternSubstFree(PatternSubst_p junk);
bool           PatSubstExtend(PatternSubst_p subst, FunCode symbol,
                              int arity);
PatternSubst_p PatternSubstCopy(PatternSubst_p subst);

#define PatternIdGetArity(ident) (((ident)/NORM_SYMBOL_LIMIT)-1)
#define PatternIdGetIdent(ident) ((ident)%NORM_SYMBOL_LIMIT)

#define PatIdIsNormId(symbol) ((symbol) >= NORM_SYMBOL_LIMIT)

bool    PatSymbolIsBound(PatternSubst_p subst, FunCode f_code);
FunCode PatSymbValue(PatternSubst_p subst, FunCode f_code);

#define PatEqnLTerm(eqn, dir) ((dir)==PENormal?(eqn)->lterm:(eqn)->rterm)
#define PatEqnRTerm(eqn, dir) ((dir)==PENormal?(eqn)->rterm:(eqn)->lterm)

bool          PatternTermCompute(PatternSubst_p subst, Term_p term);
CompareResult PatternTermCompare(PatternSubst_p subst1, Term_p t1,
                                 PatternSubst_p subst2, Term_p t2);
bool          PatternTermPairCompute(PatternSubst_p subst, Eqn_p eqn,
                                     PatEqnDirection direction);
CompareResult PatternTermPairCompare(PatternSubst_p subst1, Eqn_p
                                     eqn1, PatEqnDirection dir1, PatternSubst_p
                                     subst2,Eqn_p eqn2, PatEqnDirection dir2);
bool          PatternLitListCompute(PatternSubst_p subst, PStack_p listrep);
CompareResult PatternLitListCompare(PatternSubst_p subst1, PStack_p
                                    listrep1, PatternSubst_p subst2,
                                    PStack_p listrep2);

long PatternClauseCompute(Clause_p clause, PatternSubst_p* subst,
                          PStack_p *listrep);

bool           PatternSubstBacktrack(PatternSubst_p subst,
                                     PStackPointer old_state);

void           PatternTermPrint(FILE* out, PatternSubst_p subst,
                                Term_p term, Sig_p sig);
void           PatternEqnPrint(FILE* out, PatternSubst_p subst, Eqn_p
                               eqn, PatEqnDirection direction);
void           PatternClausePrint(FILE* out, PatternSubst_p subst,
                                  PStack_p listrep);

PStack_p       DebugPatternClauseToStack(Clause_p clause);

Term_p         PatternTranslateSig(Term_p term, PatternSubst_p subst,
                                   Sig_p old_sig, Sig_p new_sig,
                                   VarBank_p new_vars);

FunCode        PatternSubstGetOriginalSymbol(PatternSubst_p subst,
                                             FunCode f);

/* void           PatternSubstDebugPrint(FILE* out, PatternSubst_p
   subst); */
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
