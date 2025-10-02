/*-----------------------------------------------------------------------

  File  : cte_simpletypes.c

  Author: Simon Cruanes (simon.cruanes@inria.fr),  Petar Vukmirovic,
  Stephan Schulz

  Contents

  Implementation of simple types for the TSTP TFF (and THF) format.

  A complex ("arrow") type is an array [t1,...,tn, t], representing
  the type t1 -> ... -> tn -> t or (t1, ... tn) -> tn, depending on
  yout viewpoint. In particular, the last element in the array is the
  return sort.

  Copyright 2013-2025 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sat Jul  6 09:45:14 CEST 2013
  Completely rewritten by Petar Vukmirovic around the end of 2017.

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
#define STPredefined  STReal /* Greatest pre-defined type encoding */

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

#define AllocSimpleSort(code)       TypeAlloc((code), 0, NULL)
#define AllocArrowType(arity, args) ((assert((arity) > 0)),\
   ((arity) == 1 ? (args)[0] : TypeAlloc(ArrowTypeCons, (arity), (args))))

#define TypeArgArrayAlloc(n) ((Type_p*) ((n) == 0 ? NULL : SizeMalloc((n)*sizeof(Type_p))))
#define TypeArgArrayFree(junk, n) (((n)==0) ? NULL : ( SizeFreeReal((junk),((n)*sizeof(Type_p))) ))

#define  TypeIsArrow(t)       ((t)->f_code == ArrowTypeCons)
#define  TypeIsKind(t)        ((t)->f_code == STKind)
#define  TypeIsIndividual(t)  ((t)->f_code == STIndividuals)
#define  TypeIsTypeConstructor(t) (TypeIsKind(t) || (TypeIsArrow(t) && TypeIsKind((t)->args[0])))

bool TypeIsUntyped(Type_p t);

#define  GetRetType(t)        (TypeIsArrow(t) ? (t)->args[(t)->arity-1] : (t))

int TypeGetMaxArity(Type_p t);

int TypesCmp(Type_p t1, Type_p t2);
Type_p FlattenType(Type_p type);
DStr_p TypeAppEncodedName(Type_p type);

Type_p ArrowTypeFlattened(Type_p const* args, int args_num, Type_p ret);
Type_p TypeDropFirstArg(Type_p ty);
bool IsChoiceType(Type_p ty);


bool TypeHasBool(Type_p t);

void TypeFree(Type_p junk);


/*-----------------------------------------------------------------------
//
// Function: TypeAlloc()
//
//   Allocates new type cell.
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


/*-----------------------------------------------------------------------
//
// Function: AllocArrowTypeCopyArgs()
//
//   Allocates an arrow type where arguments of arrow are represented
//   in a statically allocated array -- thus we need to dynamically
//   allocate them and copy them in the dynamic array.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
static inline Type_p AllocArrowTypeCopyArgs(int arity, Type_p const* args)
{
   assert(arity > 0);
   Type_p* args_copy = TypeArgArrayAlloc(arity);
   for(int i=0; i<arity; i++)
   {
      args_copy[i] = args[i];
   }
   return AllocArrowType(arity, args_copy); //casting away the cons
}

/*-----------------------------------------------------------------------
//
// Function: TypeGetOrder()
//
//   Calculates the order of the type.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
static inline int TypeGetOrder(Type_p t)
{
   int order=0;
   if(TypeIsArrow(t))
   {
      assert(!TypeIsArrow(t->args[t->arity-1]));
      for(int i=0; i<t->arity; i++)
      {
         order = MAX(order, TypeGetOrder(t->args[i]));
      }
      order++;
   }
   return order;
}

#define VAR_ORDER(ty) (TypeGetOrder(ty) + (TypeIsArrow(ty) ? 1 : 0))

#endif


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
