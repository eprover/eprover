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

<1> Sat Jul  6 09:45:14 CEST 2013
    New

-----------------------------------------------------------------------*/

#ifndef CTE_SIMPLETYPES

#define CTE_SIMPLETYPES

#include "cio_scanner.h"
#include "cte_simplesorts.h"

#define STNoSort      0
#define STBool        1     /* Boolean sort, will replace/extend the predicate bit */
#define STIndividuals 2     /* Default sort, "individuums" */
#define STKind        3     /* The "sort of sorts", $tType in TFF */
#define STInteger     4     /* Integer numbers */
#define STRational    5     /* Rational numbers */
#define STReal        6     /* Reals */
#define ArrowTypeCons 7
#define STPredefined  ArrowTypeCons

typedef long      UniqueId;
typedef FunCode   TypeConsCode;

typedef struct typecell {
   TypeConsCode      f_code; // Called the same as for terms.
   int               arity;
   struct typecell** args;
   UniqueId          type_uid;
} TypeCell, *Type_p;

#define SortIsUserDefined(sort) (sort > STPredefined)
#define SortIsInterpreted(sort) (((sort)>=STInteger)&&((sort)<=STPredefined))

#define NO_TYPE                 NULL
#define INVALID_TYPE_UID        -1

#define TypeIsKind(t)   ((t)->f_code == STKind)
#define TypeIsBool(t)   ((t)->f_code == STBool)
#define TypeIsArrow(t)  ((t)->f_code == ArrowTypeCons)

#define TypeCellAlloc()    (Type_p) SizeMalloc(sizeof(typecell))
#define TypeCellFree(junk) SizeFree(junk, sizeof(typecell))


static __inline__ TypeAlloc(TypeConsCode c_code, int arity, Type_p args)
{
   Type_p handle = TypeCellAlloc();

   handle->f_code = c_code;
   handle->arity  = arity;
   handle->args   = args;
   handle->uid    = INVALID_TYPE_UID;
}

Type_p  TypeCopy(Type p);

#define AllocSimpleSort(code)       TypeAlloc(c_code, 0, NULL)
#define AllocArrowType(arity, args) TypeAlloc(ArrowTypeCons, arity, args)

#define TypeArgArrayAlloc(n) (Type_p*) SizeMalloc(n*sizeof(Type_p))

#define  TypeIsArrow(t) ((t)->f_code == ArrowTypeCons)
#define  TypeIsKind(t)  ((t)->f_code == STKind)
#define  TypeIsTypeConstructor (TypeIsKind(t) || TypeIsArrow(t) && TypeIsKind((t)->args[0]))


#endif
