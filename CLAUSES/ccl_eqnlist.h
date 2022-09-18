/*-----------------------------------------------------------------------

  File  : ccl_eqnlist.h

  Author: Stephan Schulz

  Contents

  Functions for dealing with (singly linked) lists of equations as
  used in clauses.

  Copyright 1998, 1999, 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Apr 10 16:46:17 MET DST 1998

-----------------------------------------------------------------------*/

#ifndef CCL_EQNLIST

#define CCL_EQNLIST

#include <ccl_eqn.h>
#include <clb_objtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef bool (*TermPredicateFun_p)(Term_p);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define EQN_LIST_LONG_LIMIT 15

void    EqnListFree(Eqn_p list);
void    EqnListGCMarkTerms(Eqn_p list);

int     EqnListSetProp(Eqn_p list, EqnProperties prop);
int     EqnListDelProp(Eqn_p list, EqnProperties prop);
int     EqnListFlipProp(Eqn_p list, EqnProperties prop);
int     EqnListQueryPropNumber(Eqn_p list, EqnProperties prop);

bool    EqnListExistsTermExcept(Eqn_p list, Eqn_p except, TermPredicateFun_p predicate);
#define EqnListExistsTerm(list, predicate) EqnListExistsTermExcept(list, NULL, predicate)

void    EqnListMapTerms(Eqn_p list, TermMapper_p f, void* arg);

int      EqnListLength(Eqn_p list);
Eqn_p    EqnListFromArray(Eqn_p* array, int lenght);
PStack_p EqnListToStack(Eqn_p list);
Eqn_p    EqnListFromStack(PStack_p stack);

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
Eqn_p   EqnListCopyReplPlain(Eqn_p list, TB_p bank, Term_p old, Term_p repl);
Eqn_p   EqnListNegateEqns(Eqn_p list);
int     EqnListRemoveDuplicates(Eqn_p list);
int     EqnListRemoveResolved(EqnRef list);
int     EqnListRemoveACResolved(EqnRef list);
int     EqnListRemoveSimpleAnswers(EqnRef list);

Eqn_p   EqnListFindNegPureVarLit(Eqn_p list);

Eqn_p   EqnListFindTrue(Eqn_p list);
bool    EqnListIsTrivial(Eqn_p list);
bool    EqnLongListIsTrivial(Eqn_p list);
bool    EqnListIsACTrivial(Eqn_p list);
bool    EqnListIsGround(Eqn_p list);
bool    EqnListIsEquational(Eqn_p list);
bool    EqnListIsPureEquational(Eqn_p list);

int     EqnListOrient(OCB_p ocb, Eqn_p list);
int     EqnListMaximalLiterals(OCB_p ocb, Eqn_p list);
bool    EqnListEqnIsMaximal(OCB_p ocb, Eqn_p list, Eqn_p eqn);
bool    EqnListEqnIsStrictlyMaximal(OCB_p ocb, Eqn_p list, Eqn_p eqn);

void    EqnListPrint(FILE* out, Eqn_p list, char* sep,
                     bool negated,  bool fullterms);
void    EqnListPrintDeref(FILE* out, Eqn_p list, char* sep,
                          DerefType deref);
void    EqnListTSTPPrint(FILE* out, Eqn_p list, char* sep, bool fullterms);
Eqn_p   EqnListParse(Scanner_p in, TB_p bank, TokenType sep);

FunCode NormSubstEqnListExcept(Eqn_p list, Eqn_p except, Subst_p
                               subst, VarBank_p vars);
#define NormSubstEqnList(list, subst, vars)                     \
   NormSubstEqnListExcept((list), NULL, (subst), (vars))

long    EqnListDepth(Eqn_p list);

void    EqnListAddSymbolDistribution(Eqn_p list, long *dist_array);
void    EqnListAddTypeDistribution(Eqn_p list, long *type_array);
void    EqnListAddSymbolDistExist(Eqn_p list, long *dist_array, PStack_p exist);
void    EqnListAddSymbolFeatures(Eqn_p list, PStack_p mod_stack, long *feature_array);
void    EqnListComputeFunctionRanks(Eqn_p list, long *rank_array, long* count);
long    EqnListCollectVariables(Eqn_p list, PTree_p *tree);
long    EqnListCollectFCodes(Eqn_p list, NumTree_p *tree);
long    EqnListAddFunOccs(Eqn_p list, PDArray_p f_occur, PStack_p res_stack);

void    EqnListSignedTermSetProp(Eqn_p list, TermProperties props, bool pos, bool neg);
#define EqnListTermSetProp(list, props) \
        EqnListSignedTermSetProp((list),(props), true,true)

void    EqnListSignedTermDelProp(Eqn_p list, TermProperties props, bool pos, bool neg);
#define EqnListTermDelProp(list, props) \
        EqnListSignedTermDelProp((list),(props), true,true)

long    EqnListTBTermDelPropCount(Eqn_p list, TermProperties props);

long    EqnListCollectSubterms(Eqn_p list, PStack_p collector);
long    EqnListCollectGroundTerms(Eqn_p list, PTree_p *res, bool top_only,
                                  bool pos_lits, bool neg_lits);
void    EqnListLambdaNormalize(Eqn_p list);

bool    EqnListFindCompLitExcept(Eqn_p xs, Eqn_p exc, Eqn_p ys,
                                 DerefType d_x, DerefType d_y);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
