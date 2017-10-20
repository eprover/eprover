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