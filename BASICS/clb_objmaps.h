/*-----------------------------------------------------------------------

File  : clb_objmaps.h

Author: Petar Vukmirovic

Contents

  Data structure for efficiently dealing with mapping a key
  to a value. You only need to provide a (total) comparison function
  on the keys and optionally a deleter function for keys.

Copyright 1998-2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> vr 23 jul 2021 16:47:42 CEST
    Built on top of objtrees.[ch]

-----------------------------------------------------------------------*/

#ifndef CLB_OBJMAPS

#define CLB_OBJMAPS

#include <clb_objtrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef void (*ObjDelFun)(void *junk);

typedef PObjTree_p PObjMap_p;

#define PObjMapTraverseInit(root)  PTreeTraverseInit(root)
#define PObjMapTraverseExit(stack) PStackFree(stack)

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
void*      PObjMapStore(PObjTree_p *root, void* key, void* value,
                        ComparisonFunctionType cmpfun);
void*      PObjMapFind(PObjTree_p *root, void* key,
                       ComparisonFunctionType cmpfun);
void*      PObjMapExtract(PObjTree_p *root, void* key,
                          ComparisonFunctionType cmpfun);
void       PObjMapFreeWDeleter(PObjTree_p root, ObjDelFun del_fun_key, ObjDelFun del_fun_val);
void       PObjMapFree(PObjTree_p root);
void*      PObjMapTraverseNext(PStack_p state);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
