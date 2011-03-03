 /*-----------------------------------------------------------------------

File  : che_axfilter.h

Author: Stephan Schulz (schulz@eprover.org)

Contents
 
  Code implementing axiom filters as a data type.

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Feb 24 02:33:05 CET 2011
    New

-----------------------------------------------------------------------*/

#include "che_axfilter.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* GeneralityMeasureNames[] =
{
   "None",
   "CountTerms",
   "CountLiterals",
   "CountFormulas",
   NULL
};

char* AxFilterDefaultSet =
   "bcr1=GSinE(CountTerms,    1.0,,,,)"
   "bcr2=GSinE(CountFormulas, 1.0,,,,)"
   "bcr3=GSinE(CountFormulas, 1.2,,,,)"
   "bcr4=GSinE(CountTerms,    1.2,,,,)";

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
// Function: AxFilterAlloc()
//
//   Allocate an empty, initialized axiom filter description.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

AxFilter_p AxFilterAlloc()
{
   AxFilter_p handle = AxFilterCellAlloc();
   
   handle->name                = NULL;
   handle->type                = AFNoFilter;
   handle->gen_measure         = GMNoMeasure;
   handle->benevolence         = 1.0;
   handle->generosity          = 1;
   handle->max_recursion_depth = LONG_MAX;
   handle->max_set_size        = LONG_LONG_MAX;
   handle->max_set_fraction    = 1.0;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: AxFilterFree()
//
//   Free an axiom filter description.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void AxFilterFree(AxFilter_p junk)
{
   if(junk->name)
   {
      FREE(junk->name);
   }
   AxFilterCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: AxFilterParse()
//
//   Parse an Axiom Filter description into a newly allocated cell.
//
//   The preliminary syntax is:
//
//    GSinE(<g-measure:type>, <benvolvence:double>, <generosity:int>, 
//    <rec-depth:int>, <set-size:int>, <set-fraction:double>)
//
//    where "GSinE" represents AFGSineE, "Generalized SinE", currently
//    the only support filter type. Other filter types can support
//    different formats. Names are not given here, but in the
//    AxFilterDefParse, see below. All numerical parameters are
//    optional, if ommitted, the default is used.
//
//
// Global Variables: -
//
// Side Effects    : IO, memory operations.
//
/----------------------------------------------------------------------*/

AxFilter_p AxFilterParse(Scanner_p in)
{
   AxFilter_p res = AxFilterAlloc();

   AcceptInpId(in, "GSinE");
   res->type = AFGSineE;
   AcceptInpTok(in, OpenBracket);
   CheckInpId(in, "CountFormulas|CountLiterals|CountTerms");
   if(TestInpId(in, "CountFormulas"))
   {
      res->gen_measure=GMFormulas;
   }
   else if(TestInpId(in, "CountTerms"))
   {
      res->gen_measure=GMTerms;
   }
   else
   {
      AktTokenError(in, "Generality measure not yet implemented", false);      
   }
   AcceptInpId(in, "CountFormulas|CountLiterals|CountTerms");
   AcceptInpTok(in, Comma);
   if(!TestInpTok(in, Comma))
   {
      res->benevolence = ParseFloat(in);
   }
   AcceptInpTok(in, Comma);
   
   if(!TestInpTok(in, Comma))
   {
      res->generosity = AktToken(in)->numval;
      AcceptInpTok(in, PosInt);
   }
   AcceptInpTok(in, Comma);
   if(!TestInpTok(in, Comma))
   {
      res->max_recursion_depth = AktToken(in)->numval;
      AcceptInpTok(in, PosInt);
   }
   AcceptInpTok(in, Comma);
   if(!TestInpTok(in, Comma))
   {
      res->max_set_size = AktToken(in)->numval;
      AcceptInpTok(in, PosInt);
   }
   AcceptInpTok(in, Comma); 
   if(!TestInpTok(in, CloseBracket))
   {
      res->max_set_fraction = ParseFloat(in);
   }
   AcceptInpTok(in, CloseBracket);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: AxFilterDefParse()
//
//   Parse an AxFilterDefinition of the form [name=]<def>, where
//   "name" is an Identifier, and <def> is an axiom filter
//   definition. If the optional part is missing, an automatically
//   generated name of the form "axfilter_auto%4udd" is
//   generated. This name is unique among auto-generated names (up to
//   the period of unsigned long, but not checked against manually
//   given names. 
//
// Global Variables: -
//
// Side Effects    : May update local static counter
//
/----------------------------------------------------------------------*/

AxFilter_p AxFilterDefParse(Scanner_p in)
{
   static unsigned long ax_id_count = 0;
   char* name;
   char  anon_name[256];
   AxFilter_p res;

   if(TestTok(LookToken(in,1), EqualSign))
   {
      CheckInpTok(in, Identifier);
      name = SecureStrdup(DStrView(AktToken(in)->literal)); 
      NextToken(in);
      AcceptInpTok(in, EqualSign);      
   }
   else
   {
      sprintf(anon_name, "axfilter_auto%4lu", ax_id_count++);
      name = SecureStrdup(anon_name);
   }
   res = AxFilterParse(in);
   res->name = name;
   
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: AxFilterPrint()
//
//   Print an axiom filter specification.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void AxFilterPrint(FILE* out, AxFilter_p filter)
{
   fprintf(out, "%s(%s, %f, %ld, %ld, %lld, %f)",
           "GSinE", 
           GeneralityMeasureNames[filter->gen_measure],
           filter->benevolence,
           filter->generosity,
           filter->max_recursion_depth,
           filter->max_set_size,
           filter->max_set_fraction);
}


/*-----------------------------------------------------------------------
//
// Function: AxFilterDefPrint()
//
//   Print an axiom filter defintion
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void AxFilterDefPrint(FILE* out, AxFilter_p filter)
{
   fprintf(out, "%s = ", filter->name);
   AxFilterPrint(out, filter);
}


/*-----------------------------------------------------------------------
//
// Function: AxFilterSetAlloc()
//
//   Allocate an empy AxFilterSet.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

AxFilterSet_p AxFilterSetAlloc()
{   
   AxFilterSet_p set = AxFilterSetCellAlloc();
   
   set->set = PStackAlloc();
   return set;   
}


/*-----------------------------------------------------------------------
//
// Function: AxFilterSetFree()
//
//   Free an axion filter set (including the filters).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void AxFilterSetFree(AxFilterSet_p junk)
{
   AxFilter_p filter;

   while(!PStackEmpty(junk->set))
   {
      filter = PStackPopP(junk->set);
      AxFilterFree(filter);
   }
   PStackFree(junk->set);
   AxFilterSetCellFree(junk);

}

/*-----------------------------------------------------------------------
//
// Function: AxFilterSetParse()
//
//   Parse a set of axfilter definitions. Returns number of filters
//   parsed. 
//
// Global Variables: -
//
// Side Effects    : Input, memory operations
//
/----------------------------------------------------------------------*/

long AxFilterSetParse(Scanner_p in, AxFilterSet_p set)
{
   long res = 0;
   AxFilter_p filter;
   
   while(TestInpTok(in, Identifier))
   {
      filter = AxFilterDefParse(in);
      PStackPushP(set->set, filter);
      res++;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: AxFilterSetCreateInternal()
//
//   Create and return an AxFilterSet from a provided string
//   description. 
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

AxFilterSet_p AxFilterSetCreateInternal(char* str)
{
   AxFilterSet_p filters = AxFilterSetAlloc();   
   Scanner_p in = CreateScanner(StreamTypeInternalString, str, true, NULL);
   AxFilterSetParse(in, filters);
   DestroyScanner(in); 
   return filters;
}



/*-----------------------------------------------------------------------
//
// Function: AxFilterSetPrint()
//
//   Print a set of axfilter definitions.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void AxFilterSetPrint(FILE* out, AxFilterSet_p set)
{
   PStackPointer i;

   for(i=0; i<PStackGetSP(set->set); i++)
   {
      AxFilterDefPrint(out, PStackElementP(set->set, i));
      fprintf(out, "\n");
   }
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


