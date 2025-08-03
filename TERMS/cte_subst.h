/*-----------------------------------------------------------------------

  File  : cte_subst.h

  Author: Stephan Schulz

  Contents

  Definitions for substitutions. Substitutions are really represented
  by term cells with bindings. The substitution type is only a
  disguised stack keeping track of the bound variables for
  backtracking.

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Mar  5 00:22:28 MET 1998

  -----------------------------------------------------------------------*/

#ifndef CTE_SUBST

#define CTE_SUBST

#include <clb_pstacks.h>
#include <clb_pqueue.h>
#include <cte_termbanks.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef PStackCell SubstCell;
typedef PStack_p   Subst_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SubstAlloc()    PStackAlloc()
#define SubstFree(junk) PStackFree(junk)

#define SubstDelete(junk) SubstBacktrack(junk);SubstFree(junk)
#define SubstDeleteSkolem(junk) SubstBacktrackSkolem(junk);SubstFree(junk)
#define SubstIsEmpty(subst) PStackEmpty(subst)

static inline PStackPointer SubstAddBinding(Subst_p subst, Term_p var, Term_p bind);
bool          SubstBacktrackSingle(Subst_p subst);
int           SubstBacktrackToPos(Subst_p subst, PStackPointer pos);
int           SubstBacktrack(Subst_p subst);

PStackPointer SubstNormTerm(Term_p term, Subst_p subst, VarBank_p vars, Sig_p sig);

bool          SubstBindingPrint(FILE* out, Term_p var, Sig_p sig, DerefType deref);
long          SubstPrint(FILE* out, Subst_p subst, Sig_p sig, DerefType deref);
bool          SubstIsRenaming(Subst_p subt);
bool          SubstHasHOBinding(Subst_p subt);

PStackPointer SubstBindAppVar(Subst_p subst, Term_p var,
                              Term_p term, int up_to,
                              TB_p bank);

void          SubstBacktrackSkolem(Subst_p subst);
void          SubstSkolemizeTerm(Term_p term, Subst_p subst, Sig_p sig);
void          SubstCompleteInstance(Subst_p subst, Term_p term,
                                    Term_p default_binding);


/*-----------------------------------------------------------------------
//
// Function: SubstAddBinding()
//
//   Perform a new binding and store it in the subst. Return the old
//   stackpointer (i.e. the value that you'll have to backtrack to to
//   get rid of this binding).
//
//
// Global Variables: -
//
// Side Effects    : Changes bindings, adds to the substitution.
//
/----------------------------------------------------------------------*/

PStackPointer SubstAddBinding(Subst_p subst, Term_p var, Term_p bind)
{
   PStackPointer ret = PStackGetSP(subst);

   assert(subst);
   assert(var);
   assert(bind);
   assert(TermIsFreeVar(var));
   assert(!(var->binding));
   //assert(problemType == PROBLEM_HO || !TermCellQueryProp(bind, TPPredPos)
   //      || bind->f_code == SIG_TRUE_CODE || bind->f_code == SIG_FALSE_CODE); // Skolem symbols also
   assert(var->type);
   assert(bind->type);
   assert(var->type == bind->type);

   /* printf(COMCHAR" %ld <- %ld \n", var->f_code, bind->f_code); */
   var->binding = bind;
   PStackPushP(subst, var);

   return ret;
}

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
