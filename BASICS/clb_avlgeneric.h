/*-----------------------------------------------------------------------

File  : clb_avlgeneric.h

Author: Stephan Schulz

Contents

  Macros for the creation of generic binary tree functions. Currently
  used for traversal functions only. Please note that the name is
  obsolete (used for convenience only). The functions currently
  implemented are generic for binary search trees, and binary search
  trees in E are splay trees by now.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat May 30 17:55:39 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLB_AVLGENERIC

#define CLB_AVLGENERIC


#include <clb_pstacks.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// MACRO: AVL_TRAVERSE_DECLARATION()
//
//   Produce a declaration for AVL traversal functions. Name is the name
//   prefix to use for the functions, type is the pointer type used in
//   tree construction.
//
/----------------------------------------------------------------------*/

#define AVL_TRAVERSE_DECLARATION(name,type)\
PStack_p name##TraverseInit(type root);\
type     name##TraverseNext(PStack_p state);


/*-----------------------------------------------------------------------
//
// MACRO: AVL_TRAVERSE_DEFINITION()
//
//   Produce code for AVL traversal functions as follows:
//
//-----------------------------------------------------------------------
//
// Function: <name>TraverseInit()
//
//   Return a stack containing the path to the smallest element in the
//   avl tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
//---------------------------------------------------------------------
//
// Function: <name>TraverseNext()
//
//   Given a stack describing a traversal state, return the next node
//   and update the stack.
//
// Global Variables: -
//
// Side Effects    : Updates stack
//
/----------------------------------------------------------------------*/

#define AVL_TRAVERSE_DEFINITION(name,type)\
PStack_p name##TraverseInit(type root)\
{\
   PStack_p stack = PStackAlloc();\
\
   while(root)\
   {\
      PStackPushP(stack, root);\
      root = root->lson;\
   }\
   return stack;\
}\
\
\
type name##TraverseNext(PStack_p state)\
{\
   type handle, res;\
\
   if(PStackEmpty(state))\
   {\
      return NULL;\
   }\
   res = PStackPopP(state);\
   handle = res->rson;\
   while(handle)\
   {\
      PStackPushP(state, handle);\
      handle = handle->lson;\
   }\
   return res;\
}



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





