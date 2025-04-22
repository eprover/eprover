/*-----------------------------------------------------------------------

File  : cte_signature.h

Author: Stephan Schulz

Contents

  Definitions for dealing with signatures, i.e. data structures
  storing information about function symbols and their properties.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Sep 18 16:54:31 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CTE_SIGNATURE

#define CTE_SIGNATURE

#include <clb_stringtrees.h>
#include <clb_numtrees.h>
#include <clb_pdarrays.h>
#include <clb_properties.h>
#include <cte_functypes.h>
#include <cte_simpletypes.h>
#include <cte_typebanks.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   FPIgnoreProps  =    0, /* No properties, mask everything out */
   FPTypeFixed    =    1, /* We are sure about the type of the symbol */
   FPTypePoly     =    2, /* Ad-hoc polymorphic predicate type */
   FPFOFOp        =    4, /* Symbol is encoded first order operator */
   FPSpecial      =    8, /* Symbol is a special symbol introduced internally */
   FPAssociative  =   16, /* Function symbol is binary and associative */
   FPCommutative  =   32, /* Function symbol is binary and commutates */
   FPIsAC         =   FPAssociative|FPCommutative,
   FPInterpreted  =   64, /* Interpreted symbol $ident */
   FPIsInteger    =  128, /* Sequence of digits, may be semi-interpreted */
   FPIsRational   =  256, /* [-]a/b */
   FPIsFloat      =  512, /* Floating point number */
   FPIsObject     =  1024, /* ""-enclosed string, by definition denotes
                            unique object." */
   FPDistinctProp =  FPIsObject | FPIsInteger | FPIsRational | FPIsFloat,
   FPOpFlag       = 2048, /* Used for temporary oerations, by
                           * defintion off if not in use! */
   FPClSplitDef   = 4096, /* Predicate is a clause split defined
                           * symbol. */
   FPPseudoPred   = 8192, /* Pseudo-predicate used for side effects
                           * only, does not conceptually contribute to
                           * truth of clause */
   FPTypedApplication = FPPseudoPred * 2, /* Symbol used to represent typed
                                             first-order binary application symbol */
   FPIsInjDefSkolem = FPTypedApplication * 2, /* Symbol is Skolem for injective function */
   FPSkolemSymbol = FPIsInjDefSkolem * 2, /* Obvious ;-) */
   FPDefPred      = FPSkolemSymbol * 2, /* Used for all new epred()s,
                                        * but hopefully only for split
                                        * literals and Tseitin defined
                                        * predicates */
   FPDefFun       = FPDefPred *2, /* Used for edef()s, currently not
                                   * useful ;-) */
}FunctionProperties;


struct sigcell;
struct termcell;

/* These function are defined in cte_typecheck.c and return the return
 * sort of the term, or NULL if the typecheck fails. Polymorphic
 * symbols have no fixed type! */

typedef Type_p (*PolyTypeCheckFun)(struct sigcell *sig, struct termcell *t);

Type_p TypeCheckEq(struct sigcell *sig, struct termcell *t);
Type_p TypeCheckDistinct(struct sigcell *sig, struct termcell *t);
Type_p TypeCheckArithBinop(struct sigcell *sig, struct termcell *t);
Type_p TypeCheckArithConv(struct sigcell *sig, struct termcell *t);


/* Keep information about function symbols: Access external name and
   arity (and possibly additional information at a later time) by
   internal numerical code for function symbol. */

typedef struct funccell
{
   /* f_code is implicit by position in the array */
   char*  name;     // "real" name
   char*  pname;    // Used for external printing
   int    arity;
   int    alpha_rank; /* We sometimes need an arbitrary but stable
                         order on symbols and use alphabetic. */
   int    feature_offset; /* For use with (heuristic) term features,
                             based on arity and function/predicate
                             distinction. */
   PolyTypeCheckFun poly_typecheck;
   Type_p type;       /* Simple type of the symbol */
   FunctionProperties properties;
}FuncCell, *Func_p;


/* A signature contains information about function symbols with
   direct access by internal code (f_info is organized as a array,
   with f_info[f_code] being the information associated with f_code)
   and efficient access by external name (via the f_index array).

   Function codes are integers starting at 1, while variables are
   encoded by negative integers. 0 is unused and can thus express
   error conditions when accessing some things f_code. f_info[0] is
   unused. */

#define DEFAULT_SIGNATURE_SIZE 20
#define DEFAULT_SIGNATURE_GROW 2

typedef struct sigcell
{
   bool      alpha_ranks_valid; /* The alpha-ranks are up to date */
   long      size;     /* Size the array */
   FunCode   f_count;  /* Largest used f_code */
   FunCode   internal_symbols; /* Largest auto-inserted internal symbol */
   Func_p    f_info;   /* The array */
   StrTree_p f_index;  /* Back-assoc: Given a symbol, get the index */
   PStack_p  ac_axioms; /* All recognized AC axioms */
   /* The following are special symbols needed for pattern
      manipulation. We want very efficient access to them! Also
      resused in FOF parsing. */
   FunCode   eqn_code;
   FunCode   neqn_code;
   FunCode   cnil_code;
   PDArray_p orn_codes;

   /* The following is for encoding first order formulae as terms. I
      do like to reuse the robust sharing infrastructure for
      CNFization and formula rewriting (inspired by Tommi Juntilla's
      reuse of the same in MathSAT). */
   FunCode   not_code;
   FunCode   qex_code;
   FunCode   qall_code;
   FunCode   and_code;
   FunCode   or_code;
   FunCode   impl_code;
   FunCode   equiv_code;
   FunCode   nand_code;
   FunCode   nor_code;
   FunCode   bimpl_code;
   FunCode   xor_code;
   /* And here are codes for interpreted symbols */
   FunCode   answer_code;       /* For answer literals */
   FunCode   distinct_code;     /* $distinct */

   /* Sort and type banks (type => sort, but a shortcut is useful) */
   TypeBank_p  type_bank;
   bool        typed_symbols; /* Explicit types/sorts */

   /* Counters for generating new symbols */
   long      skolem_count;
   long      newpred_count;
   long      newdef_count;
   /* Which properties are used for recognizing implicit distinctness?*/
   FunctionProperties distinct_props;
   PStack_p let_scopes;
   PStack_p let_names; // collects all the temp names ever allocated
}SigCell, *Sig_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* Limit (exclusive) of distinguished arity for features */

#define SIG_FEATURE_ARITY_LIMIT 6

/* Special constant for internal operations */

#define SIG_TRUE_CODE    1
#define SIG_FALSE_CODE   2
#define SIG_NIL_CODE     3
#define SIG_CONS_CODE    4
#define SIG_PHONY_APP_CODE 17
// used for encoding lambdas with named variables,
// ACHTUNG: appears only during parsing phase
// %x. f (g x) is encoded as NAMED_LAMBDA(X, F(G(X)))
#define SIG_NAMED_LAMBDA_CODE SIG_PHONY_APP_CODE+1
// used for encoding lambdas represented using DB indices
// ACHTUNG: appears only during saturation
// %x. f (g x) is encoded as DB_LAMBDA(F(G(0)))
#define SIG_DB_LAMBDA_CODE SIG_NAMED_LAMBDA_CODE+1

#define SIG_ITE_CODE SIG_DB_LAMBDA_CODE+1
#define SIG_LET_CODE SIG_ITE_CODE+1

/* Handle properties */

#define FuncSetProp(symb, prop) SetProp((symb), (prop))
#define FuncDelProp(symb, prop) DelProp((symb), (prop))

/* Are _all_ properties in prop set for symb? */
#define FuncQueryProp(symb, prop) QueryProp((symb), (prop))

/* Are any properties in prop set in term? */
#define FuncIsAnyPropSet(symb, prop) IsAnyPropSet((symb), (prop))

/* With a more convenient external interface: */

#define SigSetFuncProp(sig, symb, prop) \
        FuncSetProp(&(sig->f_info[(symb)]), (prop))
#define SigDelFuncProp(sig, symb, prop) \
        FuncDelProp(&(sig->f_info[(symb)]), (prop))
#define SigQueryFuncProp(sig, symb, prop) \
        FuncQueryProp(&(sig->f_info[(symb)]), (prop))
#define SigIsAnyFuncPropSet(sig, symb, prop) \
        FuncIsAnyPropSet(&(sig->f_info[(symb)]), (prop))

extern bool      SigSupportLists; /* Auto-Insert special symbols
                 $nil=3, $cons=4 for list
                 representations */
#define SigCellAlloc() (SigCell*)SizeMalloc(sizeof(SigCell))
#define SigCellFree(junk)         SizeFree(junk, sizeof(SigCell))

Sig_p   SigAlloc(TypeBank_p bank);
void    SigInsertInternalCodes(Sig_p sig);
void    SigFree(Sig_p junk);

#define SigGetFCount(sig) ((sig)->f_count)
#define SigExternalSymbols(sig)                         \
        ((sig)->f_count-(sig)->internal_symbols)

#define SigInterpreteNumbers(sig) ((sig)->null_code)

FunCode SigFindFCode(Sig_p sig, const char* name);
static inline int     SigFindArity(Sig_p sig, FunCode f_code);

static inline char*   SigFindName(Sig_p sig, FunCode f_code);
bool    SigIsPredicate(Sig_p sig, FunCode f_code);
bool    SigIsFunction(Sig_p sig, FunCode f_code);
bool    SigIsFixedType(Sig_p sig, FunCode f_code);
void    SigFixType(Sig_p sig, FunCode f_code);
bool    SigIsPolymorphic(Sig_p sig, FunCode f_code);
void    SigSetPolymorphic(Sig_p sig, FunCode f_code, bool value);
bool    SigQueryProp(Sig_p sig, FunCode f, FunctionProperties prop);

#define SigIsFunConst(sig, f_code) (SigFindArity((sig), (f_code))==0&&\
                                    !SigIsPredicate((sig),(f_code)))
#define SigIsSimpleAnswerPred(sig, f_code) \
        ((f_code)==(sig)->answer_code)

void    SigSetSpecial(Sig_p sig, FunCode f_code, bool value);
void    SigSetAllSpecial(Sig_p sig, bool value);
bool    SigIsSpecial(Sig_p sig, FunCode f_code);
int     SigGetAlphaRank(Sig_p sig, FunCode f_code);

FunCode SigInsertId(Sig_p sig, const char* name, int arity, bool
          special_id);
FunCode SigPopId(Sig_p sig);
long    SigBacktrack(Sig_p sig, FunCode f_count);

FunCode SigInsertLetId(Sig_p sig, const char* name, Type_p type);
FunCode SigInsertFOFOp(Sig_p sig, const char* name, int arity);
void    SigPrint(FILE* out, Sig_p sig);
void    SigPrintSpecial(FILE* out, Sig_p sig);
void    SigPrintACStatus(FILE* out, Sig_p sig);
FunCode SigParseKnownOperator(Scanner_p in, Sig_p sig);
FunCode SigParseSymbolDeclaration(Scanner_p in, Sig_p sig, bool special_id);
FunCode SigParse(Scanner_p in, Sig_p sig, bool special_ids);
int     SigFindMaxUsedArity(Sig_p sig);
int     SigFindMaxPredicateArity(Sig_p sig);
int     SigFindMinPredicateArity(Sig_p sig);
int     SigFindMaxFunctionArity(Sig_p sig);
int     SigFindMinFunctionArity(Sig_p sig);
int     SigCountAritySymbols(Sig_p sig, int arity, bool predicates);
int     SigCountSymbols(Sig_p sig, bool predicates);
int     SigAddSymbolArities(Sig_p sig, PDArray_p distrib, bool
             predicates, long selection[]);
long    SigCollectSortConsts(Sig_p sig, Type_p type, PStack_p res);


/* Special functions for dealing with special symbols */

static inline FunCode SigGetEqnCode(Sig_p sig, bool positive);
FunCode SigGetOtherEqnCode(Sig_p sig, FunCode f_code);
static inline FunCode SigGetOrCode(Sig_p sig);
static inline FunCode SigGetCNilCode(Sig_p sig);
FunCode SigGetOrNCode(Sig_p sig, int arity);

FunCode SigGetNewFCode(Sig_p sig, int arity, char *prefix,
                       long *counter, FunctionProperties props);
#define SigGetNewSkolemCode(sig, arity) \
   SigGetNewFCode((sig), (arity),"esk", &(sig)->skolem_count, FPSkolemSymbol)
#define SigGetNewPredicateCode(sig, arity) \
   SigGetNewFCode((sig), (arity),"epred", &(sig)->newpred_count, FPDefPred)
#define SigGetNewDefCode(sig, arity) \
   SigGetNewFCode((sig), (arity),"edef", &(sig)->newdef_count, FPDefFun)

FunCode SigGetNewTypedSkolem(Sig_p sig, Type_p *args, int num_args, Type_p ret_type);

/* Types */
#define SigDefaultSort(sig)  ((sig)->type_bank->default_type)
#define SigGetType(sig, f)   ((sig)->f_info[(f)].type)
void    SigDeclareType(Sig_p sig, FunCode f, Type_p type);
void    SigDeclareFinalType(Sig_p sig, FunCode f, Type_p type);
void    SigDeclareIsFunction(Sig_p sig, FunCode f);
void    SigDeclareIsPredicate(Sig_p sig, FunCode f);
void    SigPrintTypes(FILE* out, Sig_p sig);
void    SigPrintTypeDeclsTSTP(FILE* out, Sig_p sig);
void    SigParseTFFTypeDeclaration(Scanner_p in, Sig_p sig);
void    SigPrintTypeDeclsTSTPSelective(FILE* out, Sig_p sig, NumTree_p *symbols);
bool    SigHasUnimplementedInterpretedSymbols(Sig_p sig);
long    SigFCodesCollectTypes(Sig_p sig, NumTree_p fcodes, PTree_p *types);
void    SigUpdateFeatureOffset(Sig_p sig, FunCode f);
void    SigEnterLetScope(Sig_p sig, PStack_p type_decls);
void    SigExitLetScope(Sig_p sig);


typedef bool (*FunConstCmpFunType)(FunCode,  FunCode, long*, long*);

FunCode SigGetTypedApp(Sig_p sig, Type_p arg1, Type_p arg2, Type_p ret);
void SigPrintAppEncodedDecls(FILE* out, Sig_p sig);
bool SigSymbolUnifiesWithVar(Sig_p sig, FunCode f_code);
bool SigHasChoiceSym(Sig_p sig);

#define SigIsLogicalSymbol(sig, f_code) \
   (assert(f_code > 0),                                                 \
    SigQueryFuncProp(sig, f_code, FPFOFOp) ||                           \
    f_code == SIG_TRUE_CODE || f_code == SIG_FALSE_CODE ||              \
    f_code == (sig)->eqn_code || f_code == (sig)->neqn_code ||          \
    f_code == (sig)->qex_code || f_code == (sig)->qall_code)



/*---------------------------------------------------------------------*/
/*                        Inline functions                             */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: SigFindArity()
//
//   Given  signature and a function symbol code, return the arity of
//   the symbol.
//
// Global Variables: -
//
// Side Effects    : Abort if illegal f_code
//
/----------------------------------------------------------------------*/

static inline int SigFindArity(Sig_p sig, FunCode f_code)
{
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return (sig->f_info[f_code]).arity;
}


/*-----------------------------------------------------------------------
//
// Function: SigFindName()
//
//   Given  signature and a function symbol code, return a pointer to
//   the name. This pointer is only valid as long as the signature
//   exists!
//
// Global Variables: -
//
// Side Effects    : Abort if illegal f_code
//
/----------------------------------------------------------------------*/

static inline char* SigFindName(Sig_p sig, FunCode f_code)
{
   if(!f_code)
   {
      return "UNNAMED_DB";
   }
   assert(f_code > 0);
   assert(f_code <= sig->f_count);

   return (sig->f_info[f_code]).pname;
}


/*-----------------------------------------------------------------------
//
// Function: SigGetEqnCode()
//
//   Return the FunCode for $eq or $neq, create them if non-existant.
//
// Global Variables: -
//
// Side Effects    : May change sig
//
/----------------------------------------------------------------------*/

static inline FunCode SigGetEqnCode(Sig_p sig, bool positive)
{
   assert(sig);

   if(positive)
   {
      if(sig->eqn_code)
      {
         return sig->eqn_code;
      }
      sig->eqn_code = SigInsertId(sig, "$eq", 2, true);
      assert(sig->eqn_code);
      SigSetFuncProp(sig, sig->eqn_code, FPFOFOp | FPTypePoly);
      return sig->eqn_code;
   }
   else
   {
      if(sig->neqn_code)
      {
         return sig->neqn_code;
      }
      sig->neqn_code = SigInsertId(sig, "$neq", 2, true);
      assert(sig->neqn_code);
      SigSetFuncProp(sig, sig->eqn_code, FPFOFOp | FPTypePoly);
      return sig->neqn_code;
   }
}


/*-----------------------------------------------------------------------
//
// Function:  SigGetOrCode()
//
//   As above, for $or
//
// Global Variables: -
//
// Side Effects    : May change sig
//
/----------------------------------------------------------------------*/

static inline FunCode SigGetOrCode(Sig_p sig)
{
   assert(sig);

   if(sig->or_code)
   {
      return sig->or_code;
   }
   sig->or_code = SigInsertId(sig, "$or", 2, true);
   assert(sig->or_code);
   return sig->or_code;
}


/*-----------------------------------------------------------------------
//
// Function:  SigGetCNilCode()
//
//   As above, for $cnil
//
// Global Variables: -
//
// Side Effects    : May change sig
//
/----------------------------------------------------------------------*/

static inline FunCode SigGetCNilCode(Sig_p sig)
{
   assert(sig);

   if(sig->cnil_code)
   {
      return sig->cnil_code;
   }
   sig->cnil_code = SigInsertId(sig, "$cnil", 0, true);
   assert(sig->cnil_code);
   return sig->cnil_code;
}


/*-----------------------------------------------------------------------
//
// Function: SigGetFeatureOffset()
//
//   Return the feature offset of the symbol. This is arity limited by
//   SIG_FEATURE_ARITY_LIMIT for function symbols, the same shifted up
//   by SIG_FEATURE_ARITY_LIMIT for predicate symbols.
//
// Global Variables: -
//
// Side Effects    : May update the value in the sig
//
/----------------------------------------------------------------------*/

static inline int SigGetFeatureOffset(Sig_p sig, FunCode f)
{
   if(sig->f_info[f].feature_offset == -1)
   {
      SigUpdateFeatureOffset(sig,f);
   }
   return sig->f_info[f].feature_offset;
}

#define SigGetDepthFeatureOffset(sig, f)\
   (SigGetFeatureOffset((sig),(f))+2*SIG_FEATURE_ARITY_LIMIT)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
