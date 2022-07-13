/*-----------------------------------------------------------------------

File  : cte_match_mgu_1-1.h

Author: Stephan Schulz

Contents

  Interface to simple, non-indexed 1-1 match and unification
  routines on shared terms (and unshared terms with shared
  variables).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 11 16:17:33 MET 1998
    New

-----------------------------------------------------------------------*/

#ifndef CTE_MATCH_MGU_1_1

#define CTE_MATCH_MGU_1_1

#include <clb_os_wrapper.h>
#include <cte_subst.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/
typedef enum which_term {
   NoTerm    = 0,
   LeftTerm  = 1,
   RightTerm = 2
} UnifTermSide;

typedef enum oracle_unif_result {
    UNIFIABLE,
    NOT_UNIFIABLE,
    NOT_IN_FRAGMENT,
} OracleUnifResult;

typedef bool UnificationResult;

extern const UnificationResult UNIF_FAILED;
extern const UnificationResult UNIF_SUCC;

#define UnifFailed(u_res) (!u_res)

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#ifdef MEASURE_UNIFICATION
extern long UnifAttempts;
extern long UnifSuccesses;
#endif

PERF_CTR_DECL(MguTimer);

#define MATCH_FAILED -1

bool OccurCheck(restrict Term_p term, restrict Term_p var);

// FO matching and unification
bool SubstComputeMatch(Term_p matcher, Term_p to_match, Subst_p subst);
bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst);

// HO matching and unification
int  PartiallyMatchVar(Term_p var_matcher, Term_p to_match, Sig_p sig, bool perform_OccursCheck);
int  SubstComputeMatchHO(Term_p matcher, Term_p to_match, Subst_p subst);
UnificationResult SubstComputeMguHO(Term_p t1, Term_p t2, Subst_p subst);


#ifdef ENABLE_LFHO

// If we're working in HOL mode, we choose run FO/HO unification/matching
// based on the problem type.
bool SubstMatchComplete(Term_p t, Term_p s, Subst_p subst);
bool SubstMguComplete(Term_p t, Term_p s, Subst_p subst);

#else

// If we are working in FOL mode, we revert to normal E behavior.
#define SubstMatchComplete(t, s, subst) (SubstComputeMatch(t, s, subst))
#define SubstMguComplete(t, s, subst)   (SubstComputeMgu(t, s, subst))

#endif

#define VerifyMatch(matcher, to_match) \
        TermStructEqualDeref((matcher), (to_match), \
              DEREF_ONCE, DEREF_NEVER)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





