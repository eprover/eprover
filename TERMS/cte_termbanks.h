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
#include <cte_termcellstore.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct tbcell
{
   unsigned long in_count;       /* How many terms have been inserted? */
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
   TermProperties garbage_state; /* For the mark-and sweep garbage
				    collection. This is flipped at
				    each sweep, and all new term cell
				    get the new value, so that marking
				    can be done by flipping in the
				    term cell. */
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
   TermCellStoreCell term_store; /* Here are the terms */
   struct gc_admin_cell *gc;     /* Higher level code can register
                                  * garbage collection information
                                  * here. This is only a convenience
                                  * link, memory needs to be managed
                                  * elsewhere. */
}TBCell, *TB_p;
   


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern bool TBPrintTermsFlat;
extern bool TBPrintInternalInfo;
extern bool TBPrintDetails;

#define TBCellAlloc() (TBCell*)SizeMalloc(sizeof(TBCell))
#define TBCellFree(junk)         SizeFree(junk, sizeof(TBCell))

TB_p    TBAlloc(Sig_p sig);
void    TBFree(TB_p junk);

long    TBTermNodes(TB_p bank);
#define TBNonVarTermNodes(bank) TermCellStoreNodes(&(bank)->term_store)
#define TBStorage(bank)\
         (TERMCELL_DYN_MEM*(bank)->term_store.entries\
         +(bank)->term_store.arg_count*TERMP_MEM)

#define TBCellIdent(term) (TermIsVar(term)?(term)->f_code:term->entry_no)

Term_p  DefaultSharedTermCellAlloc(void);
#define TermIsTrueTerm(term) ((term)->f_code==SIG_TRUE_CODE)

static __inline__ bool TBTermEqual(Term_p t1, Term_p t2);
#define TBTermIsSubterm(super, term)\
        TermIsSubterm((super),(term),DEREF_NEVER,TBTermEqual)

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

Term_p  TBAllocNewSkolem(TB_p bank, PStack_p variables, bool atom);

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


/*-----------------------------------------------------------------------
//
// Function:  TBTermEqual()
//
//   Test wether two shared terms in the same termbank are the
//   same. This is a simple pointer comparison, this function only
//   exists so that it can be passed as a function pointer.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ bool TBTermEqual(Term_p t1, Term_p t2)
{
   return (t1==t2);
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





