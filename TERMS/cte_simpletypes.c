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
   SizeFree(junk->args, junk->arity*sizeof(Type_p));
   TypeTopFree(junk);
}