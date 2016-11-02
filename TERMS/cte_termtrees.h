/*-----------------------------------------------------------------------

File  : cte_termtrees.h

Author: Stephan Schulz

Contents

  Functionality of term-top indexed trees (I found that I can
  cleanly separate this from the termbank stuff).

  There are two sets of funktions for the manangment of term trees in
  CLIB: Funktions operating only on the top cell, and functions descending
  the term structure. Top level functions implement a conventional AVL
  tree with key f_code.masked_properties.entry_nos_of_args and are
  implemented here, recursive versions are in cte_termbanks.[hc]

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Nov 27 19:12:51 MET 1997
    New
<2> Thu Jan 28 00:59:59 MET 1999
    Replaced AVL trees with Splay-Trees


-----------------------------------------------------------------------*/

#ifndef CTE_TERMTREES

#define CTE_TERMTREES

#include <cte_termfunc.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void   TermTreeFree(Term_p junk);
long   TermTopCompare(Term_p t1, Term_p t2);
Term_p TermTreeFind(Term_p *root, Term_p term);
Term_p TermTreeInsert(Term_p *root, Term_p term);
Term_p TermTreeExtract(Term_p *root, Term_p term);
bool   TermTreeDelete(Term_p *root, Term_p term);
void   TermTreeSetProp(Term_p root, TermProperties props);
void   TermTreeDelProp(Term_p root, TermProperties props);
long   TermTreeNodes(Term_p root);

AVL_TRAVERSE_DECLARATION(TermTree, Term_p)
#define TermTreeTraverseExit(stack) PStackFree(stack)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





