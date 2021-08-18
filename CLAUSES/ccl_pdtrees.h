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

  Created: Mon Jun 22 17:04:32 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CCL_PDTREES

#define CCL_PDTREES

#include <clb_intmap.h>
#include <ccl_clausepos.h>
#include <clb_simple_stuff.h>
#include <clb_objmaps.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Information about how far are we in the traversal
   of neighbors of the current node */
typedef enum {
   TRAVERSING_SYMBOLS = 0,
   TRAVERSING_VARIABLES = 1,
   DONE = 2
} TraversalState;


/* A node in the perfect discrimination tree... */
typedef struct pdt_node_cell
{
   IntMap_p           f_alternatives;   /* Function symbols */
   PObjMap_p          v_alternatives;   /* Variables */
   PObjMap_p          db_alternatives;  /* DeBruijn alternatives */
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
   PStackPointer      prev_subst;       /* For backtracking, to make sure
                                           that we have a clean state when
                                           we start backtracking. */
   PStack_p           var_traverse_stack; /* For traversing during
                                             matching. Iterator through
                                             the variables stored in v_alternatives */
   int                trav_state;         /* For traversing during
                                             matching. Remembers how far we are */
   bool               leaf;    /* In HO inner nodes can store clauses,
                                  so we mark leaves explicitly -- an optimization */
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
   long      node_count;     /* How many tree nodes? */
   long      clause_count;   /* How many clauses? */
   long      arr_storage_est;/* How much memory used by arrays? */
   unsigned  long match_count;   /* How often has the index been
                                    searched? */
   unsigned  long visited_count; /* How many nodes in the index have
                                    been visited? */
   TB_p      bank;            /* When we make a prefix term, we want to
                                 make it shared */
   Deleter   deleter;         /* frees the extra data stored in ClausePos_p */
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
#define  PDT_NODE_INIT_VAL(tree)    (0)
#define  PDT_NODE_CLOSED(tree,node) (DONE)

#define   PDTreeCellAlloc()    (PDTreeCell*)SizeMalloc(sizeof(PDTreeCell))
#define   PDTreeCellFree(junk) SizeFree(junk, sizeof(PDTreeCell))


#ifdef CONSTANT_MEM_ESTIMATE
#define PDTREE_CELL_MEM 16
#else
#define PDTREE_CELL_MEM MEMSIZE(PDTreeCell)
#endif

PDTree_p  PDTreeAllocWDeleter(TB_p bank, Deleter deleter);
#define   PDTreeAlloc(bank) (PDTreeAllocWDeleter(bank, NULL))
void      PDTreeFree(PDTree_p tree);

#ifdef CONSTANT_MEM_ESTIMATE
#define PDTNODE_MEM 52
#else
#define PDTNODE_MEM MEMSIZE(PDTNodeCell)
#endif

#define   PDTreeStorage(tree)                   \
   ((tree)                                      \
    ?                                           \
    ((tree)->node_count*PDTNODE_MEM             \
     +(tree)->arr_storage_est                                           \
     +(tree)->clause_count*(PDTREE_CELL_MEM+CLAUSEPOSCELL_MEM))         \
    :                                                                   \
    0)

extern bool PDTreeUseAgeConstraints;
extern bool PDTreeUseSizeConstraints;

#define PDTNodeGetSizeConstraint(node) ((node)->size_constr != -1 ? (node)->size_constr : pdt_compute_size_constraint((node)))
#define PDTNodeGetAgeConstraint(node) (!SysDateIsInvalid((node)->age_constr))? (node)->age_constr: pdt_compute_age_constraint((node))

#define   PDTNodeCellAlloc()    (PDTNodeCell*)SizeMalloc(sizeof(PDTNodeCell))
#define   PDTNodeCellFree(junk) SizeFree(junk, sizeof(PDTNodeCell))
PDTNode_p PDTNodeAlloc(void);
void      PDTNodeFree(PDTNode_p tree, Deleter deleter);

void      TermLRTraverseInit(PStack_p stack, Term_p term);
Term_p    TermLRTraverseNext(PStack_p stack);
Term_p    TermLRTraversePrev(PStack_p stack, Term_p term);

bool      PDTreeInsert(PDTree_p tree, ClausePos_p demod_side);
bool      PDTreeInsertTerm(PDTree_p tree, Term_p term, 
                           ClausePos_p demod_side, bool store_data);
long      PDTreeDelete(PDTree_p tree, Term_p term, Clause_p clause);
PDTNode_p PDTreeMatchPrefix(PDTree_p tree, Term_p term,  
                            long* matched, long* remains);

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
