/*-----------------------------------------------------------------------

  File  : cle_annoterms.c

  Author: Stephan Schulz

  Contents

  Functions for dealing with annotated terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Tue Jul 20 17:49:05 MET DST 1999

  -----------------------------------------------------------------------*/

#include "cle_annoterms.h"


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
// Function: annotation_collect_max()
//
//   Set max_values[i] := max(max_values[i], anno(i+1)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void annotation_collect_max(DDArray_p max_values, Annotation_p anno)
{
   int i, elements;
   double old_max, old_val;
   DDArray_p array;

   array    = AnnotationValues(anno);
   elements = AnnotationLength(anno)-1;

   for(i=0; i < elements; i++)
   {
      old_max = DDArrayElement(max_values, i);
      old_val = DDArrayElement(array, i+1);
      DDArrayAssign(max_values, i, MAX(old_max, old_val));
   }
}


/*-----------------------------------------------------------------------
//
// Function: annotation_normalize()
//
//   Divide the values in anno by the corresponding max_value.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void annotation_normalize(Annotation_p anno, DDArray_p max_values)
{
   int i, elements;
   double old_max, old_val;
   DDArray_p array;

   array    = AnnotationValues(anno);
   elements = AnnotationLength(anno)-1;

   for(i=0; i < elements; i++)
   {
      old_max = DDArrayElement(max_values, i);
      if(old_max!=0.0)
      {
         old_val = DDArrayElement(array, i+1);
         DDArrayAssign(array, i+1, old_val/old_max);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: AnnoTermAlloc()
//
//   Allocate an initialized AnnoTermCell
//
// Global Variables: -
//
// Side Effects    : Gets term reference
//
/----------------------------------------------------------------------*/

AnnoTerm_p AnnoTermAlloc(Term_p term, Annotation_p annos)
{
   AnnoTerm_p handle = AnnoTermCellAlloc();

   handle->term = term;
   handle->annotation = annos;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: AnnoTermFree()
//
//   Free an annotated term. Does not free the
//   substitution. Carefull...it _will_ free the annotation.
//
// Global Variables: -
//
// Side Effects    : May change term bank.
//
/----------------------------------------------------------------------*/

void AnnoTermFree(TB_p bank, AnnoTerm_p junk)
{
   assert(junk->term);

   AnnotationTreeFree(junk->annotation);
   AnnoTermCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: AnnoTermParse()
//
//   Parse an annotated term and return it.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory operations
//
/----------------------------------------------------------------------*/

AnnoTerm_p AnnoTermParse(Scanner_p in, TB_p bank, long expected)
{
   Term_p       term;
   Annotation_p annos = NULL;
   AnnoTerm_p   handle;

   //term = TBRawTermParse(in, bank);
   term = TBTermParse(in, bank);
   AcceptInpTok(in, Colon);
   AnnotationListParse(in, &annos, expected);
   AcceptInpTok(in, Fullstop);
   handle = AnnoTermAlloc(term, annos);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AnnoTermPrint()
//
//   Print an annotated term.
//
// Global Variables: -
//
// Side Effects    : Output.
//
/----------------------------------------------------------------------*/

void AnnoTermPrint(FILE* out, TB_p bank, AnnoTerm_p term, bool fullterms)
{
   TBPrintTerm(out, bank, term->term, fullterms);
   fputs(" : ", out);
   AnnotationListPrint(out, term->annotation);
   fputc('.', out);
}


/*-----------------------------------------------------------------------
//
// Function:  AnnoTermRecToFlatEnc()
//
//   Take an annotated term encoding a clause in recursive format and
//   recode it into flat format.
//
// Global Variables: -
//
// Side Effects    : Changes term bank
//
/----------------------------------------------------------------------*/

void AnnoTermRecToFlatEnc(TB_p bank, AnnoTerm_p term)
{
   Term_p newrep;

   newrep = FlatRecodeRecClauseRep(bank,term->term);
   term->term = newrep;
}


/*-----------------------------------------------------------------------
//
// Function: AnnoSetAlloc()
//
//   Allocate an empty, initialized set of annotated terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

AnnoSet_p AnnoSetAlloc(TB_p bank)
{
   AnnoSet_p handle = AnnoSetCellAlloc();

   handle->terms = bank;
   SigGetEqnCode(handle->terms->sig, true);
   SigGetEqnCode(handle->terms->sig, false);
   SigGetOrCode(handle->terms->sig);
   SigGetCNilCode(handle->terms->sig);

   handle->set   = NULL;
   handle->subst = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AnnoSetFree()
//
//   Free a set of annotated terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void AnnoSetFree(AnnoSet_p junk)
{
   ArrayTree_p handle;
   PStack_p stack;

   stack = ArrayTreeTraverseInit(junk->set);

   while((handle = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= handle->last_used_index; i++) {
         if (handle->entries[i].key > -3) {
            AnnoTermFree(junk->terms, handle->entries[i].val1.p_val);
         }
      }
      // AnnoTermFree(junk->terms, handle->entries[0].val1.p_val);
   }
   ArrayTreeTraverseExit(stack);

   ArrayTreeFree(junk->set);
   /* Keep in mind that bank is external! */
   assert(junk->subst == NULL);
   AnnoSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: AnnoSetAddTerm()
//
//   Insert term into set, where term is expected to belong to the
//   sets term bank. Returns true if the term is new, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes set, may destroy term.
//
/----------------------------------------------------------------------*/

bool AnnoSetAddTerm(AnnoSet_p set, AnnoTerm_p term)
{
   ArrayTree_p exists, handle, conflict;
   AnnoTerm_p existing_term;
   IntOrP    tmp;
   bool res = true;

   assert(TBFind(set->terms, term->term));

   exists = ArrayTreeFind(&(set->set), term->term->entry_no);
   if(exists)
   {
      res = false;
      existing_term = exists->entries[0].val1.p_val;
      while(term->annotation)
      {
         handle = ArrayTreeExtractEntry(&term->annotation,
                                      term->annotation->entries[0].key);
         conflict = ArrayTreeInsert(&(existing_term->annotation), handle);
         if(conflict)
         {
            AnnotationCombine(conflict, handle);
            AnnotationFree(handle);
         }
      }
      AnnoTermFree(set->terms, term);
   }
   else
   {
      tmp.p_val = term;
      ArrayTreeStore(&(set->set), term->term->entry_no, tmp, tmp);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: AnnoSetParse()
//
//   Parse a set of annotated terms.
//
// Global Variables: -
//
// Side Effects    : Reads input, changes bank and set.
//
/----------------------------------------------------------------------*/

AnnoSet_p AnnoSetParse(Scanner_p in, TB_p bank, long expected)
{
   AnnoSet_p  set = AnnoSetAlloc(bank);
   AnnoTerm_p handle;

   while(TestInpTok(in, TermStartToken))
   {
      handle = AnnoTermParse(in, set->terms, expected);

      AnnoSetAddTerm(set, handle);
   }
   return set;
}


/*-----------------------------------------------------------------------
//
// Function: AnnoSetPrint()
//
//   Print a set of annotated terms.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void AnnoSetPrint(FILE* out, AnnoSet_p set)
{
   PStack_p stack;
   ArrayTree_p handle;

   fprintf(out, "\n# Annotated terms:\n");

   stack = ArrayTreeTraverseInit(set->set);
   while((handle = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= handle->last_used_index; i++) {
         AnnoTermPrint(out, set->terms, handle->entries[i].val1.p_val, true);
      }
      // AnnoTermPrint(out, set->terms, handle->entries[0].val1.p_val, true);
      fputc('\n', out);
   }
   ArrayTreeTraverseExit(stack);
}

/*-----------------------------------------------------------------------
//
// Function: AnnoSetComputePatternSubst()
//
//   Compute a pattern subst for all terms in set. Return true if
//   subst has been modified.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool AnnoSetComputePatternSubst(PatternSubst_p subst, AnnoSet_p set)
{
   PStack_p       stack;
   ArrayTree_p    handle;
   AnnoTerm_p     current;
   bool           res = false, tmp;

   assert(set);

   stack = ArrayTreeTraverseInit(set->set);
   while((handle = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= handle->last_used_index; i++) {
         if (handle->entries[i].key > -3) {
            current =  handle->entries[i].val1.p_val;
            assert(current);
            tmp = PatternTermCompute(subst,current->term);
            res = res || tmp;
         }
      }
      // current =  handle->entries[0].val1.p_val;
      // assert(current);
      // tmp = PatternTermCompute(subst,current->term);
      // res = res || tmp;
   }
   ArrayTreeTraverseExit(stack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: AnnoSetRemoveByIdent()
//
//   Given a set of terms and an example source id, remove all
//   annotations from the source, and any terms that have no remaining
//   annotations. Return number of terms deleted.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

long AnnoSetRemoveByIdent(AnnoSet_p set, long set_ident)
{
   long          count = 0;
   PStack_p      stack, to_delete = PStackAlloc();
   Annotation_p  anno;
   AnnoTerm_p    current;
   ArrayTree_p   handle;

   assert(set);

   stack = ArrayTreeTraverseInit(set->set);
   while((handle = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= handle->last_used_index; i++) {
         if (handle->entries[i].key > -3) {
            current = handle->entries[i].val1.p_val;
            anno = ArrayTreeExtractEntry(&(current->annotation), set_ident);
            if(anno)
            {
               AnnotationFree(anno);
            }
            if(!current->annotation)
            {
               PStackPushInt(to_delete, handle->entries[0].key);
            }
         }
      }
      // current = handle->entries[0].val1.p_val;
      // anno = ArrayTreeExtractEntry(&(current->annotation), set_ident);
      // if(anno)
      // {
      //    AnnotationFree(anno);
      // }
      // if(!current->annotation)
      // {
      //    PStackPushInt(to_delete, handle->entries[0].key);
      // }
   }
   ArrayTreeTraverseExit(stack);

   while(!PStackEmpty(to_delete))
   {
      handle = ArrayTreeExtractEntry(&(set->set),
                                   PStackPopInt(to_delete));
      assert(handle);
      AnnoTermFree(set->terms, handle->entries[0].val1.p_val);
      ArrayTreeNodeFree(handle);
      count++;
   }
   PStackFree(to_delete);

   return count;
}

/*-----------------------------------------------------------------------
//
// Function: AnnoSetRemoveExceptIdentList()
//
//   Given a set of terms and a stack of idents, remove all
//   annotations from the source whose are not in the stack. Remove
//   terms without annotations as well. If set_idents is
//   ANNOTATIONS_MERGE_ALL, do nothing. Returns number of terms deleted.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

long AnnoSetRemoveExceptIdentList(AnnoSet_p set, PStack_p set_idents)
{
   long          count = 0;
   PStack_p      stack, to_delete;
   Annotation_p  anno;
   AnnoTerm_p    current;
   ArrayTree_p   handle, tmptree, check;
   PStackPointer i;

   assert(set);

   if(set_idents == ANNOTATIONS_MERGE_ALL)
   {
      return 0;
   }
   assert(set);

   to_delete = PStackAlloc();
   stack = ArrayTreeTraverseInit(set->set);
   while((handle = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t j = 0; j <= handle->last_used_index; j++) {
         if (handle->entries[j].key > -3) {
            tmptree = NULL;
            current = handle->entries[j].val1.p_val;
            for(i=0; i<PStackGetSP(stack); i++)
            {
               anno = ArrayTreeExtractEntry(&(current->annotation),
                                          PStackElementInt(set_idents, i));
               if(anno)
               {
                  check = ArrayTreeInsert(&tmptree, anno);
                  UNUSED(check); assert(!check);
               }
            }
            AnnotationTreeFree(current->annotation);
            current->annotation = tmptree;
            if(!current->annotation)
            {
               PStackPushInt(to_delete, handle->entries[j].key);
            }
         }
      }
      // tmptree = NULL;
      // current = handle->entries[0].val1.p_val;
      // for(i=0; i<PStackGetSP(stack); i++)
      // {
      //    anno = ArrayTreeExtractEntry(&(current->annotation),
      //                               PStackElementInt(set_idents, i));
      //    if(anno)
      //    {
      //       check = ArrayTreeInsert(&tmptree, anno);
      //       UNUSED(check); assert(!check);
      //    }
      // }
      // AnnotationTreeFree(current->annotation);
      // current->annotation = tmptree;
      // if(!current->annotation)
      // {
      //    PStackPushInt(to_delete, handle->entries[0].key);
      // }
   }
   ArrayTreeTraverseExit(stack);

   while(!PStackEmpty(to_delete))
   {
      handle = ArrayTreeExtractEntry(&(set->set),
                                   PStackPopInt(to_delete));
      assert(handle);
      AnnoTermFree(set->terms, handle->entries[0].val1.p_val);
      ArrayTreeNodeFree(handle);
      count++;
   }
   PStackFree(to_delete);

   return count;
}

/*-----------------------------------------------------------------------
//
// Function: AnnoSetFlatten()
//
//   Given a set of annotated terms and a stack of idents (or
//   ANNOTATIONS_MERGE_ALL), compute a single flat annotation for all
//   terms and remove terms which have no annotation. Return number of
//   terms remaining.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

long AnnoSetFlatten(AnnoSet_p set, PStack_p set_idents)
{
   long          count = 0, annos_found;
   PStack_p      stack, to_delete = PStackAlloc();
   Annotation_p  anno;
   AnnoTerm_p    current;
   ArrayTree_p   handle;

   assert(set);

   stack = ArrayTreeTraverseInit(set->set);
   while((handle = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= handle->last_used_index; i++) {
         if (handle->entries[i].key > -3) {
            current = handle->entries[i].val1.p_val;
            anno = AnnotationAlloc();
            anno->entries[i].key = 0;
            annos_found = AnnotationMerge(&(current->annotation),
                                          anno, set_idents);
            if(annos_found)
            {
               assert(current->annotation);
               AnnotationLength(anno) =
                  AnnotationLength(current->annotation);
               AnnotationTreeFree(current->annotation);
               current->annotation = anno;
            }
            else
            {
               AnnotationFree(anno);
               PStackPushInt(to_delete, handle->entries[i].key);
            }
         }
      }
      // current = handle->entries[0].val1.p_val;
      // anno = AnnotationAlloc();
      // anno->entries[0].key = 0;
      // annos_found = AnnotationMerge(&(current->annotation),
      //                               anno, set_idents);
      // if(annos_found)
      // {
      //    assert(current->annotation);
      //    AnnotationLength(anno) =
      //       AnnotationLength(current->annotation);
      //    AnnotationTreeFree(current->annotation);
      //    current->annotation = anno;
      // }
      // else
      // {
      //    AnnotationFree(anno);
      //    PStackPushInt(to_delete, handle->entries[0].key);
      // }
   }
   ArrayTreeTraverseExit(stack);

   while(!PStackEmpty(to_delete))
   {
      handle = ArrayTreeExtractEntry(&(set->set),
                                   PStackPopInt(to_delete));
      assert(handle);
      AnnoTermFree(set->terms, handle->entries[0].val1.p_val);
      ArrayTreeNodeFree(handle);
   }
   PStackFree(to_delete);

   return count;
}


/*-----------------------------------------------------------------------
//
// Function: AnnoSetNormalizeFlatAnnos()
//
//   Normalize the annotations, i.e. divide each annotation value by
//   the maximum value of this annotation for all terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void AnnoSetNormalizeFlatAnnos(AnnoSet_p set)
{
   PStack_p   stack;
   ArrayTree_p  cell;
   AnnoTerm_p current;
   DDArray_p  max_values =  DDArrayAlloc(ANNOTATION_DEFAULT_SIZE,
                                         ANNOTATION_DEFAULT_SIZE);

   assert(set);

   stack = ArrayTreeTraverseInit(set->set);
   while((cell = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= cell->last_used_index; i++) {
         if (cell->entries[i].key > -3) {
            current = cell->entries[i].val1.p_val;
            assert(current);
            annotation_collect_max(max_values, current->annotation);
         }
      }
      // current = cell->entries[0].val1.p_val;
      // assert(current);
      // annotation_collect_max(max_values, current->annotation);
   }
   ArrayTreeTraverseExit(stack);

   stack = ArrayTreeTraverseInit(set->set);
   while((cell = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= cell->last_used_index; i++) {
         if (cell->entries[i].key > -3) {
            current = cell->entries[i].val1.p_val;
            assert(current);
            annotation_normalize(current->annotation, max_values);
         }
      }
      // current = cell->entries[0].val1.p_val;
      // assert(current);
      // annotation_normalize(current->annotation, max_values);
   }
   ArrayTreeTraverseExit(stack);
   DDArrayFree(max_values);
}



/*-----------------------------------------------------------------------
//
// Function: AnnoSetRecToFlatEnc()
//
//   Recode all terms in set from recursive to flat format. Returns
//   number of terms.
//
// Global Variables: -
//
// Side Effects    : Changes bank.
//
/----------------------------------------------------------------------*/

long AnnoSetRecToFlatEnc(TB_p bank, AnnoSet_p set)
{
   long      res = 0;
   PStack_p  stack;
   ArrayTree_p cell;
   AnnoTerm_p    current;

   assert(set);
   assert(bank);

   stack = ArrayTreeTraverseInit(set->set);
   while((cell = ArrayTreeTraverseNext(stack)))
   {
      for (uint8_t i = 0; i <= cell->last_used_index; i++) {
         if (cell->entries[i].key > -3) {
            current = cell->entries[i].val1.p_val;
            assert(current);
            AnnoTermRecToFlatEnc(bank, current);
            res++;
         }
      }
      // current = cell->entries[0].val1.p_val;
      // assert(current);
      // AnnoTermRecToFlatEnc(bank, current);
      // res++;
   }
   ArrayTreeTraverseExit(stack);

   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
