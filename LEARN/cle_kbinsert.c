/*-----------------------------------------------------------------------

File  : cle_kbinsert.c

Author: Stephan Schulz

Contents

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul 27 22:51:29 GMT 1999
    New

-----------------------------------------------------------------------*/

#include "cle_kbinsert.h"



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
// Function: ParseExampleClause()
//
//   Parse an example clause into an annotated term format. Return
//   clause as AnnoTerm or NULL if pattern-computation is to
//   expensive.
//
// Global Variables: -
//
// Side Effects    : Reads input
//
/----------------------------------------------------------------------*/

AnnoTerm_p ParseExampleClause(Scanner_p in, TB_p parse_terms, TB_p
               internal_terms, long ident)
{
   Term_p         clauserep, newrep, tmp;
   Clause_p       clause;
   Annotation_p   anno = AnnotationAlloc();
   long           i;
   double         value;
   PatternSubst_p subst;
   PStack_p       listrep;
   AnnoTerm_p     handle = NULL;

   AcceptInpTok(in,PosInt);
   AcceptInpTok(in,Colon);

   anno->key = ident;
   AcceptInpTok(in, OpenBracket);

   /* The 0th position of the annotation vector always has the number
      of original annotations merged into it -> as the annotation is
      newly created, it's always 1 here. */

   AnnotationCount(anno) = 1;

   /* The first value is special! -> In the annotation, it is used
      twice: For the number of proofs in which the clause was used
      (position 1) and, as in the example
      file, as the distance from the proof (position 2). */
   if(AktToken(in)->numval == 0)
   {
      DDArrayAssign(anno->val1.p_val, 1, 1);
   }
   else
   {
      DDArrayAssign(anno->val1.p_val, 1, 0);
   }
   DDArrayAssign(anno->val1.p_val, 2, (float)AktToken(in)->numval);

   AcceptInpTok(in, PosInt);

   i=3;
   while(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      value = ParseFloat(in);
      DDArrayAssign(anno->val1.p_val, i, value);
      i++;
   }
   anno->val2.i_val = i;

   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Colon);
   clause = ClauseParse(in, parse_terms);

   subst   = PatternDefaultSubstAlloc(parse_terms->sig);
   listrep = PStackAlloc();

   if(PatternClauseCompute(clause, &subst, &listrep))
   {
      clauserep = RecEncodeClauseListRep(parse_terms, listrep);
      tmp = PatternTranslateSig(clauserep, subst, parse_terms->sig,
            internal_terms->sig,
            internal_terms->vars);
      newrep = TBInsert(internal_terms, tmp, DEREF_NEVER);
      TermFree(tmp);
      /* TBDelete(parse_terms, clauserep); */
      handle = AnnoTermAlloc(newrep, anno);
   }
   else
   {
      AnnotationFree(anno);
   }
   ClauseFree(clause);
   PatternSubstFree(subst);
   PStackFree(listrep);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: KBAxiomsInsert()
//
//   Insert the example "name" into set and return the ident
//   assigned.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

long KBAxiomsInsert(ExampleSet_p set, ClauseSet_p axioms, Sig_p sig,
          char* name)
{
   ExampleRep_p handle = ExampleRepCellAlloc();

   handle->ident = set->count+1;
   handle->name  = SecureStrdup(name);
   handle->features = FeaturesAlloc();
   ComputeClauseSetNumFeatures(handle->features, axioms, sig);
   ExampleSetInsert(set, handle);

   return handle->ident;
}


/*-----------------------------------------------------------------------
//
// Function: KBParseExampleFile()
//
//   Parse an example file into the existing structures.
//
// Global Variables: -
//
// Side Effects    : Changes everything ;-)
//
/----------------------------------------------------------------------*/

void KBParseExampleFile(Scanner_p in, char* name, ExampleSet_p set,
         AnnoSet_p examples, Sig_p res_sig)
{
   TB_p        terms;
   ClauseSet_p axioms = ClauseSetAlloc();
   long        ident;
   AnnoTerm_p  handle;
   TypeBank_p  sort_table = TypeBankAlloc();

   terms = TBAlloc(SigAlloc(sort_table));
   ClauseSetParseList(in, axioms, terms);

   ident = KBAxiomsInsert(set, axioms, terms->sig, name);
   ClauseSetFree(axioms);
   SigFree(terms->sig);
   terms->sig = NULL;
   TBFree(terms);

   AcceptInpTok(in, Fullstop);

   terms = TBAlloc(res_sig);

   while(!TestInpTok(in, NoToken))
   {
      handle = ParseExampleClause(in, terms, examples->terms, ident);
      if(handle)
      {
         AnnoSetAddTerm(examples, handle);
      }
   }
   terms->sig = NULL;
   TBFree(terms);
   TypeBankFree(sort_table);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
