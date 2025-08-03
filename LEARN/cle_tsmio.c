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


/*-----------------------------------------------------------------------
//
// Function: rec_get_highest_weight()
//
//   Return the highest eval_weight in a recursive tsm.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double rec_get_highest_weight(TSM_p tsm)
{
   double tmp, res = 1000000000000.0; /* Large enough */
   int i,j;
   TSA_p tsa;

   assert(tsm);

   for(i=0; i<=tsm->maxindex;i++)
   {
      tsa = PDArrayElementP(tsm->tsas, i);
      if(tsa)
      {
    res = MAX(res, tsa->eval_weight);
    for(j=0; j<tsa->arity; j++)
    {
       tmp = rec_get_highest_weight(tsa->arg_tsms[j]);
       res = MAX(res, tmp);
    }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: level_get_highest_weight()
//
//   Return the highest eval_weight in a sinlge-level tsm.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double level_get_highest_weight(TSM_p tsm)
{
   double res = 1000000000000.0; /* Large enough */
   int i;
   TSA_p tsa;

   assert(tsm);

   for(i=0; i<=tsm->maxindex;i++)
   {
      tsa = PDArrayElementP(tsm->tsas, i);
      if(tsa)
      {
    res = MAX(res, tsa->eval_weight);
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: get_highest_weight()
//
//   Return the highest eval_weight in a tsm.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static double get_highest_weight(TSMAdmin_p admin)
{
   double res = 100000000.0,tmp; /* Large enough */
   PStackPointer i;

   assert(admin);

   switch(admin->tsmtype)
   {
   case TSMTypeFlat:
    res = level_get_highest_weight(admin->tsm);
    break;
   case TSMTypeRecursive:
    res = rec_get_highest_weight(admin->tsm);
    break;
   case TSMTypeRecurrent:
    res = level_get_highest_weight(admin->tsm);
    break;
   case TSMTypeRecurrentLocal:
    for(i=0; i<PStackGetSP(admin->tsmstack); i++)
    {
       tmp =
          level_get_highest_weight(PStackElementP(admin->tsmstack,i));
       res = MAX(res, tmp);
    }
    break;
   default:
    assert(false && "Not a valid tsm type");
    break;
   }
   return res;
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
   admin->unmapped_weight =  get_highest_weight(admin);

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
