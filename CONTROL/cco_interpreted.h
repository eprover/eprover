/*-----------------------------------------------------------------------

File  : cco_interpreted.h

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Code for handling (some) interpreted symbols. Initially, this will
  only deal with answer predicates (some of which may be false in
  otherwise empty clauses). Once things have shaken out, I expect more
  general solutions here...

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Jun  4 22:53:09 CEST 2011
    New

-----------------------------------------------------------------------*/

#ifndef CCO_INTERPRETED

#define CCO_INTERPRETED


#include <ccl_proofstate.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/*
#define XXXCellAlloc()    (XXXCell*)SizeMalloc(sizeof(XXXCell))
#define XXXCellFree(junk) SizeFree(junk, sizeof(XXXCell))

XXX_p  XXXAlloc();
void   XXXFree(XXX_p junk);
*/


void ClausePrintAnswer(FILE* out, Clause_p clause, ProofState_p state);
int ClauseEvaluateAnswerLits(Clause_p clause);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
