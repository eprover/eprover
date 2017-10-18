/*-----------------------------------------------------------------------

File  : ccl_pdtrees.h

Author: Stephan Schulz

Contents

  Perfect discrimination trees for optimized rewriting and
  subsumption. PDTrees are machines and have a state - each new search
  must initialize a tree to a consistent state, and only one search
  may be conducted at any given time.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun 22 17:04:32 MET DST 1998
    New
<2> Fri Mar  2 16:06:12 CET 2001
    Completely rewritten

-----------------------------------------------------------------------*/

#ifndef CCL_PDTREES

#define CCL_PDTREES

#include <clb_intmap.h>
#include <ccl_clausepos.h>
#include <clb_simple_stuff.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* A node in the perfect discrimination tree... */

typedef struct pdt_node_cell
{
   IntMap_p           f_alternatives;   /* Function symbols */
   PDArray_p          v_alternatives;   /* Variables */
   FunCode            max_var;          /* Largest variable... */
   long               size_constr;      /* Only terms that have at
                  least this weight are
                  indexed at or beyond this
                  node */
   SysDate            age_constr;       /* Only clauses that are older
                  than this date are indexed
                  at or beyond this node */
   struct pdt_node_cell *parent;        /* Back-pointer to next node
                  towards the root */
   long               ref_count;        /* How many entries share this
                  node? */
   PTree_p            entries;          /* Clauses that are indexed
                  - this should be NULL at
                  all but leaf nodes. */
   Term_p             variable;         /* If this  node corresponds
                  to a variable, point to it
                  (so that we can bind it
                  while searching for
                  matches) */
   bool               bound;            /* Did we bind a variable (in
                  fact, the one above...) to
                  reach this node? I.e. do we
                  need to backtrack this
                  binding if we backtrack
                  over this node? */
   FunCode            trav_count;       /* For traversing during
                  matching. Both 0 and
                  node->max_var+1 represent
                  the (maximal one) function
                  symbol alternative, i is
                  variable i. */
}PDTNodeCell, *PDTNode_p;

/* A PDTreeCell is an object encapsulating a PDTree and the necessary
   data structures to efficiently seach it */

typedef struct pd_tree_cell
{
   PDTNode_p tree;
   PStack_p  term_stack;     /* For flattening the term */
   PStack_p  term_proc;      /* Store traversed terms for backtracking */
   PDTNode_p tree_pos;       /* For traversing the tree */
   PStack_p  store_stack;    /* For traversing entries in leaves */
   Term_p    term;           /* ...used as a key during search */
   SysDate   term_date;      /* Temporarily bound during matching */
   long      term_weight;    /* Ditto */
   int       prefer_general; /* Ditto */
   long      node_count;     /* How many tree nodes? */
   long      clause_count;   /* How many clauses? */
   long      arr_storage_est;/* How much memory used by arrays? */
   unsigned  long match_count;   /* How often has the index been
               searched? */
   unsigned  long visited_count; /* How many nodes in the index have
               been visited? */
}PDTreeCell, *PDTree_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#ifdef PDT_COUNT_NODES
#define PDT_COUNT_INC(x) ((x)++)
extern unsigned long PDTNodeCounter;
#else
#define PDT_COUNT_INC(x)
#endif


#define PDNODE_FUN_INIT_ALT 8
#define PDNODE_FUN_GROW_ALT 6
#define PDNODE_VAR_INIT_ALT 4
#define PDNODE_VAR_GROW_ALT 4

#define  PDTREE_IGNORE_TERM_WEIGHT LONG_MAX
#define  PDTREE_IGNORE_NF_DATE     SysDateCreationTime()
#define  PDT_NODE_INIT_VAL(tree)   ((tree)->prefer_general)
#define  PDT_NODE_CLOSED(tree,node) ((tree)->prefer_general?\
                                    (((node)->max_var)+2):(((node)->max_var)+1))

#define   PDTreeCellAlloc()    (PDTreeCell*)SizeMalloc(sizeof(PDTreeCell))
#define   PDTreeCellFree(junk) SizeFree(junk, sizeof(PDTreeCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define PDTREE_CELL_MEM 16
#else
#define PDTREE_CELL_MEM MEMSIZE(PDTreeCell)
#endif

PDTree_p  PDTreeAlloc(void);
void      PDTreeFree(PDTree_p tree);

#ifdef CONSTANT_MEM_ESTIMATE
#define PDTNODE_MEM 52
#else
#define PDTNODE_MEM MEMSIZE(PDTNodeCell)
#endif

#define   PDTreeStorage(tree) \
          ((tree)\
          ?\
          ((tree)->node_count*PDTNODE_MEM\
           +(tree)->arr_storage_est\
           +(tree)->clause_count*(PDTREE_CELL_MEM+CLAUSEPOSCELL_MEM))\
          :\
           0)

extern bool PDTreeUseAgeConstraints;
extern bool PDTreeUseSizeConstraints;

#define PDTNodeGetSizeConstraint(node) ((node)->size_constr != -1 ? (node)->size_constr : pdt_compute_size_constraint((node)))
#define PDTNodeGetAgeConstraint(node) (!SysDateIsInvalid((node)->age_constr))? (node)->age_constr: pdt_compute_age_constraint((node))

#define   PDTNodeCellAlloc()    (PDTNodeCell*)SizeMalloc(sizeof(PDTNodeCell))
#define   PDTNodeCellFree(junk) SizeFree(junk, sizeof(PDTNodeCell))
PDTNode_p PDTNodeAlloc(void);
void      PDTNodeFree(PDTNode_p tree);

void      TermLRTraverseInit(PStack_p stack, Term_p term);
Term_p    TermLRTraverseNext(PStack_p stack);
Term_p    TermLRTraversePrev(PStack_p stack, Term_p term);

void      PDTreeInsert(PDTree_p tree, ClausePos_p demod_side);
long      PDTreeDelete(PDTree_p tree, Term_p term, Clause_p clause);

void      PDTreeSearchInit(PDTree_p tree, Term_p term, SysDate
            age_constr, bool prefer_general);
void      PDTreeSearchExit(PDTree_p tree);

PDTNode_p PDTreeFindNextIndexedLeaf(PDTree_p tree, Subst_p subst);

ClausePos_p PDTreeFindNextDemodulator(PDTree_p tree, Subst_p subst);

void PDTreePrint(FILE* out, PDTree_p tree);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






