/*-----------------------------------------------------------------------

File  : cte_termfunc.h

Author: Stephan Schulz

Contents
 
  Most of the user-level functionality for unshared terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Feb 25 16:50:36 MET 1998
    Ripped from cte_terms.h (should be obsolete by now)

-----------------------------------------------------------------------*/

#ifndef CTE_TERMFUNC

#define CTE_TERMFUNC

#include <clb_numtrees.h>
#include <cte_termvars.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern bool      TermPrintLists; /* Using [...] notation */

#define TermStartToken (SigSupportLists?\
                       (FuncSymbStartToken|OpenSquare|Mult):\
		       (FuncSymbStartToken|Mult))

void   VarPrint(FILE* out, FunCode var);
void   TermPrint(FILE* out, Term_p term, Sig_p sig, DerefType deref);
void   TermPrintArgList(FILE* out, Term_p *args, int arity, Sig_p sig,
			DerefType deref);
FuncSymbType TermParseOperator(Scanner_p in, DStr_p id);
FunCode       TermSigInsert(Sig_p sig, const char* name, int arity, bool
                            special_id, FuncSymbType type);
Term_p TermParse(Scanner_p in, Sig_p sig, VarBank_p vars);
int    TermParseArgList(Scanner_p in, Term_p** arg_anchor, Sig_p sig,
                         VarBank_p vars);
Term_p TermCopy(Term_p source, VarBank_p vars, DerefType deref);
Term_p TermCopyKeepVars(Term_p source, DerefType deref);
Term_p TermEquivCellAlloc(Term_p source, VarBank_p vars);

bool   TermStructEqual(Term_p t1, Term_p t2);
bool   TermStructEqualNoDeref(Term_p t1, Term_p t2);
bool   TermStructEqualNoDerefHardVars(Term_p t1, Term_p t2);

bool   TermStructEqualDeref(Term_p t1, Term_p t2, DerefType deref_1,
	  		    DerefType deref_2);
bool   TermStructEqualDerefHardVars(Term_p t1, Term_p t2, DerefType deref_1,
                                    DerefType deref_2);

int    TermStructWeightCompare(Term_p t1, Term_p t2);

int    TermLexCompare(Term_p t1, Term_p t2);

bool   TermIsSubterm(Term_p super, Term_p test, DerefType deref,
                      TermEqualTestFun EqualTest);

bool    TermIsSubtermDeref(Term_p super, Term_p test, DerefType
			   deref_super, DerefType deref_test);

#define TermIsStructSubterm(super, term) \
        TermIsSubterm((super),(term),DEREF_ALWAYS,TermStructEqual)

long    TermWeight(Term_p term, long vweight, long fweight);
#define TermStandardWeight(term) \
        (TermIsShared(term)? \
	 (term)->weight:\
	 TermWeight((term),DEFAULT_VWEIGHT,DEFAULT_FWEIGHT))

long    TermFsumWeight(Term_p term, long vweight, long flimit, 
                       long *fweights, long default_fweight);

long    TermNonLinearWeight(Term_p term, long vlweight, long vweight,
			    long fweight);
long    TermSymTypeWeight(Term_p term, long vweight, long fweight,
			  long cweight, long pweight);
long    TermDepth(Term_p term);

bool    TermIsDefTerm(Term_p term, int min_arity);

bool    TermHasFCode(Term_p term, FunCode f);

bool    TermHasVariables(Term_p term, bool unbound_only);
#define TermIsGround(term) (!TermHasVariables((term), false))
#define TermHasUnboundVariables(term) TermHasVariables((term), true)

FunCode TermFindMaxVarCode(Term_p term);

long    VarBankCheckBindings(FILE* out, VarBank_p bank, Sig_p sig);

#define TermAddSymbolDistribution(term, dist_array)\
        TermAddSymbolDistributionLimited((term),(dist_array), LONG_MAX)
void    TermAddSymbolDistributionLimited(Term_p term, long *dist_array, 
					 long limit);
void    TermAddSymbolDistExist(Term_p term, long *dist_array, 
                               PStack_p exists);
void    TermAddSymbolFeaturesLimited(Term_p term, long depth,
				     long *freq_array, long* depth_array,
				     long limit);

void    TermAddSymbolFeatures(Term_p term, PStack_p mod_stack, 
                              long depth, long *feature_array, long offset);

void    TermComputeFunctionRanks(Term_p term, long *rank_array, long *count);
long    TermCollectPropVariables(Term_p term, PTree_p *tree,
				  TermProperties prop);
#define TermCollectVariables(term,tree)\
        TermCollectPropVariables((term), (tree), TPIgnoreProps)
long    TermAddFunOcc(Term_p term, PDArray_p f_occur, PStack_p res_stack);

long    TermLinearize(PStack_p stack, Term_p term);


Term_p  TermCheckConsistency(Term_p term, DerefType deref);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





