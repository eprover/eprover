/*-----------------------------------------------------------------------

File  : ccl_clausefunc.c

Author: Stephan Schulz

Contents

  Clause functions that need to know about sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Aug  7 00:02:44 CEST 2001
    New, partitioned ccl_clausesets.h

-----------------------------------------------------------------------*/


#include "ccl_clausefunc.h"


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
// Function: ClauseCanonCompareRef()
//
///  Compare two indirectly pointed to clauses with
//   ClauseStructWeightLexCompare().
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ClauseCanonCompareRef(const void *clause1ref, const void* clause2ref)
{
   const Clause_p* c1 = clause1ref;
   const Clause_p* c2 = clause2ref;

   return CMP(ClauseStructWeightLexCompare(*c1, *c2),0);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveLiteralRef()
//
//   Remove *lit from clause, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause and possibly clause->set->literals
//
/----------------------------------------------------------------------*/

void ClauseRemoveLiteralRef(Clause_p clause, Eqn_p *lit)
{
   Eqn_p handle = *lit;

   if(EqnIsPositive(handle))
   {
      clause->pos_lit_no--;
   }
   else
   {
      clause->neg_lit_no--;
   }
   if(clause->set)
   {
      clause->set->literals--;
   }
   clause->weight -= EqnStandardWeight(handle);
   EqnListDeleteElement(lit);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveLiteral()
//
//   Remove lit from clause, adjusting counters as necessary. This is
//   a lot less efficient then ClauseRemoveLiteralRef(), as we have to
//   search for the literal.
//
// Global Variables: -
//
// Side Effects    : Changes clause and possibly clause->set->literals
//
/----------------------------------------------------------------------*/

void ClauseRemoveLiteral(Clause_p clause, Eqn_p lit)
{
   EqnRef handle = &(clause->literals);

   while(*handle!=lit)
   {
      assert(*handle);
      handle = &((*handle)->next);
   }
   ClauseRemoveLiteralRef(clause, handle);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseFlipLiteralSign()
//
//   Change the sign of lit, adjusting counters as necessary.
//
// Global Variables: -
//
// Side Effects    : Changes clause.
//
/----------------------------------------------------------------------*/

void ClauseFlipLiteralSign(Clause_p clause, Eqn_p lit)
{
   if(EqnIsPositive(lit))
   {
      clause->pos_lit_no--;
      clause->neg_lit_no++;
   }
   else
   {
      clause->neg_lit_no--;
      clause->pos_lit_no++;
   }
   EqnFlipProp(lit, EPIsPositive);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseKillChildren()
//
//   Remove and delete children of a clause.
//
// Global Variables: -
//
// Side Effects    : Changes term sets
//
/----------------------------------------------------------------------*/

void ClauseKillChildren(Clause_p clause)
{
   Clause_p current;

   while(clause->children)
   {
      current = clause->children->key;
      assert(clause == current->parent1 || clause == current->parent2);
      /* printf("ClauseKillChildren(%d) -> %d\n",
    (int)clause,(int)current); */
      ClauseDetachParents(current);
      if(!ClauseQueryProp(current, CPIsProtected))
      {
    ClauseSetDeleteEntry(current);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveSuperfluousLiterals()
//
//   Remove duplicate and trivial negative literals from the
//   clause. Return number of removed literals.
//
// Global Variables: -
//
// Side Effects    : Changes clause, termbanks
//
/----------------------------------------------------------------------*/

int ClauseRemoveSuperfluousLiterals(Clause_p clause)
{
   Eqn_p handle;
   int   removed = 0;

   assert(!ClauseIsAnyPropSet(clause, CPIsDIndexed|CPIsSIndexed));

   removed += EqnListRemoveResolved(&(clause->literals));
   removed += EqnListRemoveDuplicates(clause->literals);

   if(removed)
   {
      clause->neg_lit_no = 0;
      clause->pos_lit_no = 0;
      handle = clause->literals;
      ClauseDelProp(clause, CPInitial|CPLimitedRW);

      while(handle)
      {
    if(EqnIsPositive(handle))
    {
       clause->pos_lit_no++;
    }
    else
    {
       clause->neg_lit_no++;
    }
    handle = handle->next;
      }

      if(clause->set)
      {
    clause->set->literals-=removed;
      }
   }
   if(removed)
   {
      ClausePushDerivation(clause, DCNormalize, NULL, NULL);
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetRemoveSuperflousLiterals()
//
//   For all clauses in set remove the trivial and duplicated
//   literals. Return number of literals removed.
//
// Global Variables: -
//
// Side Effects    : Only as described
//
/----------------------------------------------------------------------*/

long ClauseSetRemoveSuperfluousLiterals(ClauseSet_p set)
{
   Clause_p handle;
   long res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
     handle->succ)
   {
      res += ClauseRemoveSuperfluousLiterals(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCanonize()
//
//   Canonize a clause set by canonizing all
//   clauses, and sorting them in the order defined by
//   ClauseStructWeightLexCompare().
//
// Global Variables: -
//
// Side Effects    : Memory usage.
//
/----------------------------------------------------------------------*/

void ClauseSetCanonize(ClauseSet_p set)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!= set->anchor;
       handle = handle->succ)
   {
      ClauseRemoveSuperfluousLiterals(handle);
      ClauseCanonize(handle);
   }
   ClauseSetSort(set, ClauseCanonCompareRef);

   /* printf("Canonized: \n");
      ClauseSetPrint(stdout, set, true); */
}

/*-----------------------------------------------------------------------
//
// Function: ClauseRemoveACResolved()
//
//   Remove AC-resolved literals.
//
// Global Variables: -
//
// Side Effects    : As above
//
/----------------------------------------------------------------------*/

int ClauseRemoveACResolved(Clause_p clause)
{
   int   removed = 0;
   Sig_p sig;

   if(clause->neg_lit_no==0)
   {
      return 0;
   }
   sig = clause->literals->bank->sig;
   removed += EqnListRemoveACResolved(&(clause->literals));
   clause->neg_lit_no -= removed ;
   if(removed)
   {
      ClauseDelProp(clause, CPInitial|CPLimitedRW);
      DocClauseModification(GlobalOut, OutputLevel, clause,
             inf_ac_resolution, NULL, sig, NULL);
      ClausePushACResDerivation(clause, sig);
   }
   if(clause->set)
   {
      clause->set->literals-=removed;
   }
   return removed;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseUnitSimplifyTest()
//
//   Return true if clause can be simplified by a top-simplify-reflect
//   step with the (non-orientable) unit clause simplifier.
//
// Global Variables: -
//
// Side Effects    : Changes clause and values in set
//
/----------------------------------------------------------------------*/

bool ClauseUnitSimplifyTest(Clause_p clause, Clause_p simplifier)
{
   bool positive,tmp;
   EqnRef handle;
   Eqn_p  simpl;

   assert(ClauseIsUnit(simplifier));
   simpl = simplifier->literals;
   assert(EqnIsNegative(simpl)||!EqnIsOriented(simpl));

   positive = EqnIsPositive(simpl);

   if(EQUIV(positive, ClauseIsPositive(clause)))
   {
      return 0;
   }

   handle = &(clause->literals);

   while(*handle)
   {
      tmp = EqnIsPositive(*handle);
      if(XOR(positive,tmp)&&EqnSubsumeP(simpl,*handle))
      {
    return true;
      }
      handle = &((*handle)->next);
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseArchive()
//
//   Create an archive copy of clause in archive. The
//   archive copy inherits info and derivation. The original loses
//   info, and gets a new derivation that points to the archive
//   copy. Returns pointer to the archived copy.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseArchive(ClauseSet_p archive, Clause_p clause)
{
   Clause_p archclause;

   assert(archive);
   assert(clause);

   archclause = ClauseFlatCopy(clause);

   archclause->info = clause->info;
   archclause->derivation = clause->derivation;
   clause->info       = NULL;
   clause->derivation = NULL;
   ClausePushDerivation(clause, DCCnfQuote, archclause, NULL);
   ClauseSetInsert(archive, archclause);

   return archclause;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetArchive()
//
//   Create an archive copy of each clause in set in archive. The
//   archive copy inherits info and derivation. The original loses
//   info, and gets a new derivation that points to the archive copy.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseSetArchive(ClauseSet_p archive, ClauseSet_p set)
{
   Clause_p handle;

   assert(archive);
   assert(set);

   for(handle = set->anchor->succ; handle!= set->anchor;
       handle = handle->succ)
   {
      ClauseArchive(archive, handle);
   }
}



/*-----------------------------------------------------------------------
//
// Function: PStackClausePrint()
//
//   Print the clauses on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void PStackClausePrint(FILE* out, PStack_p stack, char* extra)
{
   PStackPointer i;
   Clause_p clause;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      clause = PStackElementP(stack, i);
      ClausePrint(out, clause, true);
      if(extra)
      {
         fprintf(out, "%s", extra);
      }
      fputc('\n', out);
   }
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






