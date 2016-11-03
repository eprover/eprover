/*-----------------------------------------------------------------------

File  : ccl_g_lithash.h

Author: Stephan Schulz

Contents

  Algorithms and data structures implementing a simple literal
  indexing structure for implementing local unification constraints
  for the grounding procedure.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 20 15:26:11 CEST 2001
    New

-----------------------------------------------------------------------*/

#ifndef CCL_G_LITHASH

#define CCL_G_LITHASH

#include <ccl_clausesets.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct lit_desc_cell
{
   Term_p   lit; /* The actual literal (left hand side of E equation -
          we are only doing this for non-equational literals
       */
   Clause_p clause; /* If literal occurs in exactly one clause, note
             it here, otherwise this is 0. Note that only
             literals actually occurring in the clause set
             should be in the index */
}LitDescCell,*LitDesc_p;

/* Note that while we store shared terms, we do not take any refrences
   to them. Hence the literal hash is only guaranteed to be valid, as
   long as all the clauses contributing to it are in existance! */

typedef struct lit_hash_cell
{
   long     sig_size;   /* Largest symbol in sig */
   PTree_p  *pos_lits;  /* Array of PObjTrees for each predicate */
   PTree_p  *neg_lits;  /* symbol, literals are stored separated by
            sign */
}LitHashCell,*LitHash_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define LitDescCellAlloc()    (LitDescCell*)SizeMalloc(sizeof(LitDescCell))
#define LitDescCellFree(junk) SizeFree(junk, sizeof(LitDescCell))

int LitDescCompare(const void* lit1, const void* lit2);

#define LitHashCellAlloc()    (LitHashCell*)SizeMalloc(sizeof(LitHashCell))
#define LitHashCellFree(junk) SizeFree(junk, sizeof(LitHashCell))

LitHash_p LitHashAlloc(Sig_p sig);
void      LitHashFree(LitHash_p junk);

void      LitHashInsertEqn(LitHash_p hash, Eqn_p eqn, Clause_p clause);
void      LitHashInsertClause(LitHash_p hash, Clause_p clause);
void      LitHashInsertClauseSet(LitHash_p hash, ClauseSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





