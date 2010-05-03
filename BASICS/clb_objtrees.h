/*-----------------------------------------------------------------------

File  : clb_objtrees.h

Author: Stephan Schulz

Contents
 
  Data structures for the efficient management of objects represented
  by pointers. This inherits the ptree structure, but uses comparison
  on objects (by a user-provided comparison function) instead of pointer
  comparisons.

  Copyright 1998-2010 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PTree_p PTreeObjInsert(PTree_p *root, PTree_p newnode,
		       ComparisonFunctionType cmpfun);
void*   PTreeObjStore(PTree_p *root, void* key,
		      ComparisonFunctionType cmpfun);
PTree_p PTreeObjFind(PTree_p *root, void* key,
		     ComparisonFunctionType cmpfun);
PTree_p PTreeObjFindBinary(PTree_p root, void* key, 
			   ComparisonFunctionType cmpfun);
PTree_p PTreeObjExtractEntry(PTree_p *root, void* key,
			     ComparisonFunctionType cmpfun);
void*   PTreeObjExtractObject(PTree_p *root, void* key,
			      ComparisonFunctionType cmpfun);
void*   PTreeObjExtractRootObject(PTree_p *root,
				  ComparisonFunctionType cmpfun);
void    PTreeObjMerge(PTree_p *root, PTree_p add,
		      ComparisonFunctionType cmpfun);

void    PObjTreeFree(PTree_p root, ObjDelFun del_fun);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





