/*-----------------------------------------------------------------------

  File  : cle_examplerep.c

  Author: Stephan Schulz

  Contents

  Functions for dealing with (sets of) example representations.

  Copyright 1998, 1999, 2024 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jul 27 11:46:29 MET DST 1999

-----------------------------------------------------------------------*/

#include "cle_examplerep.h"



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
// Function: ExampleRepFree()
//
//   Free an example represenation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ExampleRepFree(ExampleRep_p junk)
{
   FeaturesFree(junk->features);
   FREE(junk->name);
   ExampleRepCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ExampleRepPrint()
//
//   Print an example representation.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ExampleRepPrint(FILE* out, ExampleRep_p rep)
{
   fprintf(out, "%ld: \"%s\"\n", rep->ident, rep->name);
   NumFeaturesPrint(out, rep->features);
   fputc('\n', out);
}


/*-----------------------------------------------------------------------
//
// Function: ExampleRepParse()
//
//   Parse an example representation and return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Input, memory operations.
//
/----------------------------------------------------------------------*/

ExampleRep_p  ExampleRepParse(Scanner_p in)
{
   ExampleRep_p handle = ExampleRepCellAlloc();

   handle->ident = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Colon);
   CheckInpTok(in, Name);
   if(TestInpTok(in, String))
   {
      char *tmp;
      int l;

      tmp = DStrCopy(AktToken(in)->literal);
      l = strlen(tmp);
      tmp[l-1] = '\0';
      handle->name = SecureStrdup(tmp+1);
      FREE(tmp);
   }
   else
   {
      handle->name = DStrCopy(AktToken(in)->literal);
   }
   NextToken(in);
   handle->features = NumFeaturesParse(in);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ExampleSetAlloc()
//
//   Allocate an empty example set and return a pointer to it.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ExampleSet_p ExampleSetAlloc(void)
{
   ExampleSet_p handle = ExampleSetCellAlloc();

   handle->count = 0;
   handle->ident_index = NULL;
   handle->name_index = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetFree()
//
//   Free an exampel set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ExampleSetFree(ExampleSet_p junk)
{
   PStack_p stack;
   NumTree_p handle;

   stack = NumTreeTraverseInit(junk->ident_index);
   while((handle = NumTreeTraverseNext(stack)))
   {
      ExampleRepFree(handle->val1.p_val);
   }
   PStackFree(stack);
   NumTreeFree(junk->ident_index);
   StrTreeFree(junk->name_index);
   ExampleSetCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetFindName()
//
//   Find an entry by name, return NULL if non-existant.
//
// Global Variables: -
//
// Side Effects    : - (well, reorganizes tree)
//
/----------------------------------------------------------------------*/

ExampleRep_p  ExampleSetFindName(ExampleSet_p set, char* name)
{
   StrTree_p handle;

   handle = StrTreeFind(&(set->name_index), name);
   if(!handle)
   {
      return NULL;
   }
   return handle->val1.p_val;
}

/*-----------------------------------------------------------------------
//
// Function: ExampleSetInsert()
//
//   Insert rep into set. Return true if it works, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

bool ExampleSetInsert(ExampleSet_p set, ExampleRep_p rep)
{
   IntOrP tmp;
   StrTree_p res;
   bool res1;

   tmp.p_val = rep;

   res1 = NumTreeStore(&(set->ident_index), rep->ident, tmp, tmp);
   if(!res1)
   {
      return false;
   }
   res = StrTreeStore(&(set->name_index), rep->name, tmp, tmp);
   if(!res)
   {
      return false;
   }
   set->count = MAX(set->count, rep->ident);
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetExtract()
//
//   Extract rep from set and return it. Return NULL is rep does not
//   exist in set.
//
// Global Variables: -
//
// Side Effects    : Changes trees.
//
/----------------------------------------------------------------------*/

ExampleRep_p ExampleSetExtract(ExampleSet_p set, ExampleRep_p rep)
{
   ExampleRep_p handle;
   NumTree_p    cell;
   bool         res;

   cell = NumTreeExtractEntry(&(set->ident_index), rep->ident);
   if(!cell)
   {
      return NULL;
   }
   handle = cell->val1.p_val;
   NumTreeCellFree(cell);
   res = StrTreeDeleteEntry(&(set->name_index), rep->name);
   UNUSED(res); assert(res);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetDeleteId()
//
//   Delete the example with ident id. Returns success.
//
// Global Variables: -
//
// Side Effects    : Changes set, memory operations
//
/----------------------------------------------------------------------*/

bool ExampleSetDeleteId(ExampleSet_p set, long ident)
{
   ExampleRep_p handle;
   NumTree_p    cell;

   cell = NumTreeFind(&(set->ident_index), ident);
   if(!cell)
   {
      return false;
   }
   handle = ExampleSetExtract(set, cell->val1.p_val);
   assert(handle);
   ExampleRepFree(handle);
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: ExampleSetDeleteName()
//
//   Delete the example with name name. Returns success.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ExampleSetDeleteName(ExampleSet_p set, char* name)
{
   ExampleRep_p handle;
   StrTree_p    cell;

   cell = StrTreeFind(&(set->name_index), name);
   if(!cell)
   {
      return false;
   }
   handle = ExampleSetExtract(set, cell->val1.p_val);
   assert(handle);
   ExampleRepFree(handle);
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: ExampleSetPrint()
//
//   Print a set of example representations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void ExampleSetPrint(FILE* out, ExampleSet_p set)
{
   PStack_p  stack;
   NumTree_p handle;

   stack = NumTreeTraverseInit(set->ident_index);
   while((handle = NumTreeTraverseNext(stack)))
   {
      ExampleRepPrint(out, handle->val1.p_val);
   }
   NumTreeTraverseExit(stack);
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetParse()
//
//   Parse a list of examples into set. Return number of items parsed.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory, may give error, changes
//                   set->count.
//
/----------------------------------------------------------------------*/

long ExampleSetParse(Scanner_p in, ExampleSet_p set)
{
   long         count = 0;
   ExampleRep_p handle;
   DStr_p       source_name;
   long         line, column;
   StreamType   type;
   bool         res;

   while(TestInpTok(in, PosInt))
   {
      line = AktToken(in)->line;
      column = AktToken(in)->column;
      source_name = DStrGetRef(AktToken(in)->source);
      type = AktToken(in)->stream_type;
      handle = ExampleRepParse(in);
      res =  ExampleSetInsert(set, handle);
      if(!res)
      {
         Error("%s Entry %s conficts with existing entries",
               SYNTAX_ERROR,
               PosRep(type, source_name, line, column),
               handle->ident);
      }
      count++;
      DStrReleaseRef(source_name);
   }
   return count;
}


/*-----------------------------------------------------------------------
//
// Function: ExampleSetSelectByDist()
//
//   Push idents of the most similar examples onto results. How many
//   examples is controlled by select, set_part, and dist_part:
//   Selected are at most select examples, at most part*setsize
//   examples and only examples whose distance is not larger than
//   dist_part times average distance.
//
// Global Variables: -
//
// Side Effects    : Memory operations, somewhat costly.
//
/----------------------------------------------------------------------*/

long ExampleSetSelectByDist(PStack_p results, ExampleSet_p set,
             Features_p target, double pred_w, double
             func_w, double *weights, long sel_no,
             double set_part, double dist_part)
{
   long             set_size = NumTreeNodes(set->ident_index),
      i, climit;
   double           dlimit, dist, avg;
   WeightedObject_p tmp_array = WeightedObjectArrayAlloc(set_size);
   PStack_p     stack;
   NumTree_p    cell;
   ExampleRep_p current;

   i = 0;
   avg = 0;
   stack = NumTreeTraverseInit(set->ident_index);
   while((cell = NumTreeTraverseNext(stack)))
   {
      current = cell->val1.p_val;
      dist = NumFeatureDistance(target, current->features, pred_w,
                                func_w, weights);
      tmp_array[i].weight       = dist;
      tmp_array[i].object.p_val = current;
      avg += dist;
      i++;
   }
   assert(i == set_size);
   NumTreeTraverseExit(stack);
   avg = avg /(double)set_size;

   WeightedObjectArraySort(tmp_array, set_size);
   climit = MIN(sel_no, set_part*set_size);
   dlimit = dist_part*avg;
   assert(climit <= set_size);
   for(i=0; i<climit && tmp_array[i].weight <= dlimit; i++)
   {
      current = tmp_array[i].object.p_val;
      if(Verbose)
      {
         fprintf(stderr, "Selected problem %ld: %s\n", current->ident,
                 current->name);
      }
      PStackPushInt(results, current->ident);
   }
   WeightedObjectArrayFree(tmp_array);
   return i;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
