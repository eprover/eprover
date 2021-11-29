/*-----------------------------------------------------------------------

  File  : ccl_clausesets.h

  Author: Stephan Schulz

  Contents

  Definitions dealing with collections of clauses

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  5 02:28:25 MET DST 1997

-----------------------------------------------------------------------*/

#ifndef CCL_CLAUSESETS

#define CCL_CLAUSESETS

#include <ccl_inferencedoc.h>
#include <ccl_derivation.h>
#include <ccl_fcvindexing.h>
#include <ccl_tautologies.h>
#include <ccl_pdtrees.h>
#include <clb_plist.h>
#include <clb_objtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Clause sets are doubly linked lists of clauses with indices for the
   various potential evaluations. */

typedef struct clausesetcell
{
   long      members; /* How many clauses are there? */
   long      literals; /* And how many literals? */
   Clause_p  anchor;  /* The clauses */
   SysDate   date;    /* Age of the clause set, used for optimizing
          rewriting. The special date SysCreationDate()
          is used to indicate ignoring of dates when
          checking for irreducability. */
   PDTree_p  demod_index; /* If used for demodulators */
   FVIAnchor_p fvindex; /* Used for non-unit subsumption */
   PDArray_p eval_indices;
   long      eval_no;
   DStr_p     identifier;
}ClauseSetCell, *ClauseSet_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define CLAUSECELL_DYN_MEM (CLAUSECELL_MEM+3*PTREE_CELL_MEM)

#define ClauseSetCellAlloc()    (ClauseSetCell*)SizeMalloc(sizeof(ClauseSetCell))
#define ClauseSetCellFree(junk) SizeFree(junk, sizeof(ClauseSetCell))

#define     ClauseSetStorage(set)\
            (((CLAUSECELL_DYN_MEM+EVAL_MEM((set)->eval_no))*(set)->members+\
            EQN_CELL_MEM*(set)->literals)+\
            PDTreeStorage(set->demod_index)+\
       FVIndexStorage(set->fvindex))

ClauseSet_p ClauseSetAlloc(void);
void        ClauseSetFreeClauses(ClauseSet_p set);
#define     ClauseSetCardinality(set) ((set)->members)
#define     ClauseSetEmpty(set)\
            ((set)->anchor->succ == (set)->anchor)
long        ClauseSetStackCardinality(PStack_p stack);
void        ClauseSetFree(ClauseSet_p junk);
void        ClauseSetGCMarkTerms(ClauseSet_p set);
void        ClauseSetInsert(ClauseSet_p set, Clause_p newclause);
long        ClauseSetInsertSet(ClauseSet_p set, ClauseSet_p from);
void        ClauseSetPDTIndexedInsert(ClauseSet_p set, Clause_p newclause);
void        ClauseSetIndexedInsert(ClauseSet_p set, FVPackedClause_p newclause);
void        ClauseSetIndexedInsertClause(ClauseSet_p set, Clause_p newclause);
void        ClauseSetIndexedInsertClauseSet(ClauseSet_p set, ClauseSet_p source);
Clause_p    ClauseSetExtractEntry(Clause_p clause);
#define     ClauseSetMoveClause(set, clause) \
            ClauseSetExtractEntry(clause);ClauseSetInsert((set), (clause))
Clause_p    ClauseSetExtractFirst(ClauseSet_p set);
void        ClauseSetDeleteEntry(Clause_p clause);
Clause_p    ClauseSetFindBest(ClauseSet_p set, int idx);
void        ClauseSetPrint(FILE* out, ClauseSet_p set, bool
            fullterms);
void        ClauseSetTSTPPrint(FILE* out, ClauseSet_p set, bool fullterms);
void        ClauseSetPrintPrefix(FILE* out, char* prefix, ClauseSet_p set);
void        ClauseSetSort(ClauseSet_p set, ComparisonFunctionType cmp_fun);

void        ClauseSetSetProp(ClauseSet_p set, FormulaProperties prop);
void        ClauseSetDelProp(ClauseSet_p set, FormulaProperties prop);
void        ClauseSetSetTPTPType(ClauseSet_p set, FormulaProperties type);

long        ClauseSetMarkCopies(ClauseSet_p set);
long        ClauseSetDeleteMarkedEntries(ClauseSet_p set);
long        ClauseSetDeleteCopies(ClauseSet_p set);
long        ClauseSetDeleteNonUnits(ClauseSet_p set);

long        ClauseSetGetTermNodes(ClauseSet_p set);
long        ClauseSetMarkSOS(ClauseSet_p set, bool tptp_types);

void        ClauseSetTermSetProp(ClauseSet_p set, TermProperties prop);
long        ClauseSetTBTermPropDelCount(ClauseSet_p set, TermProperties prop);
long        ClauseSetGetSharedTermNodes(ClauseSet_p set);

long        ClauseSetParseList(Scanner_p in, ClauseSet_p set, TB_p bank);
void        ClauseSetMarkMaximalTerms(OCB_p ocb, ClauseSet_p set);
void        ClauseSetSortLiterals(ClauseSet_p set, ComparisonFunctionType cmp_fun);

SysDate     ClauseSetListGetMaxDate(ClauseSet_p *demodulators, int
                limit);
Clause_p    ClauseSetFind(ClauseSet_p set, Clause_p clause);
Clause_p    ClauseSetFindById(ClauseSet_p set, long ident);
void        ClauseSetRemoveEvaluations(ClauseSet_p set);
long        ClauseSetFilterTrivial(ClauseSet_p set);
long        ClauseSetFilterTautologies(ClauseSet_p set, TB_p work_bank);

Clause_p    ClauseSetFindMaxStandardWeight(ClauseSet_p set);

ClausePos_p ClauseSetFindEqDefinition(ClauseSet_p set, int min_arity,                                       Clause_p start);

void        ClauseSetDocInital(FILE* out, long level, ClauseSet_p set);
void        ClauseSetDocQuote(FILE* out, long level, ClauseSet_p set,
               char* comment);
void        ClauseSetPropDocQuote(FILE* out, long level,
                                  FormulaProperties prop,
                                  ClauseSet_p set, char* comment);
#define     ClauseSetDocQuote(out, level, set, comment)                 \
   ClauseSetPropDocQuote((out), (level),CPIgnoreProps, (set), (comment))
#ifndef NDBUG

bool        ClauseSetVerifyDemod(ClauseSet_p demods, ClausePos_p pos);

bool        PDTreeVerifyIndex(PDTree_p root, ClauseSet_p demods);

#endif

void        EqAxiomsPrint(FILE* out, Sig_p sig, bool single_subst);


void        ClauseSetAddSymbolDistribution(ClauseSet_p set, long
                                           *dist_array);
void        ClauseSetAddTypeDistribution(ClauseSet_p set, long *type_array);
void        ClauseSetAddConjSymbolDistribution(ClauseSet_p set,
                                               long *dist_array);
void        ClauseSetAddAxiomSymbolDistribution(ClauseSet_p set,
                                                long *dist_array);

void        ClauseSetComputeFunctionRanks(ClauseSet_p set, long
                 *rank_array, long* count);

FunCode     ClauseSetFindFreqSymbol(ClauseSet_p set, Sig_p sig, int
                arity, bool least);
long        ClauseSetMaxVarNumber(ClauseSet_p set);

long        ClauseSetFindCharFreqVectors(ClauseSet_p set,
                FreqVector_p fsum,
                FreqVector_p fmax,
                FreqVector_p fmin,
                FVCollect_p cspec);

PermVector_p PermVectorCompute(ClauseSet_p set, FVCollect_p cspec,
                               bool eliminate_uninformative);

long         ClauseSetFVIndexify(ClauseSet_p set);
long         ClauseSetNewTerms(ClauseSet_p set, TB_p terms);

long         ClauseSetSplitConjectures(ClauseSet_p set,
                                       PList_p conjectures,
                                       PList_p rest);
long long    ClauseSetStandardWeight(ClauseSet_p set);


void         ClauseSetDerivationStackStatistics(ClauseSet_p set);

long         ClauseSetPushClauses(PStack_p stack, ClauseSet_p set);

void         ClauseSetDefaultWeighClauses(ClauseSet_p set);

long         ClauseSetCountConjectures(ClauseSet_p set, long* hypos);
int          ClauseConjectureOrder(ClauseSet_p set);

bool         ClauseSetIsUntyped(ClauseSet_p set);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
