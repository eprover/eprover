/*-----------------------------------------------------------------------

  File  : cte_simpletypes.c

  Author: Simon Cruanes (simon.cruanes@inria.fr)

  Contents

  Implementation of simple types for the TSTP TFF format

  Copyright 2013-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

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
Type_p  TypeCopy(Type_p orig)
{
   Type_p handle = TypeAlloc(orig->f_code, orig->arity, TypeArgArrayAlloc(orig->arity));

   for(int i=0; i<orig->arity; i++)
   {
      handle->args[i] = orig->args[i];
   }

   return handle;
}


void TypeTopFree(Type_p junk)
{
   SizeFree(junk, sizeof(*junk)); 
}


void TypeFree(Type_p junk)
{
   if (junk->arity)
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


int TypesCmp(Type_p t1, Type_p t2)
{
   int res = t1->f_code - t2->f_code;

   // if it is not arrow type cons -> same nr of args
   assert(!(t1->f_code == t2->f_code && t1->f_code != ArrowTypeCons) || t1->arity == t2->arity);

   if (!res)
   {      
     res = t1->arity - t2->arity;
     for(int i=0; i<t1->arity && !res; i++)
     {
        res = PCmp(t1->args[i], t2->args[i]);
     }
   }

   return res;
}


bool arguments_flattened(Type_p t)
{
   for(int i=0; i<t->arity-1; i++)
   {
      if (FlattenType(t->args[i]) != t->args[i])
      {
         return false;
      }
   }
   
   return true;
}

// Assumption is that everything before is flattened, it is just the last argument
// that is an arrow 

// return value might be unshared!
Type_p FlattenType(Type_p type)
{
   assert(arguments_flattened(type));

   Type_p res = type;

   if (type->arity && type->args[type->arity-1]->f_code == ArrowTypeCons)
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

Type_p  GetReturnSort(Type_p type)
{
   assert(type);
   if (TypeIsArrow(type))
   {
      return type->args[type->arity-1];
   }
   else
   {
      return type;
   }
}

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
   }
}

DStr_p TypeAppEncodedName(Type_p type)
{
   DStr_p name = DStrAlloc();
   DStrAppendStr(name, "__type_");
   if (SortIsUserDefined(type->f_code) || TypeIsArrow(type))
   {
      DStrAppendInt(name, type->type_uid);
   }
   else
   {
      DStrAppendStr(name, (char*)get_builtin_name(type));
   }
   
   return name;
}