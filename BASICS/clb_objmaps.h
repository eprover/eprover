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

#include <clb_pstacks.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef void (*KeyValDelFun)(void *key, void* val);

struct objmap_node;
typedef struct objmap_node* PObjMap_p;

PStack_p PObjMapTraverseInit(PObjMap_p, PStack_p);
void*    PObjMapTraverseNext(PStack_p, void**);
#define  PObjMapTraverseExit(stack) (assert(stack), PStackReset(stack))


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
void*      PObjMapStore(PObjMap_p *root, void* key, void* value,
                        ComparisonFunctionType cmpfun);
void**     PObjMapGetRef(PObjMap_p *root, void* key,
                         ComparisonFunctionType cmpfun, bool* updated);
void*      PObjMapFind(PObjMap_p *root, void* key,
                       ComparisonFunctionType cmpfun);
void*      PObjMapExtract(PObjMap_p *root, void* key,
                          ComparisonFunctionType cmpfun);
void       PObjMapFreeWDeleter(PObjMap_p root, KeyValDelFun del_fun);
void       PObjMapFree(PObjMap_p root);
size_t     SizeOfPObjNode();
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
