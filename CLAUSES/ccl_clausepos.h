/*-----------------------------------------------------------------------

File  : ccl_clausepos.h

Author: Stephan Schulz

Contents

  Positions of subterms in clauses (and in equations).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed May 20 03:34:54 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CLAUSEPOS

#define CLAUSEPOS

#include <cte_termpos.h>
#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



typedef struct clauseposcell
{
   Clause_p  clause;
   Eqn_p     literal;
   EqnSide   side;
   TermPos_p pos;
   void*     data;
}ClausePosCell, *ClausePos_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

typedef void (*Deleter)(void*);


#define ClausePosCellAlloc() (ClausePosCell*)SizeMalloc(sizeof(ClausePosCell))
#define ClausePosCellFree(junk)         SizeFree(junk, sizeof(ClausePosCell))
static inline void    ClausePosCellFreeWDeleter(ClausePos_p junk, Deleter del);

#ifdef CONSTANT_MEM_ESTIMATE
#define CLAUSEPOSCELL_MEM 20
#else
#define CLAUSEPOSCELL_MEM MEMSIZE(ClausePosCell)
#endif

static inline ClausePos_p ClausePosAlloc(void);
static inline void        ClausePosFreeWDeleter(ClausePos_p junk, Deleter deleter);
#define ClausePosFree(junk) ClausePosFreeWDeleter(junk, NULL)

static inline Term_p   ClausePosGetSide(ClausePos_p pos);
static inline Term_p   ClausePosGetOtherSide(ClausePos_p pos);
static inline Term_p   ClausePosGetSubterm(ClausePos_p pos);
#define ClausePosIsTop(position) ((PStackEmpty((position)->pos)))


void     ClausePosPrint(FILE* out, ClausePos_p pos);
Eqn_p    ClausePosFindPosLiteral(ClausePos_p pos, bool maximal);
Eqn_p    ClausePosFindMaxLiteral(ClausePos_p pos, bool positive);
Term_p   ClausePosFindFirstMaximalSide(ClausePos_p pos, bool positive);
Term_p   ClausePosFindNextMaximalSide(ClausePos_p pos, bool positive);
Term_p   ClausePosFindFirstMaximalSubterm(ClausePos_p pos);
Term_p   ClausePosFindNextMaximalSubterm(ClausePos_p pos);

bool     TermComputeRWSequence(PStack_p stack, Term_p from, Term_p to,
                               int inject_op);


/*---------------------------------------------------------------------*/
/*                Inline Functions                                     */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClausePosAlloc()
//
//   Allocate an empty, semi-initialized ClausePosCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline ClausePos_p ClausePosAlloc(void)
{
   ClausePos_p handle = ClausePosCellAlloc();

   handle->literal = NULL;
   handle->side    = LeftSide;
   handle->pos     = TermPosAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function:  ClausePosCellFreeWDeleter()
//
//   Free a clause pos cell and use deleter on junk->data
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline void ClausePosCellFreeWDeleter(ClausePos_p junk, Deleter deleter)
{
   assert(junk);

   if(deleter)
   {
      deleter(junk);
   }
   ClausePosCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function:  ClausePosFree()
//
//   Free a clausepos.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static inline void ClausePosFreeWDeleter(ClausePos_p junk, Deleter deleter)
{
   assert(junk);

   TermPosFree(junk->pos);
   ClausePosCellFree(junk);
   if(deleter)
   {
      deleter(junk->data);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosGetSide()
//
//   Given a clause position, return the designated side of the
//   literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p ClausePosGetSide(ClausePos_p pos)
{
   if(pos->side == LeftSide)
   {
      return pos->literal->lterm;
   }
   return pos->literal->rterm;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePosGetOtherSide()
//
//   Given a clause position, return the _not_ designated side of the
//   literal - don't ask, this has its use!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p ClausePosGetOtherSide(ClausePos_p pos)
{
   if(pos->side == LeftSide)
   {
      return pos->literal->rterm;
   }
   return pos->literal->lterm;
}



/*-----------------------------------------------------------------------
//
// Function: ClausePosGetSubterm()
//
//   Given a clause position, return the designated subterm of the
//   literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline Term_p ClausePosGetSubterm(ClausePos_p pos)
{
   return TermPosGetSubterm(ClausePosGetSide(pos), pos->pos);
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





