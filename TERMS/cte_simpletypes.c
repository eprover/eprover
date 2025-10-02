/*-----------------------------------------------------------------------

  File  : cte_simpletypes.c

  Author: Simon Cruanes (simon.cruanes@inria.fr)

  Contents

  Implementation of simple types for the TSTP TFF format

  Copyright 2013-2025 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Redesigned ground up by Petar Vukmirovic in October 2017.

  Created: <1> Sat Jul  6 09:45:14 CEST 2013

  -----------------------------------------------------------------------*/

#include "cte_simpletypes.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: is_flattened()
//
//  Checks if type t is represented as flattened
//  that is it is either a unit type or it is a type such that
//  the last argument is not arrow and all arguments are flattened.
//
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool is_flattened(Type_p t)
{
   for(int i=0; i<t->arity-1; i++)
   {
      if(!is_flattened(t->args[i]))
      {
         return false;
      }
   }

   return t->arity == 0 || !TypeIsArrow(t->args[t->arity-1]);
}


/*-----------------------------------------------------------------------
//
// Function: arguments_flattened()
//
//  Checks if arguments of type t are flattened -- see is_flattened
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool arguments_flattened(Type_p t)
{
   for(int i=0; i<t->arity-1; i++)
   {
      if(!is_flattened(t->args[i]))
      {
         return false;
      }
   }

   return true;
}

/*-----------------------------------------------------------------------
//
// Function: get_builtin_name()
//
//    Returns the name of the built-in type in TPTP syntax.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static const char* get_builtin_name(Type_p t)
{
   assert(!SortIsUserDefined(t->f_code) && !TypeIsArrow(t));

   switch(t->f_code)
   {
      case STBool:
         return "$o";
      case STIndividuals:
         return "$i";
      case STKind:
         return "$tType";
      case STInteger:
         return "$int";
      case STRational:
         return "$rat";
      case STReal:
         return "$real";
      default:
         assert("Type not built-in " && false);
         return ""; // stiffle warning
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TypeIsUntyped()
//
//   Return if the type does not contain any non-standard sorts,
//   i.e. if it is a type that occurs in plain cnf/fof problems.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TypeIsUntyped(Type_p t)
{
   if(!TypeIsArrow(t))
   {
      return (t->f_code == STBool)||(t->f_code == STIndividuals);
   }
   for(int i=0; i<t->arity; i++)
   {
      if(!TypeIsUntyped(t->args[i]))
      {
         return false;
      }
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCopy()
//
//  Creates a shallow copy of orig.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Type_p  TypeCopy(Type_p orig)
{
   Type_p handle = TypeAlloc(orig->f_code, orig->arity,
                             TypeArgArrayAlloc(orig->arity));

   for(int i=0; i<orig->arity; i++)
   {
      handle->args[i] = orig->args[i];
   }

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: TypeTopFree()
//
//  Frees the type cell used by junk.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void TypeTopFree(Type_p junk)
{
   TypeCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: TypeFree()
//
//  Frees the type cell used by junk and the argument array.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void TypeFree(Type_p junk)
{
   assert(junk);
   if(junk->arity)
   {
      assert(junk->args);
      SizeFree(junk->args, junk->arity*sizeof(Type_p));
   }
   else
   {
      assert(junk->args == NULL);
   }
   TypeTopFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: TypesCmp()
//
//  Ad-hoc total order on types. Based on pointer values.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

int TypesCmp(Type_p t1, Type_p t2)
{
   //NOTE 2 STEPHAN:
   // this is the source of some differences in clause sorting.
   int res = t1->f_code - t2->f_code;

   // if it is not arrow type cons -> same nr of args
   assert(!(t1->f_code == t2->f_code && t1->f_code != ArrowTypeCons)
          || t1->arity == t2->arity);

   if(!res)
   {
      res = t1->arity - t2->arity; // Should be 0? (StS)
      for(int i=0; i<t1->arity && !res; i++)
      {
         res = PCmp(t1->args[i], t2->args[i]);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FlattenType()
//
//  Makes sure type is represented using flattened representation, i.e.
//  the one where the last argument is not not arrow.
//  IMPORTANT: Assumes all arguments are flattened.
//             Return value is an unshared type.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Type_p FlattenType(Type_p type)
{
   assert(arguments_flattened(type));

   Type_p res = type;

   if(type->arity && type->args[type->arity-1]->f_code == ArrowTypeCons)
   {
      int total_args = type->arity-1 + type->args[type->arity-1]->arity;
      int i;
      Type_p* args = TypeArgArrayAlloc(total_args);

      for(i=0; i < type->arity-1; i++)
      {
         args[i] = type->args[i];
      }

      // i is now type->arity-1
      for(int j=0; j < type->args[i]->arity; j++)
      {
         args[i+j] = type->args[i]->args[j];
      }

      res = AllocArrowType(total_args, args);
   }

   return res;

}


/*-----------------------------------------------------------------------
//
// Function: GetReturnSort()
//
//  Returns the return type of function with the given type.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Type_p GetReturnSort(Type_p type)
{
   assert(type);
   if(TypeIsArrow(type))
   {
      return type->args[type->arity-1];
   }
   else
   {
      return type;
   }
}

/*-----------------------------------------------------------------------
//
// Function: TypeAppEncodedName()
//
//  Encodes type as a string.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

DStr_p TypeAppEncodedName(Type_p type)
{
   DStr_p name = DStrAlloc();

   if(SortIsUserDefined(type->f_code) || TypeIsArrow(type))
   {
      assert(type->type_uid != INVALID_TYPE_UID);
      DStrAppendStr(name, "type_");
      DStrAppendInt(name, type->type_uid);
   }
   else
   {
      DStrAppendStr(name, (char*)get_builtin_name(type));
   }

   return name;
}

/*-----------------------------------------------------------------------
//
// Function: TypeGetMaxArity()
//
//  Given a type, determine what is the maximal arity of a function
//  symbol.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

inline int TypeGetMaxArity(Type_p t)
{
  return (TypeIsArrow(t) ? (t)->arity-1 : 0);
}

/*-----------------------------------------------------------------------
//
// Function: TypeHasBool()
//
//  Does type have bool as an argument? Recursively checks in arguments
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool TypeHasBool(Type_p t)
{
   if(t->f_code == STBool)
   {
      return true;
   }

   bool ans = false;
   for(int i=0; i<t->arity && !(ans = TypeHasBool(t->args[i])); i++)
      ;

   return ans;
}

/*-----------------------------------------------------------------------
//
// Function: ArrowTypeFlattened()
//
//  Makes the flattened arrow type out of arguments and return type.
//  Flattening refers to flattening out return type when arrow is
//  constructed. If args_num is 0, returns return_type.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Type_p ArrowTypeFlattened(Type_p const* args, int args_num, Type_p ret)
{
   if (args_num == 0)
   {
      return ret;
   }
   else
   {
      Type_p* args_ret = TypeArgArrayAlloc(args_num+1);
      for(int i=0; i<args_num; i++)
      {
         args_ret[i] = args[i];
      }
      args_ret[args_num] = ret;
      Type_p args_ret_ty = AllocArrowType(args_num+1, args_ret);
      Type_p res = FlattenType(args_ret_ty);

      if (res != args_ret_ty)
      {
         TypeFree(args_ret_ty);
      }
      return res;
   }
}

/*-----------------------------------------------------------------------
//
// Function: TypeDropFirstArg()
//
//  Drop the first argument of a type, creating a new,
//  possibly unshared type. Assumes that type is arrow.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Type_p TypeDropFirstArg(Type_p ty)
{
   assert(TypeIsArrow(ty));

   if (ty->arity == 2)
   {
      return ty->args[1];
   }
   else
   {
      assert(ty->arity >= 3);
      Type_p* args = TypeArgArrayAlloc(ty->arity-1);
      for(int i=0; i<ty->arity-1; i++)
      {
         args[i] = ty->args[i+1];
      }
      return AllocArrowType(ty->arity-1, args);

   }

}

/*-----------------------------------------------------------------------
//
// Function: IsChoiceType()
//
//  Does the type correspond a monomorphized type of a choice symbol?
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool IsChoiceType(Type_p ty)
{
   bool is_choice = false;
   if(TypeIsArrow(ty) && ty->arity>=2 &&
      (TypeIsArrow(ty->args[0]) && ty->args[0]->arity == 2 &&
       TypeIsPredicate(ty->args[0])))
   {
      // remaining checks if var type is (A -> o) -> A
      is_choice = true;
      Type_p a_type = ty->args[0]->args[0];
      if(TypeIsArrow(a_type) && a_type->arity == ty->arity-1)
      {
         for(int i=0; is_choice && i< a_type->arity; i++)
         {
            is_choice = is_choice && a_type->args[i] == ty->args[i+1];
         }
      }
      else if (!TypeIsArrow(a_type) && ty->arity == 2)
      {
         is_choice = a_type == ty->args[1];
      }
      else
      {
         is_choice = false;
      }
   }
   return is_choice;
}
