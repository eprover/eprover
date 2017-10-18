/*-----------------------------------------------------------------------

File  : clb_floattrees.h

Author: Stephan Schulz

Contents

  Definitions for SPLAY trees with long integer keys and up to two
  long or pointer values. Copied from clb_stringtrees.h

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Aug 21 11:55:40 GMT 1999
    Stilen from clb_numtrees.h

-----------------------------------------------------------------------*/

#ifndef CLB_FLOATTREES

#define CLB_FLOATTREES

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/



/* General purpose data structure for indexing objects by a (double)
   float  key. Integer values are supported directly, for all other objects
   pointers can be used (and need to be casted carefully by the
   wrapper functions). Objects pointed to by the value fields are not
   part of the data stucture and will not be touched by deallocating
   trees or tree nodes. */

typedef struct floattreecell
{
   double               key;
   IntOrP               val1;
   IntOrP               val2;
   struct floattreecell *lson;
   struct floattreecell *rson;
}FloatTreeCell, *FloatTree_p;


#define FloatTreeCellAlloc() (FloatTreeCell*)SizeMalloc(sizeof(FloatTreeCell))
#define FloatTreeCellFree(junk)        SizeFree(junk, sizeof(FloatTreeCell))

FloatTree_p FloatTreeCellAllocEmpty(void);
void        FloatTreeFree(FloatTree_p junk);
FloatTree_p FloatTreeInsert(FloatTree_p *root, FloatTree_p new);
bool        FloatTreeStore(FloatTree_p *root, double key, IntOrP val1,
            IntOrP val2);
FloatTree_p FloatTreeFind(FloatTree_p *root, double key);
FloatTree_p FloatTreeExtractEntry(FloatTree_p *root, double key);
bool        FloatTreeDeleteEntry(FloatTree_p *root, double key);
long        FloatTreeNodes(FloatTree_p root);

AVL_TRAVERSE_DECLARATION(FloatTree, FloatTree_p)
#define FloatTreeTraverseExit(stack) PStackFree(stack)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





