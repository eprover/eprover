/*-----------------------------------------------------------------------

File  : ccl_eqnlist.h

Author: Stephan Schulz

Contents

   Functions for dealing with (singly linked) lists of equations as
   used in clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Apr 10 16:46:17 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCL_EQNLIST

#define CCL_EQNLIST

#include <ccl_eqn.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


void    EqnListFree(Eqn_p list);
void    EqnListGCMarkTerms(Eqn_p list);

int     EqnListSetProp(Eqn_p list, EqnProperties prop);
int     EqnListDelProp(Eqn_p list, EqnProperties prop);
int     EqnListFlipProp(Eqn_p list, EqnProperties prop);
int     EqnListQueryPropNumber(Eqn_p list, EqnProperties prop);

int      EqnListLength(Eqn_p list);
Eqn_p    EqnListFromArray(Eqn_p* array, int lenght);
PStack_p EqnListToStack(Eqn_p list);

Eqn_p   EqnListExtractElement(EqnRef element);
#define EqnListExtractFirst(list)\
        EqnListExtractElement(list)
Eqn_p   EqnListExtractByProps(EqnRef list, EqnProperties props, bool
               negate);
void    EqnListDeleteElement(EqnRef element);
#define EqnListDeleteFirst(list)\
        EqnListDeleteElement(list)
void    EqnListInsertElement(EqnRef pos, Eqn_p element);
#define EqnListInsertFirst(list, element)\
        EqnListInsertElement((list), (element))
Eqn_p   EqnListAppend(EqnRef list, Eqn_p newpart);
Eqn_p   EqnListFlatCopy(Eqn_p list);
Eqn_p   EqnListCopy(Eqn_p list, TB_p bank);
Eqn_p   EqnListCopyExcept(Eqn_p list, Eqn_p except, TB_p bank);
Eqn_p   EqnListCopyOpt(Eqn_p list);
Eqn_p   EqnListCopyOptExcept(Eqn_p list, Eqn_p except);
Eqn_p   EqnListCopyDisjoint(Eqn_p list);
Eqn_p   EqnListCopyRepl(Eqn_p list, TB_p bank, Term_p old, Term_p repl);
Eqn_p   EqnListNegateEqns(Eqn_p list);
int     EqnListRemoveDuplicates(Eqn_p list);
int     EqnListRemoveResolved(EqnRef list);
int     EqnListRemoveACResolved(EqnRef list);
int     EqnListRemoveSimpleAnswers(EqnRef list);

Eqn_p   EqnListFindNegPureVarLit(Eqn_p list);

Eqn_p   EqnListFindTrue(Eqn_p list);
bool    EqnListIsTrivial(Eqn_p list);
bool    EqnListIsACTrivial(Eqn_p list);
bool    EqnListIsGround(Eqn_p list);
bool    EqnListIsEquational(Eqn_p list);
bool    EqnListIsPureEquational(Eqn_p list);

int     EqnListOrient(OCB_p ocb, Eqn_p list);
int     EqnListMaximalLiterals(OCB_p ocb, Eqn_p list);
bool    EqnListEqnIsMaximal(OCB_p ocb, Eqn_p list, Eqn_p eqn);
bool    EqnListEqnIsStrictlyMaximal(OCB_p ocb, Eqn_p list, Eqn_p eqn);
void    EqnListDeleteTermProperties(Eqn_p list, TermProperties props);

void    EqnListPrint(FILE* out, Eqn_p list, char* sep,
           bool negated,  bool fullterms);
void    EqnListTSTPPrint(FILE* out, Eqn_p list, char* sep, bool fullterms);
Eqn_p   EqnListParse(Scanner_p in, TB_p bank, TokenType sep);

FunCode NormSubstEqnListExcept(Eqn_p list, Eqn_p except, Subst_p
                subst, VarBank_p vars);
#define NormSubstEqnList(list, subst, vars) \
        NormSubstEqnListExcept((list), NULL, (subst), (vars))

long    EqnListDepth(Eqn_p list);

void    EqnListAddSymbolDistribution(Eqn_p list, long *dist_array);
void    EqnListAddSymbolDistExist(Eqn_p list, long *dist_array, PStack_p exist);
void    EqnListAddSymbolFeatures(Eqn_p list, PStack_p mod_stack, long *feature_array);
void    EqnListComputeFunctionRanks(Eqn_p list, long *rank_array, long* count);
long    EqnListCollectVariables(Eqn_p list, PTree_p *tree);
long    EqnListAddFunOccs(Eqn_p list, PDArray_p f_occur, PStack_p res_stack);

void    EqnListTermSetProp(Eqn_p list, TermProperties props);
long    EqnListTBTermDelPropCount(Eqn_p list, TermProperties props);
long    EqnListTermDelProp(Eqn_p list, TermProperties props);

long    EqnListCollectSubterms(Eqn_p list, PStack_p collector);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





