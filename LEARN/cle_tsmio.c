/*-----------------------------------------------------------------------

File  : cle_tsmio.c

Author: Stephan Schulz

Contents


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Aug 31 13:47:43 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cle_tsmio.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static double selection_weights[] = SEL_FEATURE_WEIGHTS;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: get_default_eval()
//
//   Return the default evaluation for a set of annoterms: Assume
//   proofs=0, proof distance    = max pd in set+1, all other values =
//   average.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static double get_default_eval(AnnoSet_p annoset,double evalweights[])
{
   Annotation_p anno = AnnotationAlloc();
   PStack_p   stack;
   NumTree_p  handle;
   int        i;
   long       count = 0, currentcount;
   DDArray_p  currentvalues, old;
   AnnoTerm_p current;
   double     oldval, result;

   assert(annoset);
   assert(evalweights);

   AnnotationLength(anno) = KB_ANNOTATION_NO;
   old = AnnotationValues(anno);
   stack = NumTreeTraverseInit(annoset->set);
   while((handle = NumTreeTraverseNext(stack)))
   {
       current = handle->val1.p_val;
       currentvalues = AnnotationValues(current->annotation);
       currentcount = AnnotationCount(current->annotation);
       for(i=3; i<=KB_ANNOTATION_NO; i++)
       {
     oldval = DDArrayElement(old, i);
     oldval+= DDArrayElement(currentvalues,i)*currentcount;
     DDArrayAssign(old, i, oldval);
       }
       oldval = DDArrayElement(old, 2);
       oldval = MAX(oldval, DDArrayElement(currentvalues,2));
       DDArrayAssign(old, 2, oldval);
       count += currentcount;
   }
   NumTreeTraverseExit(stack);
   if(count!=0)
   {
      for(i=3; i<=KB_ANNOTATION_NO; i++)
      {
    oldval = DDArrayElement(old, i);
    oldval = oldval/(double)count;
    DDArrayAssign(old, i, oldval);
      }
   }
   DDArrayAssign(old, 2, DDArrayElement(old, 2)+1);

   result = AnnotationEval(anno,evalweights);
   /* printf(COMCHAR" ");AnnotationPrint(stdout, anno);
      printf(" -> %f\n",result); */
   AnnotationFree(anno);
   return result;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ExampleSetPrepare()
//
//   Create a flat annotated example set with examples tailored to
//   target from annoset. Return the default evaluatiopn.
//
// Global Variables: -
//
// Side Effects    : Changes annoset and flatset.
//
/----------------------------------------------------------------------*/

double ExampleSetPrepare(FlatAnnoSet_p flatset, AnnoSet_p annoset, double
             evalweights[], ExampleSet_p examples,  Sig_p sig,
             ClauseSet_p target, long sel_no, double set_part,
             double dist_part)
{
   Features_p   targetfeatures = FeaturesAlloc();
   PStack_p     example_ids = PStackAlloc();
   long         res;

   ComputeClauseSetNumFeatures(targetfeatures, target, sig);
   ExampleSetSelectByDist(example_ids, examples, targetfeatures,
           SEL_PRED_WEIGHT, SEL_FUNC_WEIGHT,
           selection_weights, sel_no, set_part,
           dist_part);
   AnnoSetFlatten(annoset, example_ids);
   AnnoSetNormalizeFlatAnnos(annoset);
   res = get_default_eval(annoset, evalweights);
   FlatAnnoSetTranslate(flatset, annoset, evalweights);
   PStackFree(example_ids);
   FeaturesFree(targetfeatures);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetFromKB()
//
//   Create a flat annotated example set from a knowledge base. Return
//   default evaluation.
//
// Global Variables: -
//
// Side Effects    : I/O, lot's of memory operations, changes
//                   sig.
//
/----------------------------------------------------------------------*/

double ExampleSetFromKB(AnnoSet_p annoset, FlatAnnoSet_p flatset, bool
         flat_patterns, TB_p bank, double evalweights[],
         char* kb, Sig_p sig, ClauseSet_p target, long
         sel_no, double set_part, double dist_part)
{
   DStr_p         filename = DStrAlloc();
   ExampleSet_p   proofexamples;
   Scanner_p      in;
   double res;

   proofexamples = ExampleSetAlloc();

   in = CreateScanner(StreamTypeFile,
                      KBFileName(filename, kb, "signature"),
                      true, NULL, true);
   SigParse(in, sig, true);
   DestroyScanner(in);

   in = CreateScanner(StreamTypeFile,
                      KBFileName(filename, kb, "problems"),
                      true, NULL, true);
   ExampleSetParse(in, proofexamples);
   DestroyScanner(in);
   DStrFree(filename);

   if(flat_patterns)
   {
      AnnoSetRecToFlatEnc(bank, annoset);
   }
   res = ExampleSetPrepare(flatset, annoset, evalweights, proofexamples,
           sig, target, sel_no, set_part, dist_part);
   ExampleSetFree(proofexamples);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TSMFromKB()
//
//   Create a tsm for evaluating clauses for proving target from kb
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

TSMAdmin_p TSMFromKB(bool flat_patterns, double evalweights[], char*
           kb,  Sig_p sig, ClauseSet_p target, long sel_no,
           double set_part, double dist_part, IndexType
           indextype, TSMType tsmtype, long indexdepth)
{
   DStr_p         filename = DStrAlloc();
   AnnoSet_p      annoset;
   FlatAnnoSet_p  flatset = FlatAnnoSetAlloc();
   TB_p bank =    TBAlloc(sig);
   TSMAdmin_p     admin;
   PatternSubst_p subst;
   Scanner_p      in;
   double         eval_default;

   in = CreateScanner(StreamTypeFile,
                      KBFileName(filename, kb, "clausepatterns"),
                      true, NULL, true);
   annoset = AnnoSetParse(in, bank, KB_ANNOTATION_NO);
   DestroyScanner(in);
   DStrFree(filename);

   eval_default = ExampleSetFromKB(annoset, flatset, flat_patterns,
               bank, evalweights, kb, sig, target,
               sel_no, set_part, dist_part);

   subst = PatternDefaultSubstAlloc(sig);
   AnnoSetComputePatternSubst(subst, annoset);
   /* AnnoSetPrint(stdout,annoset); */
   admin = TSMAdminAlloc(sig, tsmtype);
   TSMAdminBuildTSM(admin, flatset, indextype, indexdepth, subst);
   admin->unmapped_eval   = eval_default;

   FlatAnnoSetFree(flatset);
   bank->sig = NULL;
   AnnoSetFree(annoset);
   TBFree(bank);
   VERBOUT("TSM created\n");
   return admin;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
