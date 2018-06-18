/*-----------------------------------------------------------------------

File  : cte_termtypes.h

Author: Stephan Schulz

Contents

  Declarations for the basic term type and primitive functions, mainly
  on single term cells. This module mostly provides only
  infrastructure for higher level modules.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Created: Tue Feb 24 01:23:24 MET 1998 - Ripped out of the now obsolete
         cte_terms.h

-----------------------------------------------------------------------*/

#ifndef CTE_TERMTYPES

#define CTE_TERMTYPES

#include <clb_partial_orderings.h>
#include <cte_signature.h>
#include <clb_sysdate.h>
#include <clb_ptrees.h>
#include <clb_properties.h>
#include <cte_simpletypes.h>



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define DEFAULT_VWEIGHT  1  /* This has to be an integer > 0! */
#define DEFAULT_FWEIGHT  2  /* This has to be >= DEFAULT_VWEIGHT */

/* POWNRS = Probably obsolete with new rewriting scheme */

typedef enum
{
   TPIgnoreProps      =      0, /* For masking properties out */
   TPRestricted       =      1, /* Rewriting is restricted on this term */
   TPTopPos           =      2, /* This cell is a entry point */
   TPIsGround         =      4, /* Shared term is ground */
   TPPredPos          =      8, /* This is an original predicate
                                   position morphed into a term */
   TPIsRewritable     =     16, /* Term is known to be rewritable with
                                   respect to a current rule or rule
                                   set. Used for removing
                                   backward-rewritable clauses. Absence of
                                   this flag does not mean that the term
                                   is in any kind of normal form! POWNRS */
   TPIsRRewritable    =     32, /* Term is rewritable even if
                                   rewriting is restricted to proper
                                   instances at the top level.*/
   TPIsSOSRewritten   =     64, /* Term has been rewritten with a SoS
                                   clause (at top level) */
   TPSpecialFlag      =    128, /* For internal use with normalizing variables*/
   TPOpFlag           =    256, /* For internal use */
   TPCheckFlag        =    512, /* For internal use */
   TPOutputFlag       =   1024, /* Has this term already been printed (and
                                   thus defined)? */
   TPIsSpecialVar     =   2048, /* Is this a meta-variable generated by term
                                   top operations and the like? */
   TPIsRewritten      =   4096, /* Term has been rewritten (for the new
                                   rewriting scheme) */
   TPIsRRewritten     =   8192, /* Term has been rewritten at a
                                   subterm position or with a real
                                   instance (for the new rewriting
                                   scheme) */
   TPIsShared         =  16384, /* Term is in a term bank */
   TPGarbageFlag      =  32768, /* For the term bank garbage collection */
   TPIsFreeVar        =  65536, /* For Skolemization */
   TPPotentialParamod = 131072, /* This position needs to be tried for
                                   paramodulation */
   TPPosPolarity      = 1<<18,  /* In the term encoding of a formula,
                                   this occurs with positive polarity. */
   TPNegPolarity      = 1<<19,  /* In the term encoding of a formula,
                                   this occurs with negative polarity. */
   TPIsDerefedAppVar  = 1<<20,  /* Is the object obtained as a cache
                                   for applied variables */
   TPFromNonShared    = 1<<21,  /* for dbg */
}TermProperties;



typedef enum  /* See CLAUSES/ccl_rewrite.c for more */
{
   NoRewrite = 0,     /* Just for completness */
   RuleRewrite = 1,   /* Rewrite with rules only */
   FullRewrite = 2    /* Rewrite with rules and equations */
}RewriteLevel;

typedef struct
{
   SysDate          nf_date[FullRewrite]; /* If term is not rewritten,
                                             it is in normal form with
                                             respect to the
                                             demodulators at this date */
   struct {
      struct termcell*   replace;         /* ...otherwise, it has been
                                             rewritten to this term */
      // long               demod_id;        /* 0 means subterm! */
      struct clause_cell *demod;          /* NULL means subterm! */
   }rw_desc;
}RewriteState;


typedef struct termcell
{
   FunCode          f_code;        /* Top symbol of term */
   TermProperties   properties;    /* Like basic, lhs, top */
   int              arity;         /* Redundant, but saves handing
                                      around the signature all the
                                      time */
   struct termcell* *args;         /* Pointer to array of arguments */
   struct termcell* binding;       /* For variable bindings,
                                      potentially for temporary
                                      rewrites - it might be possible
                                      to combine the previous two in a
                                      union. */
   struct termcell* binding_cache; /* For caching the term applied variable
                                      expands to. */
   long             entry_no;      /* Counter for terms in a given
                                      termbank - needed for
                                      administration and external
                                      representation */
   long             weight;        /* Weight of the term, if term is in term bank */
   unsigned int     v_count;       /* Number of variables, if term is in term bank */
   unsigned int     f_count;       /* Number of function symbols, if term is in term bank */
   RewriteState     rw_data;       /* See above */
   Type_p           type;          /* Sort of the term */
   struct termcell* lson;          /* For storing shared term nodes in */
   struct termcell* rson;          /* a splay tree - see
                                      cte_termcellstore.[ch] */
}TermCell, *Term_p, **TermRef;


typedef uintptr_t DerefType, *DerefType_p;

#define DEREF_NEVER   0
#define DEREF_ONCE    1
#define DEREF_ALWAYS  2

/* The following is an estimate for the memory taken up by a term cell
   with arguments (the argument array is not counted separately). */

#ifdef CONSTANT_MEM_ESTIMATE
#define TERMCELL_MEM 48
#define TERMARG_MEM  4
#define TERMP_MEM    4
#else
#define TERMCELL_MEM MEMSIZE(TermCell)
#define TERMARG_MEM  sizeof(void*)
#define TERMP_MEM    sizeof(Term_p)
#endif

#define TERMCELL_DYN_MEM (TERMCELL_MEM+4*TERMARG_MEM)

#ifdef ENABLE_LFHO
#define CAN_DEREF(term) ((TermIsVar(term) && (term)->binding) || (TermIsAppliedVar(term) && \
                                (term->args[0]->binding)))
#else
#define CAN_DEREF(term) (((term)->binding))
#endif


// checks if the binding is present and if it is the cache for the
// right term
#define BINDING_FRESH(t) ((t)->binding_cache && (t)->binding && \
                           (t)->binding == (t)->args[0]->binding)

#ifdef ENABLE_LFHO
/* Sometimes we are not interested in the arity of the term, but the 
   number of arguments the term has. Due to encoding of applied variables,
   we have to discard argument 0, which is actually the head variable */ 
#define ARG_NUM(term)    (TermIsAppliedVar(term) ? term->arity-1 : term->arity)
#define DEREF_LIMIT(t,d) ((TermIsAppliedVar(t) && (t)->args[0]->binding && d == DEREF_ONCE) ? \
                          (t)->args[0]->binding->arity + ((TermIsVar((t)->args[0]->binding)) ? 1 : 0) : 0)
#define CONVERT_DEREF(i, l, d) (((i) < (l) && (d) == DEREF_ONCE) ? DEREF_NEVER : (d)) 
#else
/* making sure no compiler warnings are produced */
#define ARG_NUM(term)          (term->arity)
#define DEREF_LIMIT(t,d)       (UNUSED(t),UNUSED(d),0)
#define CONVERT_DEREF(i, l, d) (UNUSED(i),UNUSED(l),d)
#endif

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* Functions which take two terms and return a boolean, i.e. test for
   equality */

#define TERMS_INITIAL_ARGS 10

#define RewriteAdr(level) (assert(level),(level)-1)
#define TermIsVar(t) ((t)->f_code < 0)
#define TermIsConst(t)(!TermIsVar(t) && ((t)->arity==0))
#ifdef ENABLE_LFHO
#define TermIsAppliedVar(term) (term->f_code == SIG_APP_VAR_CODE)
#else
#define TermIsAppliedVar(term) (false)
#endif
#define TermIsTopLevelVar(term) (TermIsVar(term) || TermIsAppliedVar(term))

#define TermCellSetProp(term, prop) SetProp((term), (prop))
#define TermCellDelProp(term, prop) DelProp((term), (prop))
#define TermCellAssignProp(term, sel, prop) AssignProp((term),(sel),(prop))
/* Are _all_ properties in prop set in term? */
#define TermCellQueryProp(term, prop) QueryProp((term), (prop))

/* Are any properties in prop set in term? */
#define TermCellIsAnyPropSet(term, prop) IsAnyPropSet((term), (prop))

#define TermCellGiveProps(term, props) GiveProps((term),(props))
#define TermCellFlipProp(term, props) FlipProp((term),(props))

#define TermCellAlloc() (TermCell*)SizeMalloc(sizeof(TermCell))
#define TermCellFree(junk)         SizeFree(junk, sizeof(TermCell))
#define TermArgArrayAlloc(arity) ((Term_p*)SizeMalloc((arity)*sizeof(Term_p)))
#define TermArgArrayFree(junk, arity) SizeFree((junk),(arity)*sizeof(Term_p))

#define TermIsRewritten(term) TermCellQueryProp((term), TPIsRewritten)
#define TermIsRRewritten(term) TermCellQueryProp((term), TPIsRRewritten)
#define TermIsTopRewritten(term) (TermIsRewritten(term)&&TermRWDemodField(term))
#define TermIsShared(term)       TermCellQueryProp((term), TPIsShared)

#ifdef ENABLE_LFHO
Term_p  MakeRewrittenTerm(Term_p orig, Term_p new, int orig_remains);
#else
#define MakeRewrittenTerm(orig, new, remains) (assert(!remains), new)
#endif

#define TermNFDate(term,i) (TermIsRewritten(term)?\
                           SysDateCreationTime():(term)->rw_data.nf_date[i])

/* Absolutely get the value of the replace and demod fields */
#define TermRWReplaceField(term) ((term)->rw_data.rw_desc.replace)
#define TermRWDemodField(term)   ((term)->rw_data.rw_desc.demod)
#define REWRITE_AT_SUBTERM 0

/* Get the logical value of the replaced term / demodulator */
#define TermRWReplace(term) (TermIsRewritten(term)?TermRWTargetField(term):NULL)
#define TermRWDemod(term) (TermIsRewritten(term)?TermRWDemodField(term):NULL)

static __inline__ Term_p TermDefaultCellAlloc(void);
static __inline__ Term_p TermConstCellAlloc(FunCode symbol);
static __inline__ Term_p TermTopAlloc(FunCode f_code, int arity);
static __inline__ Term_p TermTopCopy(Term_p source);
static __inline__ Term_p TermTopCopyWithoutArgs(Term_p source);

void    TermTopFree(Term_p junk);
void    TermFree(Term_p junk);
Term_p  TermAllocNewSkolem(Sig_p sig, PStack_p variables, Type_p type);

void    TermSetProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermSearchProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermVerifyProp(Term_p term, DerefType deref, TermProperties prop,
                       TermProperties expected);
void    TermDelProp(Term_p term, DerefType deref, TermProperties prop);
void    TermDelPropOpt(Term_p term, TermProperties prop);
void    TermVarSetProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermVarSearchProp(Term_p term, DerefType deref, TermProperties prop);
void    TermVarDelProp(Term_p term, DerefType deref, TermProperties prop);
bool    TermHasInterpretedSymbol(Term_p term);

bool    TermIsPrefix(Term_p needle, Term_p haystack);
static __inline__ Type_p GetHeadType(Sig_p sig, Term_p term);

static __inline__ Term_p  TermDerefAlways(Term_p term);
static __inline__ Term_p  TermDeref(Term_p term, DerefType_p deref);

static __inline__ Term_p  TermTopCopy(Term_p source);

void    TermStackSetProps(PStack_p stack, TermProperties prop);
void    TermStackDelProps(PStack_p stack, TermProperties prop);

void ClearStaleCache(Term_p app_var);

/*---------------------------------------------------------------------*/
/*                  Inline functions                                   */
/*---------------------------------------------------------------------*/

// forward declaration of function used in inline functions 
Term_p applied_var_deref(Term_p orig);

/*-----------------------------------------------------------------------
//
// Function: GetHeadType()
//
//   Returns the type of the head term symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ Type_p GetHeadType(Sig_p sig, Term_p term)
{
#ifdef ENABLE_LFHO
   if(TermIsAppliedVar(term))
   {
      assert(!sig || term->f_code == SIG_APP_VAR_CODE);
      return term->args[0]->type;
   }
   else if(TermIsVar(term))
   {
      assert(term->arity == 0);
      return term->type;
   }
   else
   {
      assert(term->f_code != SIG_APP_VAR_CODE);
      return SigGetType(sig, term->f_code);
   }
#else
   return SigGetType(sig, term->f_code);
#endif
}

/*-----------------------------------------------------------------------
//
// Function: deref_step()
//
//   Dereference term once
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ Term_p deref_step(Term_p orig)
{
   assert(TermIsTopLevelVar(orig));
   //assert(TermIsVar(orig) || TermIsShared(orig));
   // assert(bank != NULL || orig->arity == 0);
   if(TermIsVar(orig))
   {
      return orig->binding;
   }
   else
   {
      //fprintf(stderr, "Derefing app var\n");
      return applied_var_deref(orig);
   }
}

/*-----------------------------------------------------------------------
//
// Function: TermDerefAlways()
//
//   Dereference a term as many times as possible.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermDerefAlways(Term_p term)
{
   assert(TermIsTopLevelVar(term) || !(term->binding));

   while(CAN_DEREF(term))
   {
#ifdef ENABLE_LFHO
      term = deref_step(term);
#else
      term = term->binding;
#endif
   }
   return term;
}

/*-----------------------------------------------------------------------
//
// Function: TermDeref()
//
//   Dereference a term. deref* tells us how many derefences to do
//   at most, it will be decremented for each dereferenciation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermDeref(Term_p term, DerefType_p deref)
{
   assert(TermIsTopLevelVar(term) || !(term->binding));

   if(*deref == DEREF_ALWAYS)
   {
      while(CAN_DEREF(term))
      {
#ifdef ENABLE_LFHO
      term = deref_step(term);
#else
      term = term->binding;
#endif
      }
   }
   else
   {
      while(*deref && CAN_DEREF(term))
      {
#ifdef ENABLE_LFHO
      bool originally_app_var = TermIsAppliedVar(term);
      term = deref_step(term);
      if((*deref) == DEREF_ONCE && originally_app_var)
      {
        break;
      }
      else
      {
        (*deref)--;
      }
#else
      term = term->binding;
      (*deref)--;
#endif
      }
   }
   return term;
}


/*-----------------------------------------------------------------------
//
// Function: TermTopCopyWithoutArgs()
//
//   Return a copy of the term node.
//   Only the top node is duplicated.
//   Arguments are not initialized.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermTopCopyWithoutArgs(restrict Term_p source)
{
   Term_p handle = TermDefaultCellAlloc();

   /* All other properties are tied to the specific term! */
   handle->properties = (source->properties&(TPPredPos));
   TermCellDelProp(handle, TPOutputFlag); /* As it gets a new id below */

   handle->f_code = source->f_code;
   handle->type   = source->type;

   if(source->arity)
   {
      handle->arity = source->arity;
      handle->args  = TermArgArrayAlloc(source->arity);
   }

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermTopCopy()
//
//   Return a copy of the term node (and potential argument
//   pointers). Only the top node and the pointers are duplicated, the
//   arguments are shared between source and copy. As this function
//   operates on nodes, it does not follow bindings! Administrative
//   stuff (refs etc. will, of course, not be copied but initialized
//   to rational values for an unshared
//   term).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermTopCopy(restrict Term_p source)
{
   Term_p handle = TermTopCopyWithoutArgs(source);

   for(int i=0; i<source->arity; i++)
   {
      handle->args[i] = source->args[i];
   }

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermDefaultCellAlloc()
//
//   Allocate a term cell with default values.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermDefaultCellAlloc(void)
{
   Term_p handle = TermCellAlloc();

   handle->properties = TPIgnoreProps;
   handle->arity      = 0;
   handle->type       = NULL;
   handle->binding    = NULL;
   handle->binding_cache = NULL;
   handle->args       = NULL;
   handle->rw_data.nf_date[0] = SysDateCreationTime();
   handle->rw_data.nf_date[1] = SysDateCreationTime();
   handle->lson = NULL;
   handle->rson = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermConstCellAlloc()
//
//   Allocate a term cell for the constant term with symbol symbol.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermConstCellAlloc(FunCode symbol)
{
   Term_p handle = TermDefaultCellAlloc();
   handle->f_code = symbol;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TermTopAlloc()
//
//   Allocate a term top with given f_code and (uninitialized)
//   argument array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ Term_p TermTopAlloc(FunCode f_code, int arity)
{
   Term_p handle = TermDefaultCellAlloc();

   handle->f_code = f_code;
   handle->arity  = arity;
   if(arity)
   {
      handle->args = TermArgArrayAlloc(arity);
   }

   return handle;
}

#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
