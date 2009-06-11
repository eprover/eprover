/*-----------------------------------------------------------------------

File  : ccl_term_clause_assoc.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Datatype allowing the association of terms with sets of clauses
  (expected to be used to index clauses by their subterms, but who
   knows...)

  Copyright 2006 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sun Nov 12 16:50:45 ICT 2006
    New

-----------------------------------------------------------------------*/

#ifndef CCL_SUBTERMINDEX

#define CCL_SUBTERMINDEX



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct term_clauses_assoc
{
   Term_p  term;
   PTree_p clauses;
}TermClausesAssocCell, *TermClauseAssoc_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define TermClauseAssocCellAlloc()\
        (TermClauseAssocCell*)SizeMalloc(sizeof(TermClauseAssocCell))
#define TermClauseAssocCellFree(junk)\
        SizeFree(junk, sizeof(TermClauseAssocCell))

TermClauseAssoc_p TermClauseAssocAlloc(Term_p term);
void              TermClauseAssocFree(TermClauseAssoc_p junk);
bool              TermClauseAssocInsert(TermClauseAssoc_p assoc, Clause_p clause);
bool              TermClauseAssocDelete(Clause_p clause);





#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





