/*-----------------------------------------------------------------------

File  : cte_simpletypes.c

Author: Simon Cruanes (simon.cruanes@inria.fr)

Contents

  Implementation of simple types for the TSTP TFF format

  Copyright 2013 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<2> Somewhere at the end of 2017. Completely rewritten 
    by Petar Vukmirovic.

<1> Sat Jul  6 09:45:14 CEST 2013
    New

-----------------------------------------------------------------------*/

#ifndef CTE_SIMPLETYPES

#define CTE_SIMPLETYPES

#include "cio_scanner.h"
#include <clb_ptrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

#define ArrowTypeCons 0
#define STBool        1     /* Boolean sort, will replace/extend the predicate bit */
#define STIndividuals 2     /* Default sort, "individuums" */
#define STKind        3     /* The "sort of sorts", $tType in TFF */
#define STInteger     4     /* Integer numbers */
#define STRational    5     /* Rational numbers */
#define STReal        6     /* Reals */
#define STPredefined  STReal

typedef long TypeUniqueID;
typedef long TypeConsCode;

typedef struct typecell {
   TypeConsCode      f_code; // Called the same as for terms.
   int               arity;
   struct typecell** args;
   TypeUniqueID      type_uid;
} TypeCell, *Type_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SortIsUserDefined(sort) ((sort) > STPredefined)
#define SortIsInterpreted(sort) (((sort)>=STInteger)&&((sort)<=STPredefined))

Type_p  GetReturnSort(Type_p type);

#define NO_TYPE                 NULL
#define INVALID_TYPE_UID        -1

#define TypeIsKind(t)   ((t)->f_code == STKind)
#define TypeIsBool(t)   ((t)->f_code == STBool)
#define TypeIsArrow(t)  ((t)->f_code == ArrowTypeCons)

#define TypeIsPredicate(t) (TypeIsBool(t) || (TypeIsArrow(t) && TypeIsBool((t)->args[(t)->arity-1])))

#define TypeCellAlloc()    (Type_p) SizeMalloc(sizeof(TypeCell))
#define TypeCellFree(junk) SizeFree(junk, sizeof(TypeCell))

Type_p  TypeCopy(Type_p orig);

#define AllocSimpleSort(code)       TypeAlloc(code, 0, NULL)
#define AllocArrowType(arity, args) TypeAlloc(ArrowTypeCons, arity, args)

#define TypeArgArrayAlloc(n) ((Type_p*) ((n) == 0 ? NULL : SizeMalloc((n)*sizeof(Type_p))))

#define  TypeIsArrow(t)       ((t)->f_code == ArrowTypeCons)
#define  TypeIsKind(t)        ((t)->f_code == STKind)
#define  TypeIsIndividual(t)  ((t)->f_code == STIndividuals)
#define  TypeIsTypeConstructor(t) (TypeIsKind(t) || (TypeIsArrow(t) && TypeIsKind((t)->args[0])))

int TypeGetMaxArity(Type_p t); 

int TypesCmp(Type_p t1, Type_p t2);
Type_p FlattenType(Type_p type);
DStr_p TypeAppEncodedName(Type_p type);

bool TypeHasBool(Type_p t);

void TypeFree(Type_p junk);

/*-----------------------------------------------------------------------
//
// Function: TypeAlloc()
//
//   Allocates new type cell.
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
static inline Type_p TypeAlloc(TypeConsCode c_code, int arity, Type_p* args)
{
   Type_p handle = TypeCellAlloc();

   handle->f_code = c_code;
   handle->arity  = arity;
   handle->args   = args;
   handle->type_uid  = INVALID_TYPE_UID;

   return handle;
}

#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
