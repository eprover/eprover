/*-----------------------------------------------------------------------

File  : ccl_overlap_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Maintaining an index from subterms to clause positions.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_overlap_index.h"



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
// Function: term_collect_into_terms()
//
//   Collect all potential into-subterms into terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long term_collect_into_terms(Term_p t, PTree_p *terms)
{
   long res = 0;
   int  i;

   if(TermIsFreeVar(t))
   {
      return res;
   }
   PTreeStore(terms, t);
   res++;
   for(i=0; i<t->arity; i++)
   {
      res += term_collect_into_terms(t->args[i], terms);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: term_collect_into_terms2()
//
//   Collect all potential into-subterms into terms/natoms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long term_collect_into_terms2(Term_p t, PTree_p *terms, PTree_p *natoms)
{
   long res = 0;
   int  i;

   if(TermIsFreeVar(t))
   {
      return res;
   }
   PTreeStore(natoms, t);
   res++;
   for(i=0; i<t->arity; i++)
   {
      res += term_collect_into_terms(t->args[i], terms);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: eqn_collect_into_terms()
//
//   Collect all paramod-into terms in lit into terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long eqn_collect_into_terms(Eqn_p lit, PTree_p *terms)
{
   long res;

   res = term_collect_into_terms(lit->lterm, terms);
   if(!EqnIsOriented(lit))
   {
      res += term_collect_into_terms(lit->rterm, terms);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_collect_into_terms2()
//
//   Collect all paramod-into terms in lit into terms/natoms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long eqn_collect_into_terms2(Eqn_p lit, PTree_p *terms, PTree_p *natoms)
{
   long res;

   if(EqnIsNegative(lit) && !EqnIsEquLit(lit))
   {
      res = term_collect_into_terms2(lit->lterm, terms, natoms);
   }
   else
   {
      res = term_collect_into_terms(lit->lterm, terms);
   }
   if(!EqnIsOriented(lit))
   {
      res += term_collect_into_terms(lit->rterm, terms);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: term_collect_into_terms_pos()
//
//   Collect all potential into-subterms/pos position onto terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long term_collect_into_terms_pos(Term_p t, CompactPos pos, PStack_p terms)
{
   long res = 0;
   int  i;

   if(TermIsFreeVar(t))
   {
      return res;
   }
   PStackPushP(terms, t);
   PStackPushInt(terms, pos);
   res++;
   pos += DEFAULT_FWEIGHT*(TermIsPhonyApp(t) ? 0 : 1);
   if(!TermIsLambda(t))
   {
      for(i=0; i<t->arity; i++)
      {
         res += term_collect_into_terms_pos(t->args[i], pos, terms);
         pos += TermStandardWeight(t->args[i]);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: term_collect_into_terms_pos2()
//
//   Collect all potential into-subterms/pos positions of the LHS of a
//   negative non-equational literal onto terms/natoms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long term_collect_into_terms_pos2(Term_p t, CompactPos pos,
                                  PStack_p terms, PStack_p natoms)
{
   long res = 0;
   int  i;

   if(TermIsFreeVar(t))
   {
      return res;
   }
   PStackPushP(natoms, t);
   PStackPushInt(natoms, pos);
   res++;
   pos += DEFAULT_FWEIGHT*(TermIsPhonyApp(t) ? 0 : 1);
   if(!TermIsLambda(t))
   {
      for(i=0; i<t->arity; i++)
      {
         /* It's term_collect_into_terms_pos() on purpose - subterm need
            to be indexed in the normal index for equational inferences!
         */
         res += term_collect_into_terms_pos(t->args[i], pos, terms);
         pos += TermStandardWeight(t->args[i]);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: eqn_collect_into_terms_pos()
//
//   Collect all paramod-into terms with position in lit into terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long eqn_collect_into_terms_pos(Eqn_p lit, CompactPos litpos,
                                PStack_p terms)
{
   long res;

   res = term_collect_into_terms_pos(lit->lterm, litpos, terms);
   if(!EqnIsOriented(lit))
   {
      litpos += TermStandardWeight(lit->lterm);
      res += term_collect_into_terms_pos(lit->rterm, litpos, terms);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: eqn_collect_into_terms_pos2()
//
//   Collect all paramod-into terms with position in lit into
//   terms/natoms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long eqn_collect_into_terms_pos2(Eqn_p lit, CompactPos litpos,
                                 PStack_p terms, PStack_p natoms)
{
   long res;

   if(EqnIsNegative(lit) && !EqnIsEquLit(lit))
   {
      res = term_collect_into_terms_pos2(lit->lterm, litpos, terms, natoms);
   }
   else
   {
      res = term_collect_into_terms_pos(lit->lterm, litpos, terms);
   }
   if(!EqnIsOriented(lit))
   {
      litpos += TermStandardWeight(lit->lterm);
      res += term_collect_into_terms_pos(lit->rterm, litpos, terms);
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: OverlapIndexInsertPos()
//
//   Insert a position with clause|pos = iterm into the index. If
//   iterm is NULL, it will be computed from clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexInsertPos(OverlapIndex_p index, Clause_p clause,
                           CompactPos pos, Term_p iterm)
{
   FPTree_p     fp_node;
   SubtermOcc_p subterm_node;

   if(!iterm)
   {
      iterm = ClauseCPosGetSubterm(clause, pos);
   }
   fp_node      = FPIndexInsert(index, iterm);
   /* printf("OverlapIndexInsertPos()..\n");
      SubtermTreePrint(stdout, fp_node->payload,
      clause->literals->bank->sig); */
   subterm_node = SubtermTreeInsertTerm((void*)&(fp_node->payload), iterm);
   ClauseTPosTreeInsertPos(&(subterm_node->pl.pos.clauses), clause, pos);
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexDeletePos()
//
//   Delete a term->clause/position association from the index.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexDeletePos(OverlapIndex_p index, Clause_p clause,
                           CompactPos pos, Term_p iterm)
{
   FPTree_p     fp_node;
   SubtermOcc_p subterm_node;

   if(!iterm)
   {
      iterm = ClauseCPosGetSubterm(clause, pos);
   }

   fp_node   = FPIndexFind(index, iterm);
   if(!fp_node)
   {
      return;
   }

   subterm_node = SubtermTreeFindTerm((void*)&(fp_node->payload), iterm);
   if(!subterm_node)
   {
      return;
   }
   ClauseTPosTreeDeletePos(&(subterm_node->pl.pos.clauses), clause, pos);
   if(!subterm_node->pl.pos.clauses)
   {
      SubtermTreeDeleteTerm((void*)&(fp_node->payload), iterm);
   }
   if(fp_node->payload == NULL)
   {
      FPIndexDelete(index, iterm);
   }
   return;
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexDeleteClauseOcc()
//
//   Delete all associations clause->pos via term from the index. This
//   is an optimization - we usually index and unindex full clauses.
//
// Global Variables: -
//
// Side Effects    : Changes the index
//
/----------------------------------------------------------------------*/

void OverlapIndexDeleteClauseOcc(OverlapIndex_p index,
                                 Clause_p clause, Term_p term)
{
   FPTree_p     fp_node;
   SubtermOcc_p subterm_node;

   /* printf("OverlapIndexDeleteClauseOcc()...\n");
   ClausePrint(stdout, clause, true);
   printf("\n");
   TermPrint(stdout, term, clause->literals->bank->sig, DEREF_NEVER);
   printf("\n"); */

   fp_node   = FPIndexFind(index, term);
   if(!fp_node)
   {
      return;
   }
   /* SubtermTreePrint(stdout, fp_node->payload,
    * clause->literals->bank->sig); */

   subterm_node = SubtermTreeFindTerm((PTree_p*)&(fp_node->payload), term);
   /* printf("Found: %p\n", subterm_node); */

   if(!subterm_node)
   {
      return;
   }
   ClauseTPosTreeDeleteClause(&(subterm_node->pl.pos.clauses), clause);
   //printf("Del\n");
   if(!subterm_node->pl.pos.clauses)
   {
      SubtermTreeDeleteTerm((PTree_p*)&(fp_node->payload), term);
   }
   if(fp_node->payload == NULL)
   {
      FPIndexDelete(index, term);
   }
   return;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIntoTerms()
//
//   Collect all term for paramodulation _into_ into tree. These are
//   non-variable terms in maximal sides of maximal literals. Return
//   number of term positions affected.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseCollectIntoTerms(Clause_p clause, PTree_p *terms)
{
   long  res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle))
      {
         res+=eqn_collect_into_terms(handle, terms);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIntoTermsPos()
//
//   Collect tuples cpos, t on stack, so that c|cpos = t and t is a
//   paramod-into position.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCollectIntoTermsPos(Clause_p clause, PStack_p terms)
{
   long  res = 0;
   Eqn_p handle;
   CompactPos pos = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle))
      {
         res += eqn_collect_into_terms_pos(handle, pos, terms);
      }
      pos += EqnStandardWeight(handle);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ClauseCollectFromTerms()
//
//   Collect all "from" terms (i.e. potential left hand sides of the
//   rule of a clause seen as a conditional rewrite rule) into terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseCollectFromTerms(Clause_p clause, PTree_p *terms)
{
   long  res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle) && EqnIsPositive(handle) && !EqnIsSelected(handle))
      {
         res++;
         PTreeStore(terms, handle->lterm);
         if(!EqnIsOriented(handle))
         {
            res++;
            PTreeStore(terms, handle->rterm);
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCollectFromTermsPos()
//
//   Collect all t|p tuples such that c|p=t and this is a paramod-from
//   position.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCollectFromTermsPos(Clause_p clause, PStack_p terms)
{
   long  res = 0;
   Eqn_p handle;
   CompactPos pos = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle) && EqnIsPositive(handle) && !EqnIsSelected(handle))
      {
         res++;
         PStackPushP(terms, handle->lterm);
         PStackPushInt(terms, pos);
         if(!EqnIsOriented(handle))
         {
            res++;
            PStackPushP(terms, handle->rterm);
            PStackPushInt(terms, pos+TermStandardWeight(handle->lterm));
         }
      }
      pos += EqnStandardWeight(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexInsertIntoClause()
//
//   Insert a clause into an overlap-into index
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexInsertIntoClause(OverlapIndex_p index, Clause_p clause)
{
   PStack_p collector = PStackAlloc();
   CompactPos pos;
   Term_p     term;

   ClauseCollectIntoTermsPos(clause, collector);
   while(!PStackEmpty(collector))
   {
      pos  = PStackPopInt(collector);
      term = PStackPopP(collector);
      OverlapIndexInsertPos(index, clause, pos, term);
   }
   PStackFree(collector);
}




/*-----------------------------------------------------------------------
//
// Function: OverlapIndexDeleteIntoClause()
//
//   Delete a clause from the overlap-into index.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexDeleteIntoClause(OverlapIndex_p index, Clause_p clause)
{
   PStack_p trans;
   Term_p   term;
   PTree_p  collector = NULL, cell;

   ClauseCollectIntoTerms(clause, &collector);

   trans = PTreeTraverseInit(collector);
   while((cell = PTreeTraverseNext(trans)))
   {
      term = cell->key;
      OverlapIndexDeleteClauseOcc(index, clause, term);
   }
   PTreeTraverseExit(trans);
   PTreeFree(collector);
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexInsertFromClause()
//
//   Insert a clause into an overlap-from index
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexInsertFromClause(OverlapIndex_p index, Clause_p clause)
{
   PStack_p collector = PStackAlloc();
   CompactPos pos;
   Term_p     term;

   ClauseCollectFromTermsPos(clause, collector);

   while(!PStackEmpty(collector))
   {
      pos  = PStackPopInt(collector);
      term = PStackPopP(collector);
      OverlapIndexInsertPos(index, clause, pos, term);
   }
   PStackFree(collector);
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexDeleteFromClause()
//
//   Delete a clause from an overlap-from index
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexDeleteFromClause(OverlapIndex_p index, Clause_p clause)
{
   PStack_p trans;
   Term_p   term;
   PTree_p  collector = NULL, cell;

   ClauseCollectFromTerms(clause, &collector);

   trans = PTreeTraverseInit(collector);
   while((cell = PTreeTraverseNext(trans)))
   {
      term = cell->key;
      OverlapIndexDeleteClauseOcc(index, clause, term);
   }
   PTreeTraverseExit(trans);
   PTreeFree(collector);
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexClauseTreePrint()
//
//   Print an overlapIndex.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void OverlapIndexClauseTreePrint(FILE* out, PObjTree_p root)
{
   if(root)
   {
      OverlapIndexClauseTreePrint(out, root->lson);
      ClauseTPosTreePrint(out, root->key);
      OverlapIndexClauseTreePrint(out, root->rson);
   }
}

/*-----------------------------------------------------------------------
//
// Function: OverlapIndexSubtermTreePrint()
//
//   Print a suberm tree (only for debugging)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void OverlapIndexSubtermTreePrint(FILE* out, SubtermTree_p root)
{
   SubtermOcc_p data;

   if(root)
   {
      OverlapIndexSubtermTreePrint(out, root->lson);
      data = root->key;
      fprintf(out, "Node: %p data=%p\n", root, data);
      fprintf(out, "Key: %ld = ", data->term->entry_no);
      if(data->pl.pos.clauses)
      {
         ClauseTPos_p clause_tpos = data->pl.pos.clauses->key;
         Clause_p clause = clause_tpos->clause;
         if(clause->literals)
         {
             TermPrint(out, data->term, clause->literals->bank->sig, DEREF_ALWAYS);
         }
         fprintf(out, "\n");
         OverlapIndexClauseTreePrint(out, data->pl.pos.clauses);
      }
      else
      {
         fprintf(out, "\nlson=%p, rson=%p\n\n", root->lson, root->rson);
      }
      OverlapIndexSubtermTreePrint(out, root->rson);
   }
}

/*-----------------------------------------------------------------------
//
// Function: OverlapIndexFPLeafPrint()
//
//   Print a leaf as the path leading to it and the number of direct
//   entries in the subterm.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void OverlapIndexFPLeafPrint(FILE* out, PStack_p stack, FPTree_p leaf)
{
   PStack_p iter = PStackAlloc();

   fprintf(out, COMCHAR" ");
   PStackPrintInt(out, "%4ld.", stack);
   fprintf(out, ":%ld terms\n", PObjTreeNodes(leaf->payload));
   OverlapIndexSubtermTreePrint(out, leaf->payload);

   PStackFree(iter);
}



/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIntoTerms2()
//
//   Collect all term for paramodulation _into_ into two trees. These
//   are non-variable terms in maximal sides of maximal
//   literals. Negative atom-terms go into the second tree, all others
//   into the first. Return number of term positions affected.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseCollectIntoTerms2(Clause_p clause, PTree_p *terms, PTree_p *natoms)
{
   long  res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle))
      {
         res+=eqn_collect_into_terms2(handle, terms, natoms);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIntoTermsPos2()
//
//   Collect tuples cpos, t on stack(s), so that c|cpos = t and t is a
//   paramod-into position. Negative non-equational predicate terms go
//   onto natoms, the rest onto terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseCollectIntoTermsPos2(Clause_p clause,
                                PStack_p terms,
                                PStack_p natoms)
{
   long  res = 0;
   Eqn_p handle;
   CompactPos pos = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsMaximal(handle))
      {
         res += eqn_collect_into_terms_pos2(handle, pos, terms, natoms);
      }
      pos += EqnStandardWeight(handle);
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: OverlapIndexInsertIntoClause2()
//
//   Insert a clause into two overlap-into indices.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexInsertIntoClause2(OverlapIndex_p tindex,
                                   OverlapIndex_p naindex,
                                   Clause_p clause)
{
   PStack_p terms = PStackAlloc();
   PStack_p natoms = PStackAlloc();
   CompactPos pos;
   Term_p     term;

   ClauseCollectIntoTermsPos2(clause, terms, natoms);
   while(!PStackEmpty(terms))
   {
      pos  = PStackPopInt(terms);
      term = PStackPopP(terms);
      OverlapIndexInsertPos(tindex, clause, pos, term);
   }
   while(!PStackEmpty(natoms))
   {
      pos  = PStackPopInt(natoms);
      term = PStackPopP(natoms);
      OverlapIndexInsertPos(naindex, clause, pos, term);
   }
   PStackFree(natoms);
   PStackFree(terms);
}


/*-----------------------------------------------------------------------
//
// Function: OverlapIndexDeleteIntoClause2()
//
//   Delete a clause from the two overlap-into indeces.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void OverlapIndexDeleteIntoClause2(OverlapIndex_p tindex,
                                   OverlapIndex_p naindex,
                                   Clause_p clause)
{
   PStack_p trans;
   Term_p   term;
   PTree_p  terms = NULL,
      natoms= NULL,
      cell;

   ClauseCollectIntoTerms2(clause, &terms, &natoms);

   trans = PTreeTraverseInit(terms);
   while((cell = PTreeTraverseNext(trans)))
   {
      term = cell->key;
      OverlapIndexDeleteClauseOcc(tindex, clause, term);
   }
   PTreeTraverseExit(trans);
   PTreeFree(terms);

   trans = PTreeTraverseInit(natoms);
   while((cell = PTreeTraverseNext(trans)))
   {
      term = cell->key;
      OverlapIndexDeleteClauseOcc(naindex, clause, term);
   }
   PTreeTraverseExit(trans);
   PTreeFree(natoms);
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
