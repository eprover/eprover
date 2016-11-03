/*-----------------------------------------------------------------------

File  : ccl_splitting.h

Author: Stephan Schulz

Contents

  Implements functions for destructive splitting of clauses with at
  least two non-propositional variable disjoint subsets of literals.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 18 18:24:18 MET DST 2001
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SPLITTING

#define CCL_SPLITTING

#include <ccl_def_handling.h>


typedef enum
{
   SplitGroundNone = 0, /* Combine ground literals with first
            subclause */
   SplitGroundOne  = 1, /* Split off (at most) _one_ ground subclause
            containing all ground literals */
   SplitGroundFull = 2  /* Split off individual ground literals */
}SplitType;


/* Describes which clauses should be split. Note that this currently
   is not orthogonal at all. The functions using this are in
   CONTROL/cco_clausesplitting.c, but we need the data type for the
   control block later on. */

typedef enum
{
   SplitNone        = 0,
   SplitHorn        = 1,
   SplitNonHorn     = 2,
   SplitNegative    = 4,
   SplitPositive    = 8,
   SplitMixed       = 16,
   SplitAll         = 7
}SplitClassType;

#define QuerySplitClass(var, prop) ((var)&(prop))
#define SetSplitClass(var, prop) (var) = ((var)|(prop))



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct lit_split_desc
{
   Eqn_p   literal;
   int     part;
   PTree_p varset;  /* Initially: Varset of literal, later either
             empty (in subsequent literals) or varset of
             partition (for first literal of partition) */
}LitSplitDescCell, *LitSplitDesc_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

bool ClauseHasSplitLiteral(Clause_p clause);
int  ClauseSplit(DefStore_p store, Clause_p clause, ClauseSet_p set,
                 SplitType how, bool fresh_defs);

long ClauseSetSplitClauses(DefStore_p store, ClauseSet_p from_set,
                           ClauseSet_p to_set, SplitType how,
                           bool fresh_defs);

long ClauseSetSplitClausesGeneral(DefStore_p store, bool fresh_defs,
                                  ClauseSet_p from_set,
                                  ClauseSet_p to_set, long tries);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





