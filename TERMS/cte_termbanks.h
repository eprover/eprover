/*-----------------------------------------------------------------------

File  : cte_termbanks.h

Author: Stephan Schulz

Contents

  Definitions for term banks - i.e. shared representations of terms as
  defined in cte_terms.h. Uses the same struct, but adds
  administrative stuff and functionality for sharing.

  There are two sets of funktions for the manangment of term trees:
  Funktions operating only on the top cell, and functions descending
  the term structure. Top level functions implement a conventional splay
  tree with key f_code.masked_properties.entry_nos_of_args and are
  implemented in cte_termtrees.[ch]

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Sep 22 00:15:39 MET DST 1997
    New
<2> Wed Feb 25 18:16:34 MET 1998
    Adapted for use of new term modules with shared variables
<3> Sat Apr  6 21:42:35 CEST 2002
    Changed for new rewriting

-----------------------------------------------------------------------*/

#ifndef CTE_TERMBANKS

#define CTE_TERMBANKS

#include <clb_numtrees.h>
#include <cio_basicparser.h>
#include <cte_varsets.h>
#include <cte_termcellstore.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct tbcell
{
   unsigned long in_count;       /* How many terms have been inserted? */
   unsigned long long insertions;/* How many termtops have been
                                    inserted into the term bank? This
                                    counts all attempted insertions
                                    (as a measure of work done. */
   Sig_p         sig;            /* Store sig info */
   VarBank_p     vars;           /* Information about (shared) variables */
   Term_p        true_term;      /* Pointer to the special term with the
                                    $true constant. */
   Term_p        false_term;     /* Pointer to the special term with the
                                    $false constant. */
   Term_p        min_term;       /* A small (ideally the minimal
                                    possible) term, to be used for RHS
                                    instantiation. */
   unsigned long rewrite_steps;  /* How many calls to TBTermReplace? */
   VarSetStore_p freevarsets;    /* Associates a term (or Tformula)
                                  * with the set of its free
                                  * variables. Only initalized for
                                  * specific operations and then reset
                                  * again */
   TermProperties garbage_state; /* For the mark-and sweep garbage
                                    collection. This is flipped at
                                    each sweep, and all new term cell
                                    get the new value, so that marking
                                    can be done by flipping in the
                                    term cell. */
   struct gc_admin_cell *gc;     /* Higher level code can register
                                  * garbage collection information
                                  * here. This is only a convenience
                                  * link, memory needs to be managed
                                  * elsewhere. */
   PDArray_p     ext_index;      /* Associate _external_ abbreviations (=
                                    entry_no's with term nodes, necessary
                                    for parsing of term bank terms. For
                                    critical cases (full protocolls) this
                                    is bound to be densly poulated -> we
                                    use an array. Please note that term
                                    replacing does not invalidate entries
                                    in ext_index
                                    (it would be pretty expensive in
                                    terms of time and memory), so higher
                                    layers have to take care of this if
                                    they want to both access terms via
                                    references and do replacing! */
   TermCellStoreCell term_store; /* Here are the terms */
}TBCell, *TB_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern bool TBPrintTermsFlat;
extern bool TBPrintInternalInfo;
extern bool TBPrintDetails;

#define TBCellAlloc() (TBCell*)SizeMalloc(sizeof(TBCell))
#define TBCellFree(junk)         SizeFree(junk, sizeof(TBCell))

#define TBSortTable(tb) (tb->vars->sort_table)

TB_p    TBAlloc(Sig_p sig);
void    TBFree(TB_p junk);

void    TBVarSetStoreFree(TB_p bank);

long    TBTermNodes(TB_p bank);
#define TBNonVarTermNodes(bank) TermCellStoreNodes(&(bank)->term_store)
#define TBStorage(bank)\
         (TERMCELL_DYN_MEM*(bank)->term_store.entries\
         +(bank)->term_store.arg_count*TERMP_MEM)

#define TBCellIdent(term) (TermIsVar(term)?(term)->f_code:term->entry_no)

#define TermIsTrueTerm(term) ((term)->f_code==SIG_TRUE_CODE)

#define TBTermIsSubterm(super, term) TermIsSubterm((super),(term),DEREF_NEVER)

#define TBTermIsTypeTerm(term)\
        ((term)->weight==(DEFAULT_VWEIGHT+DEFAULT_FWEIGHT))
#define TBTermIsXTypeTerm(term)\
        (term->arity && ((term)->weight==(DEFAULT_FWEIGHT+(term)->arity*DEFAULT_VWEIGHT)))
#define TBTermIsGround(t) TermCellQueryProp((t), TPIsGround)

Term_p  TBInsert(TB_p bank, Term_p term, DerefType deref);
Term_p  TBInsertNoProps(TB_p bank, Term_p term, DerefType deref);
Term_p  TBInsertRepl(TB_p bank, Term_p term, DerefType deref, Term_p old, Term_p repl);
Term_p  TBInsertInstantiated(TB_p bank, Term_p term);

Term_p  TBInsertOpt(TB_p bank, Term_p term, DerefType deref);
Term_p  TBInsertDisjoint(TB_p bank, Term_p term);

Term_p  TBTermTopInsert(TB_p bank, Term_p t);

Term_p  TBAllocNewSkolem(TB_p bank, PStack_p variables, SortType sort);

Term_p  TBFind(TB_p bank, Term_p term);

void    TBPrintBankInOrder(FILE* out, TB_p bank);
void    TBPrintTermCompact(FILE* out, TB_p bank, Term_p term);
#define TBPrintTermFull(out, bank, term) \
        TermPrint((out), (term), (bank)->sig, DEREF_NEVER)
void    TBPrintTerm(FILE* out, TB_p bank, Term_p term, bool fullterms);
void    TBPrintBankTerms(FILE* out, TB_p bank);
Term_p  TBTermParseReal(Scanner_p in, TB_p bank, bool check_symb_prop);

#define  TBTermParse(in, bank) TBTermParseReal((in),(bank), true)
#define  TBRawTermParse(in, bank) TBTermParseReal((in),(bank), false)

void    TBRefSetProp(TB_p bank, TermRef ref, TermProperties prop);
void    TBRefDelProp(TB_p bank, TermRef ref, TermProperties prop);

long    TBTermDelPropCount(Term_p term, TermProperties prop);

#define TBTermCellIsMarked(bank, term) \
        (GiveProps((term),TPGarbageFlag)!=(bank)->garbage_state)
void    TBGCMarkTerm(TB_p bank, Term_p term);
long    TBGCSweep(TB_p bank);
Term_p  TBCreateMinTerm(TB_p bank, FunCode min_const);

long    TBTermCollectSubterms(Term_p term, PStack_p collector);


/*---------------------------------------------------------------------*/
/*                Inline Functions                                     */
/*---------------------------------------------------------------------*/


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
