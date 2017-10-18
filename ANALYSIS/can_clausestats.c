/*-----------------------------------------------------------------------

File  : can_clausestats.c

Author: Stephan Schulz

Contents

  Simple data structure for handling a fixed elements vector of
  statistics.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 31 15:18:15 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "can_clausestats.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ClauseStatsCopy()
//
//   Create a copy of the clausestats-cell.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

ClauseStats_p ClauseStatsCopy(ClauseStats_p stats)
{
   ClauseStats_p handle = ClauseStatsCellAlloc();

   handle->proof_distance  = stats->proof_distance;
   handle->simplify_used   = stats->simplify_used;
   handle->simplify_unused = stats->simplify_unused;
   handle->generate_used   = stats->generate_used;
   handle->generate_unused = stats->generate_unused;
   handle->subsumed        = stats->subsumed;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStatsParseInto()
//
//   Parse the statistics into a statistics-cell.
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

void ClauseStatsParseInto(Scanner_p in, ClauseStats_p cell)
{
   assert(cell);

   AcceptInpTok(in, OpenBracket);
   cell->proof_distance = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Comma);
   cell->simplify_used = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Comma);
   cell->simplify_unused = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Comma);
   cell->generate_used = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Comma);
   cell->generate_unused = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Comma);
   cell->subsumed = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, CloseBracket);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStatsParse()
//
//   Parse a ClauseStats sequence and return a cell describing it.
//
// Global Variables: -
//
// Side Effects    : Memory operations, reads input
//
/----------------------------------------------------------------------*/

ClauseStats_p ClauseStatsParse(Scanner_p in)
{
   ClauseStats_p cell = ClauseStatsCellAlloc();
   ClauseStatsParseInto(in, cell);
   return cell;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStatsPrint()
//
//   Print a ClauseStats sequence.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseStatsPrint(FILE *out, ClauseStats_p cell)
{
   assert(cell);

   fprintf(out, "(%2ld,%3ld,%3ld,%3ld,%3ld, %3ld)",
      cell->proof_distance,
      cell->simplify_used,
      cell->simplify_unused,
      cell->generate_used,
      cell->generate_unused,
      cell->subsumed);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseStatsPrintNormalized()
//
//   Print clausestats in relation to the proof search size.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseStatsPrintNormalized(FILE *out, ClauseStats_p cell, long
            created, long processed)
{
   double rel_simplify_used,
          rel_simplify_unused,
          rel_generate_used,
          rel_generate_unused,
          rel_subsumed;

   assert(cell);
   assert(created >= 0);
   assert(processed >= 0);

   if(created)
   {
      rel_simplify_unused = cell->simplify_unused/(double)created;
      rel_generate_unused = cell->generate_unused/(double)created;
   }
   else
   {
      assert(cell->simplify_unused == 0);
      assert(cell->generate_unused == 0);
      rel_simplify_unused = 0;
      rel_generate_unused = 0;
   }
   if(processed)
   {
      rel_simplify_used = cell->simplify_used/(double)processed;
      rel_generate_used = cell->generate_used/(double)processed;
      rel_subsumed      = cell->subsumed/(double)processed;
   }
   else
   {
      assert(cell->simplify_used == 0);
      assert(cell->generate_used == 0);
      assert(cell->subsumed == 0);
      rel_simplify_used = 0;
      rel_generate_used = 0;
      rel_subsumed      = 0;
   }
   fprintf(out, "(%3ld,%f,%f,%f,%f,%f)",
      cell->proof_distance,
      rel_simplify_used,
      rel_simplify_unused,
      rel_generate_used,
      rel_generate_unused,
      rel_subsumed);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetInfoParse()
//
//   Parse a list of information about clauses in the form
//   ident:(a,b,c,d,e, f) * and store it into a NumTree, indexed by
//   ident. Returns number of entries parsed.
//
// Global Variables: -
//
// Side Effects    : Memory operations, reads input
//
/----------------------------------------------------------------------*/

long ClauseSetInfoParse(Scanner_p in, NumTree_p *tree)
{
   long   ident, count = 0;
   IntOrP tmp;
   ClauseStats_p cell;

   while(TestInpTok(in, PosInt)&&TestTok(LookToken(in,1), Colon))
   {
      ident = AktToken(in)->numval;
      AcceptInpTok(in, PosInt);
      AcceptInpTok(in, Colon);
      cell = ClauseStatsParse(in);
      tmp.p_val = cell;
      NumTreeStore(tree, ident, tmp, tmp);
      count++;
   }
   return count;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetInfoPrint()
//
//   Print a list of information about clauses in the form
//   ident:(a,b,c,d,e,f) * from a NumTree, indexed by
//   ident. Print number of entries printed.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

long ClauseSetInfoPrint(FILE* out, NumTree_p tree)
{
   long   ident, count = 0;
   ClauseStats_p stats;
   PStack_p stack;
   NumTree_p handle;

   stack = NumTreeTraverseInit(tree);

   while((handle = NumTreeTraverseNext(stack)))
   {
      count++;
      ident = handle->key;
      stats = handle->val1.p_val;
      fprintf(out, "%ld:", ident);
      ClauseStatsPrint(out, stats);
      fputc('\n', out);
   }
   NumTreeTraverseExit(stack);

   return count;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseStatTreeFree()
//
//   Free a numtree with clause statistics information.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseStatTreeFree(NumTree_p tree)
{
   PStack_p  stack;
   NumTree_p cell;

   stack = NumTreeTraverseInit(tree);
   while((cell = NumTreeTraverseNext(stack)))
   {
      ClauseStatsCellFree(cell->val1.p_val);
   }
   NumTreeTraverseExit(stack);
   NumTreeFree(tree);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


