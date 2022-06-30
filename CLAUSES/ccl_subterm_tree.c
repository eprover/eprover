/*-----------------------------------------------------------------------

File  : ccl_subterm_tree.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  A tree-based mapping mapping subterms to occurrences in clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_subterm_tree.h"



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
// Function: subterm_occ_free_wrapper()
//
//   Wrapper of type ObjFreeFun.
//
// Global Variables: -
//
// Side Effects    : Via SubtermOccFree()
//
/----------------------------------------------------------------------*/

static void subterm_occ_free_wrapper(void *junk)
{
   SubtermOccFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: subterm_pos_free_wrapper()
//
//   Wrapper of type ObjFreeFun.
//
// Global Variables: -
//
// Side Effects    : Via SubtermOccFree()
//
/----------------------------------------------------------------------*/

static void subterm_pos_free_wrapper(void *junk)
{
   SubtermPosFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: subterm_tree_print_dot()
//
//   Print a subterm tree in dot notation.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void subterm_tree_print_dot(FILE* out, SubtermTree_p root, Sig_p sig)
{
   SubtermOcc_p data;

   if(root)
   {
      data = root->key;
      fprintf(out, "     t%p [label=\"<l>|", root);
      TermPrint(out, data->term, sig, DEREF_ALWAYS);
      fprintf(out, "|<r>\"]\n");
      if(root->lson)
      {
         subterm_tree_print_dot(out, root->lson, sig);
         fprintf(out, "     t%p:l -- t%p\n", root, root->lson);
      }
      if(root->rson)
      {
         subterm_tree_print_dot(out, root->rson, sig);
         fprintf(out, "     t%p:r -- t%p\n", root, root->rson);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: SubtermOccAlloc()
//
//   Allocate an initialized Subterm-Occurrence-Cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SubtermOcc_p SubtermOccAlloc(Term_p term)
{
   SubtermOcc_p handle = SubtermOccCellAlloc();

   handle->term    = term;
   handle->pl.occs.rw_rest = NULL;
   handle->pl.occs.rw_full = NULL;
   handle->pl.pos.clauses  = NULL;
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermOccFree()
//
//   Free a Subterm-Occurrence-Cell
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermOccFree(SubtermOcc_p soc)
{
   PTreeFree(soc->pl.occs.rw_rest);
   PTreeFree(soc->pl.occs.rw_full);
   SubtermOccCellFree(soc);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermPosFree()
//
//   Free a Subterm-Occurrence-Cell with clause positions.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermPosFree(SubtermOcc_p soc)
{
   ClauseTPosTreeFree(soc->pl.pos.clauses);
   SubtermOccCellFree(soc);
}




/*-----------------------------------------------------------------------
//
// Function: CmpSubtermCells()
//
//   Compare two SubtermOccurrence cells via their term pointers. This
//   is a synthetic but machine-independent measure useful primarily
//   for indexing.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int CmpSubtermCells(const void *soc1, const void *soc2)
{
   const SubtermOcc_p s1 = (const SubtermOcc_p) soc1;
   const SubtermOcc_p s2 = (const SubtermOcc_p) soc2;

   return PCmp(s1->term, s2->term);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermBWTreeFree()
//
//   Free a subterm tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermBWTreeFree(SubtermTree_p root)
{
   PObjTreeFree(root, subterm_occ_free_wrapper);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermBWTreeFreeWrapper()
//
//   Free a subterm tree, with proper signature for FPIndexFree().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermBWTreeFreeWrapper(void *junk)
{
   SubtermBWTreeFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermOLTreeFree()
//
//   Free a subterm tree.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermOLTreeFree(SubtermTree_p root)
{
   PObjTreeFree(root, subterm_pos_free_wrapper);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermOLTreeFreeWrapper()
//
//   Free a subterm tree, with proper signature for FPIndexFree().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermOLTreeFreeWrapper(void *junk)
{
   SubtermOLTreeFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: SubtermTreeInsertTerm()
//
//   Return the SubtermOccNode corresponding to term, creating it if it
//   does not exist.
//
// Global Variables: -
//
// Side Effects    : Memory  operations
//
/----------------------------------------------------------------------*/

SubtermOcc_p SubtermTreeInsertTerm(SubtermTree_p *root, Term_p term)
{
   SubtermOcc_p old, newnode = SubtermOccAlloc(term);

   old = PTreeObjStore(root, newnode, CmpSubtermCells);
   if(old)
   {
      SubtermOccFree(newnode);
      newnode = old;
   }
   return newnode;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermTreeFind()
//
//   Find and return tree node with key term. Return it or NULL if no
//   such node exists.
//
// Global Variables: -
//
// Side Effects    : Rearranges tres
//
/----------------------------------------------------------------------*/

SubtermOcc_p SubtermTreeFindTerm(SubtermTree_p *root, Term_p term)
{
   SubtermOcc_p found, key = SubtermOccAlloc(term);

   found = PTreeObjFindObj(root, key,
                           CmpSubtermCells);
   SubtermOccFree(key);
   return found;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermTreeDeleteTerm()
//
//   Delete the SubtermOccNode corresponding to term,
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SubtermTreeDeleteTerm(SubtermTree_p *root, Term_p term)
{
   SubtermOcc_p old, knode = SubtermOccAlloc(term);

   old = PTreeObjExtractObject(root, knode, CmpSubtermCells);
   SubtermOccFree(old);
   SubtermOccFree(knode);
}



/*-----------------------------------------------------------------------
//
// Function: SubtermTreeInsertTermOcc()
//
//   Insert a term occurrence into the Subterm tree. Return false if an
//   entry already exists, true otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool SubtermTreeInsertTermOcc(SubtermTree_p *root, Term_p term,
                              Clause_p clause, bool restricted)
{
   SubtermOcc_p handle = SubtermTreeInsertTerm(root, term);

   if(restricted)
   {
      return PTreeStore(&(handle->pl.occs.rw_rest), clause);
   }
   return PTreeStore(&(handle->pl.occs.rw_full), clause);
}



/*-----------------------------------------------------------------------
//
// Function: SubtermTreeDeleteTermOcc()
//
//   Delete an indexing of clause via term.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool SubtermTreeDeleteTermOcc(SubtermTree_p *root, Term_p term,
                              Clause_p clause, bool restricted)
{
   SubtermOcc_p old, knode = SubtermOccAlloc(term);
   SubtermTree_p oldnode;
   bool res = false;

   oldnode = PTreeObjFind(root, knode, CmpSubtermCells);
   if(oldnode)
   {
      old = oldnode->key;
      if(restricted)
      {
         res = PTreeDeleteEntry(&(old->pl.occs.rw_rest), clause);
      }
      else
      {
         res = PTreeDeleteEntry(&(old->pl.occs.rw_full), clause);
      }
      if((old->pl.occs.rw_rest == NULL) && (old->pl.occs.rw_full == NULL))
      {
         SubtermTreeDeleteTerm(root, term);
      }
   }
   SubtermOccFree(knode);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SubtermTreePrint()
//
//   Print a suberm tree (only for debugging)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SubtermTreePrint(FILE* out, SubtermTree_p root, Sig_p sig)
{
   SubtermOcc_p data;

   if(root)
   {
      SubtermTreePrint(out, root->lson, sig);
      data = root->key;
      fprintf(out, "Node: %p data=%p\n", root, data);
      fprintf(out, "Key: %ld = ", data->term->entry_no);
      TermPrint(out, data->term, sig, DEREF_ALWAYS);
      fprintf(out, "\nlson=%p, rson=%p\n\n", root->lson, root->rson);
      SubtermTreePrint(out, root->rson, sig);
   }
}

/*-----------------------------------------------------------------------
//
// Function: SubtermTreePrintDot()
//
//   Print a suberm tree as a subgraph in Dot notation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SubtermTreePrintDot(FILE* out, SubtermTree_p root, Sig_p sig)
{
   fprintf(out,
           "     subgraph g%p{\n"
           "     nodesep=0.05\n"
           "     node [shape=record,width=1.9,height=.1, penwidth=0,"
           " style=filled, fillcolor=gray80]\n",
           root);
#ifdef PRT_SUBTERM_SET_AS_TREE
   subterm_tree_print_dot(out, root, sig);
#else
   {
      PStack_p terms = PStackAlloc();
      PStackPointer i;
      char* sep="";
      SubtermOcc_p data;

      PTreeToPStack(terms, root);
      fprintf(out, "     t%p [label=\"{|{", root);
      for(i=0; i<PStackGetSP(terms); i++)
      {
         data = PStackElementP(terms, i);
         fprintf(out, "%s", sep);
         sep = "|";
         TermPrint(out, data->term, sig, DEREF_ALWAYS);
      }
      fprintf(out, "}}\"]\n");
      PStackFree(terms);
   }
#endif
   fprintf(out, "     }\n");
}

/*-----------------------------------------------------------------------
//
// Function: SubtermTreePrintDummy()
//
//   Print subterm trees as "..."
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SubtermTreePrintDummy(FILE* out, SubtermTree_p root, Sig_p sig)
{
   fprintf(out, "     t%p [shape=box label=\"%ld terms\"]\n", root, PObjTreeNodes(root));
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


