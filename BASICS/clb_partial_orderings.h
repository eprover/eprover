/*-----------------------------------------------------------------------

File  : clb_partial_orderings.h

Author: Stephan Schulz

Contents

  Functions and datatypes useful in dealing with partial orderings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 16 22:37:09 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CLB_PARTIAL_ORDERINGS

#define CLB_PARTIAL_ORDERINGS

#include <clb_defines.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Possible results of partial ordering comparisons */

typedef enum
{
   to_unknown = 0,
   to_uncomparable = 1,
   to_equal = 2,
   to_greater = 3,
   to_lesser = 4,
   to_notgteq, /* For caching partial LPO results */
   to_notleeq
}CompareResult;

typedef enum ho_order_kind 
{
   LFHO_ORDER, // older LFHO order
   LAMBDA_ORDER  // Boolean free HO order
} HoOrderKind;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char*  POCompareSymbol[];

/* Translating standard UNIX total Quasi-Ordering comparions into
   CompareResult: */

#define Q_TO_PART(res) (((res)<0) ? to_lesser:\
                       (((res)>0) ? to_greater:to_equal))


static inline CompareResult POInverseRelation(CompareResult
                    relation);


/*---------------------------------------------------------------------*/
/*                  Implementations as inline functions                */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: POInverseRelation()
//
//   Given a comparison relation, return the inverse relation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline CompareResult POInverseRelation(CompareResult
                    relation)
{
   CompareResult res = relation;

   switch(relation)
   {
   case to_equal:
   case to_uncomparable:
    break;
   case to_greater:
    res = to_lesser;
    break;
   case to_lesser:
    res = to_greater;
    break;
   case to_notgteq:
    res = to_notleeq;
    break;
   case to_notleeq:
    res = to_notgteq;
    break;
   default:
    assert(false);
    break;
   }
   return res;
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





