/*-----------------------------------------------------------------------

File  : cto_cmpcache.c

Author: Stephan Schulz

Contents

  Cache for LPO-like comparisons.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jan  5 20:21:36 MET 2000
    New

-----------------------------------------------------------------------*/

#include "cto_cmpcache.h"



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
// Function: prepare_key()
//
//   Turn the 4 values into an ordered key. Return false if values are
//   reordered, true otherwise.
//
// Global Variables: -
//
// Side Effects    : Builds the key in *key.
//
/----------------------------------------------------------------------*/

static bool prepare_key(Term_p t1, DerefType d1, Term_p t2, DerefType
         d2, QuadKey_p key)
{
   if(DoubleKeyCmp(t1, d1, t2, d2) > 0)
   {
      key->p1 = t1;
      key->i1 = d1;
      key->p2 = t2;
      key->i2 = d2;
      return true;
   }
   key->p1 = t2;
   key->i1 = d2;
   key->p2 = t1;
   key->i2 = d1;
   return false;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: CmpCacheFind()
//
//   Find a certain comparison in the cache.
//
// Global Variables: -
//
// Side Effects    : Only by splaying the tree.
//
/----------------------------------------------------------------------*/

CompareResult CmpCacheFind(CmpCache_p *cache, Term_p t1, DerefType d1,
            Term_p t2, DerefType d2)
{
   QuadKey key;
   QuadTree_p handle;
   CompareResult res = to_unknown;
   bool nat_order;

   if(TermIsFreeVar(t1)||TermIsFreeVar(t2))
   {
      return to_unknown;
   }
   nat_order = prepare_key(t1,d1,t2,d2,&key);
   handle = QuadTreeFind(cache, &key);

   if(nat_order)
   {
      if(handle)
      {
    res = handle->val.i_val;
      }
   }
   else
   {
      if(handle)
      {
    res = POInverseRelation(handle->val.i_val);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: CmpCacheInsert()
//
//   Insert a comparison into an LPO cache. Return false if value
//   already existed, true otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool CmpCacheInsert(CmpCache_p *cache, Term_p t1, DerefType d1, Term_p
          t2, DerefType d2, CompareResult insert)
{
   QuadKey key;
   IntOrP val;
   bool nat_order, res;
   QuadTree_p handle;

   assert(insert!=to_unknown);
   /* printf("%p:%d:%p:%d:%d\n", t1,d1,t2,d2,insert); */

   if(TermIsFreeVar(t1)||TermIsFreeVar(t2))
   {
      return false;
   }

   nat_order = prepare_key(t1,d1,t2,d2,&key);
   if(nat_order)
   {
      val.i_val = insert;
   }
   else
   {
      val.i_val = POInverseRelation(insert);
   }
   handle = QuadTreeFind(cache, &key);
   if(handle)
   {
      switch(handle->val.i_val)
      {
      case to_notgteq:
       /* printf("notgteq=%ld\n", val.i_val); */
       if(val.i_val==to_notleeq)
       {
          handle->val.i_val = to_uncomparable;
       }
       else
       {
          assert((val.i_val==to_notgteq)||
            (val.i_val==to_lesser)||
            (val.i_val==to_uncomparable));
          handle->val.i_val = val.i_val;
       }
       break;
      case to_notleeq:
       /* printf("notleeq=%ld\n", val.i_val); */
       if(val.i_val==to_notgteq)
       {
          handle->val.i_val = to_uncomparable;
       }
       else
       {
          assert((val.i_val==to_notleeq)||
            (val.i_val==to_greater)||
            (val.i_val==to_uncomparable));
          handle->val.i_val = val.i_val;
       }
       break;
      default:
       assert((handle->val.i_val==val.i_val)
         ||((val.i_val==to_notgteq)
            &&(handle->val.i_val==to_lesser))
         ||((val.i_val==to_notleeq)
            &&(handle->val.i_val==to_greater)));
       break;
      }
      return false;
   }
   else
   {
      res =  QuadTreeStore(cache, &key, val);
      return res;
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/








