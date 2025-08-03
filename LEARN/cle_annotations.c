/*-----------------------------------------------------------------------

  File  : cle_annotations.c

  Author: Stephan Schulz

  Contents

  Functions for dealing with annotaions and lists of annotations.

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Jul 19 11:27:10 MET DST 1999

-----------------------------------------------------------------------*/

#include "cle_annotations.h"



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
// Function: AnnotationAlloc()
//
//   Allocate a NumTree-Cell with accompanying ClauseStats-Cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Annotation_p AnnotationAlloc(void)
{
   Annotation_p handle = NumTreeCellAllocEmpty();

   handle->val1.p_val =
      DDArrayAlloc(ANNOTATION_DEFAULT_SIZE,
         ANNOTATION_DEFAULT_SIZE);
   handle->val2.p_val = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationFree()
//
//   Free an annotation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void AnnotationFree(Annotation_p junk)
{
   assert(junk);
   assert(junk->val1.p_val);

   DDArrayFree(junk->val1.p_val);
   NumTreeCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationTreeFree()
//
//   Free an annotation tree.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void AnnotationTreeFree(Annotation_p tree)
{
   PStack_p stack = NumTreeTraverseInit(tree);
   NumTree_p handle;

   while((handle = NumTreeTraverseNext(stack)))
   {
      DDArrayFree(handle->val1.p_val);
   }
   NumTreeTraverseExit(stack);
   NumTreeFree(tree);
}

/*-----------------------------------------------------------------------
//
// Function: AnnotationParse()
//
//   Parse a single annotation of the proof:(number[,number]*) and
//   return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory operations
//
/----------------------------------------------------------------------*/

Annotation_p AnnotationParse(Scanner_p in, long expected)
{
   Annotation_p handle = AnnotationAlloc();
   long         count = 0;
   double       value;

   assert(expected >=0 );
   handle->key = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Colon);
   AcceptInpTok(in, OpenBracket);

   while(!TestInpTok(in, CloseBracket))
   {
      if(count == expected)
      {
         AktTokenError(in,
                       "Annotation has more elements than expected",
                       false);
      }
      value = ParseFloat(in);
      DDArrayAssign(handle->val1.p_val, count, value);
      count++;
      if(!TestInpTok(in, CloseBracket))
      {
         AcceptInpTok(in, Comma);
      }
   }
   if(count < expected)
   {
      AktTokenError(in,
                    "Annotation has fewer elements than expected",
                    false);
   }
   AcceptInpTok(in, CloseBracket);
   AnnotationLength(handle) = count;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationListParse()
//
//   Parse the list of annotations into the tree. Return number of
//   items parsed.
//
// Global Variables: -
//
// Side Effects    : Reads input, changes tree, memory operations
//
/----------------------------------------------------------------------*/

long AnnotationListParse(Scanner_p in, Annotation_p *tree, long
                         expected)
{
   DStr_p     source_name;
   long       line, column;
   StreamType type;
   long i=0;
   Annotation_p handle;

   while(TestInpTok(in, PosInt))
   {
      line = AktToken(in)->line;
      column = AktToken(in)->column;
      source_name = DStrGetRef(AktToken(in)->source);
      type = AktToken(in)->stream_type;
      handle = AnnotationParse(in, expected);
      handle = NumTreeInsert(tree, handle);
      if(handle)
      {
         AnnotationFree(handle);
         Error("%s Only one annotation for each proof example allowed",
               SYNTAX_ERROR,
               PosRep(type, source_name, line, column));
      }
      DStrReleaseRef(source_name);
      i++;
      if(TestInpTok(in, Comma))
      {
         NextToken(in);
      }
   }
   return i;
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationPrint()
//
//   Print a single annotation.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void AnnotationPrint(FILE* out, Annotation_p anno)
{
   long i;
   char* sep = "";

   fprintf(out, "%ld:(", anno->key);
   for(i=0; i< anno->val2.i_val; i++)
   {
      fputs(sep, out);
      fprintf(out, "%f", DDArrayElement(anno->val1.p_val, i));
      sep = ",";
   }
   fputc(')', out);
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationListPrint()
//
//   Print the list of annotations.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void AnnotationListPrint(FILE* out, Annotation_p tree)
{
   char* sep = "";
   PStack_p stack = NumTreeTraverseInit(tree);
   Annotation_p handle;

   while((handle = NumTreeTraverseNext(stack)))
   {
      fputs(sep, out);
      AnnotationPrint(out, handle);
   }
   NumTreeTraverseExit(stack);
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationCombine()
//
//   Combine two annotations into one, i.e. compute the weighted
//   average of each value. Results are returned in res.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void AnnotationCombine(Annotation_p res, Annotation_p new_anno)
{
   int i;
   double resval, resw, newval, neww;
   long length;

   /* printf("Combining: ");AnnotationPrint(stdout, res);printf("\n");
      printf("           ");AnnotationPrint(stdout, new_anno);printf("\n");*/
   resw = AnnotationCount(res);
   neww = AnnotationCount(new_anno);

   /* Res can be newly allocated... */
   length =  AnnotationLength(new_anno);
   assert(AnnotationLength(res)<=length);

   for(i=1;i<=length;i++)
   {
      resval = DDArrayElement(AnnotationValues(res),i);
      newval = DDArrayElement(AnnotationValues(new_anno),i);
      resval = (resval*resw+newval*neww)/(resw+neww);
      DDArrayAssign(AnnotationValues(res),i,resval);
   }
   AnnotationCount(res) = resw+neww;
   AnnotationLength(res) = length;
   /* printf("Combined: ");AnnotationPrint(stdout, res);printf("\n"); */
}



/*-----------------------------------------------------------------------
//
// Function: AnnotationMerge()
//
//   Given an annotation tree and a stack of annotation keys (proof
//   numbers), add the annotation vector to *collect. Return number of
//   annotations found.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long AnnotationMerge(Annotation_p *tree, Annotation_p collect,
           PStack_p sources)
{
   PStackPointer i;
   long          count = 0;
   Annotation_p  handle;

   if(sources == ANNOTATIONS_MERGE_ALL)
   {
      PStack_p stack = NumTreeTraverseInit(*tree);

      while((handle = NumTreeTraverseNext(stack)))
      {
    AnnotationCombine(collect, handle);
    count++;
      }
      NumTreeTraverseExit(stack);
   }
   else
   {
      for(i=0; i< PStackGetSP(sources); i++)
      {
    handle = NumTreeFind(tree, PStackElementInt(sources, i));
    if(handle)
    {
       AnnotationCombine(collect, handle);
       count++;
    }
      }
   }
   return count;
}


/*-----------------------------------------------------------------------
//
// Function: AnnotationEval()
//
//   Return an evaluation for the annotation. The annotation is sum
//   anno[i+1]*weights[i] (due to the special meaning of anno[0]. Yes,
//   this sucks rocks, but I'm to lazy to fix this now!)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double AnnotationEval(Annotation_p anno, double weights[])
{
   DDArray_p array;
   long      elements, i;
   double    res = 0;

   array    = AnnotationValues(anno);
   elements = AnnotationLength(anno)-1;

   for(i=0; i < elements; i++)
   {
      res += DDArrayElement(array, i+1)*weights[i];
   }
   /* printf(COMCHAR" ");AnnotationPrint(stdout, anno); printf(" --> %f\n", res); */
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
