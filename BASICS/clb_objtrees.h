/*-----------------------------------------------------------------------

File  : clb_objtrees.h

Author: Stephan Schulz

Contents

  Data structures for the efficient management of objects represented
  by pointers. This inherits the ptree structure, but uses comparison
  on objects (by a user-provided comparison function) instead of pointer
  comparisons.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Feb 15 14:10:06 MET 1999
    Borrowed from clb_ptrees.[ch]

-----------------------------------------------------------------------*/

#ifndef CLB_OBJTREES

#define CLB_OBJTREES

#include <clb_ptrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef void (*ObjDelFun)(void *junk);

typedef PTree_p PObjTree_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PObjTree_p PTreeObjInsert(PObjTree_p *root, PObjTree_p newnode,
                          ComparisonFunctionType cmpfun);
void*      PTreeObjStore(PObjTree_p *root, void* key,
                         ComparisonFunctionType cmpfun);
PObjTree_p PTreeObjFind(PObjTree_p *root, void* key,
                        ComparisonFunctionType cmpfun);
void*      PTreeObjFindObj(PObjTree_p *root, void* key,
                           ComparisonFunctionType cmpfun);
PObjTree_p PTreeObjFindBinary(PObjTree_p root, void* key,
                              ComparisonFunctionType cmpfun);
PObjTree_p PTreeObjExtractEntry(PObjTree_p *root, void* key,
                                ComparisonFunctionType cmpfun);
void*      PTreeObjExtractObject(PObjTree_p *root, void* key,
                                 ComparisonFunctionType cmpfun);
void*      PTreeObjExtractRootObject(PObjTree_p *root,
                                     ComparisonFunctionType cmpfun);
void       PTreeObjMerge(PObjTree_p *root, PObjTree_p add,
                         ComparisonFunctionType cmpfun);

void       PObjTreeFree(PObjTree_p root, ObjDelFun del_fun);

long       PObjTreeNodes(PObjTree_p root);

void       DummyObjDelFun(void* Junk);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
