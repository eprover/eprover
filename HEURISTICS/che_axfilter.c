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

/* The order of this has to match the order of values in the
 * definition of GeneralityMeasure in the header file */

char* GeneralityMeasureNames[] =
{
   "None",
   "CountTerms",
   "CountLiterals",
   "CountFormulas",
   "CoutPosFormulas",
   "CountPosLiterals",
   "CountPosTerms",
   "CoutNegFormulas",
   "CountNegLiterals",
   "CountNegTerms",
   NULL
};

char* AxFilterDefaultSet ="\
   threshold010000=Threshold(10000)\
\
   LambdaDef=LambdaDef\
\
   gf500_gu_R04_F100_L20000=GSinE(CountFormulas, ,false,   5.0,, 4,20000,1.0)\
   gf120_gu_RUU_F100_L00500=GSinE(CountFormulas, ,false,   1.2,,,  500,1.0)\
   gf120_gu_R02_F100_L20000=GSinE(CountFormulas, ,false,   1.2,, 2,20000,1.0)\
   gf150_gu_RUU_F100_L20000=GSinE(CountFormulas, ,false,   1.5,,,20000,1.0)\
   gf120_gu_RUU_F100_L00100=GSinE(CountFormulas, ,false,   1.2,,,  100,1.0)\
   gf200_gu_R03_F100_L20000=GSinE(CountFormulas, ,false,   2.0,, 3,20000,1.0)\
   gf600_gu_R05_F100_L20000=GSinE(CountFormulas, ,false,   6.0,, 5,20000,1.0, false)\
   gf200_gu_RUU_F100_L20000=GSinE(CountFormulas, ,false,   2.0,,  ,20000,1.0)\
   gf120_gu_RUU_F100_L01000=GSinE(CountFormulas, ,false,   1.2,,  , 1000,1.0, false)\
\
   gf500_h_gu_R04_F100_L20000=GSinE(CountFormulas, hypos,false,   5.0,, 4,20000,1.0, false)\
   gf120_h_gu_RUU_F100_L00500=GSinE(CountFormulas, hypos,false,   1.2,,,  500,1.0)\
   gf120_h_gu_R02_F100_L20000=GSinE(CountFormulas, hypos,false,   1.2,, 2,20000,1.0)\
   gf150_h_gu_RUU_F100_L20000=GSinE(CountFormulas, hypos,false,   1.5,,,20000,1.0)\
   gf120_h_gu_RUU_F100_L00100=GSinE(CountFormulas, hypos,false,   1.2,,,  100,1.0)\
   gf200_h_gu_R03_F100_L20000=GSinE(CountFormulas, hypos,false,   2.0,, 3,20000,1.0)\
   gf600_h_gu_R05_F100_L20000=GSinE(CountFormulas, hypos,false,   6.0,, 5,20000,1.0,false)\
   gf200_h_gu_RUU_F100_L20000=GSinE(CountFormulas, hypos,false,   2.0,,  ,20000,1.0)\
   gf120_h_gu_RUU_F100_L01000=GSinE(CountFormulas, hypos,false,   1.2,,  , 1000,1.0)\
\
   gf600_gu_R05_F100_L20000add=GSinE(CountFormulas, ,false,   6.0,, 5,20000,1.0,addnosymb)\
";

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: get_gen_measure()
//
//   Given a string, return the corresponding GenMeasure, or 0 on
//   failure.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

GeneralityMeasure get_gen_measure(char* str)
{
   int res = StringIndex(str, GeneralityMeasureNames);

   if(res==-1)
   {
      res=0;
   }
   return (GeneralityMeasure) res;
}

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

AxFilter_p AxFilterAlloc(void)
{
   AxFilter_p handle = AxFilterCellAlloc();

   handle->name                 = NULL;
   handle->type                 = AFNoFilter;
   handle->gen_measure          = GMNoMeasure;
   handle->use_hypotheses       = false;
   handle->benevolence          = 1.0;
   handle->generosity           = LONG_MAX;
   handle->max_recursion_depth  = INT_MAX;
   handle->max_set_size         = LONG_MAX; /* LONG LONG MAX is
                                              problematic */
   handle->max_set_fraction     = 1.0;
   handle->add_no_symbol_axioms = false;
   handle->trim_implications    = false;
   handle->defined_symbols_in_drel = false; // if

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
// Function: GSinEParse()
//
//   Parse an Axiom Filter description into a newly allocated cell.
//
//   The preliminary syntax is:
//
//    GSinE(<g-measure:type>, <[no]hypos>,<benvolvence:double>,
//    <generosity:int>, <rec-depth:int>, <set-size:int>,
//    <set-fraction:double>)
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

AxFilter_p GSinEParse(Scanner_p in)
{
   AxFilter_p res = AxFilterAlloc();

   AcceptInpId(in, "GSinE");
   res->type = AFGSinE;
   AcceptInpTok(in, OpenBracket);

   res->gen_measure = get_gen_measure(DStrView(AktToken(in)->literal));

   if(!res->gen_measure)
   {
      AktTokenError(in, "Unknown generality measure", false);
   }
   if(res->gen_measure!=GMTerms && res->gen_measure!=GMFormulas)
   {
      AktTokenError(in, "Generality measure not yet implemented", false);
   }
   NextToken(in);
   AcceptInpTok(in, Comma);

   if(!TestInpTok(in, Comma))
   {
      CheckInpId(in, "hypos|nohypos");
      if(TestInpId(in, "hypos"))
      {
         res->use_hypotheses = true;
      }
      NextToken(in);
   }
   AcceptInpTok(in, Comma);
   if(!TestInpTok(in, Comma) && TestInpId(in,"true|false"))
   {
      res->defined_symbols_in_drel = TestInpId(in, "true");
      AcceptInpId(in, "true|false");
      AcceptInpTok(in, Comma);
   }
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
   if(!TestInpTok(in, CloseBracket|Comma))
   {
      res->max_set_fraction = ParseFloat(in);
   }
   if(TestInpTok(in, Comma) && TestId(LookToken(in, 1), "addnosymb|ignorenosymb"))
   {
      AcceptInpTok(in, Comma);
      res->add_no_symbol_axioms = TestInpId(in, "addnosymb");
      AcceptInpId(in, "addnosymb|ignorenosymb");
   }
   if(TestInpTok(in, Comma) && TestId(LookToken(in, 1), "true|false"))
   {
      AcceptInpTok(in, Comma);
      res->trim_implications = TestInpId(in, "true");
      AcceptInpId(in, "true|false");
   }

   AcceptInpTok(in, CloseBracket);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: ThresholdParse()
//
//   Parse an Threshold filter
//
//   The preliminary syntax is:
//
//    Threshold(<threshold:int>)
//
// Global Variables: -
//
// Side Effects    : IO, memory operations.
//
/----------------------------------------------------------------------*/

AxFilter_p ThresholdParse(Scanner_p in)
{
   AxFilter_p res = AxFilterAlloc();

   AcceptInpId(in, "Threshold");
   res->type = AFThreshold;
   AcceptInpTok(in, OpenBracket);

   res->threshold = AktToken(in)->numval;
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, CloseBracket);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: LambdaDefParse()
//
//   Parse an LambdaDef filter: has no arguments
//
//
// Global Variables: -
//
// Side Effects    : IO, memory operations.
//
/----------------------------------------------------------------------*/

AxFilter_p LambdaDefParse(Scanner_p in)
{
   AxFilter_p res = AxFilterAlloc();

   AcceptInpId(in, "LambdaDef");
   res->type = AFLambdaDefines;
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: AxFilterParse()
//
//   Parse an AxFilter and return it.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory, all via subfunctions
//
/----------------------------------------------------------------------*/

AxFilter_p AxFilterParse(Scanner_p in)
{
   CheckInpId(in, "GSinE|Threshold|LambdaDef");

   if(TestInpId(in, "GSinE"))
   {
      return GSinEParse(in);
   }
   if(TestInpId(in, "Threshold"))
   {
      return ThresholdParse(in);
   }
   if(TestInpId(in, "LambdaDef"))
   {
      return LambdaDefParse(in);
   }
   return NULL;
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
// Function: AxFilterPrintBuf()
//
//   Print an axiom filter specification into a buffer. Return true on
//   success, false if the buffer is too small.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

bool AxFilterPrintBuf(char* buf, int buflen, AxFilter_p filter)
{
   int res = 0;

   switch(filter->type)
   {
   case AFGSinE:
         res = snprintf(buf, buflen, "%s(%s, %s, %s, %f, %ld, %ld, %lld, %f, %s, %s)",
                        "GSinE",
                        GeneralityMeasureNames[filter->gen_measure],
                        filter->use_hypotheses?"hypos":"nohypos",
                        filter->defined_symbols_in_drel?"true":"false",
                        filter->benevolence,
                        filter->generosity,
                        filter->max_recursion_depth,
                        filter->max_set_size,
                        filter->max_set_fraction,
                        filter->add_no_symbol_axioms?"addnosymb":"ignorenosymb",
                        filter->trim_implications?"true":"false");
         break;
   case AFThreshold:
         res = snprintf(buf, buflen, "Threshold(%ld)",
                        filter->threshold);
         break;
   case AFLambdaDefines:
         res = snprintf(buf, buflen, "LambdaDef");
   default:
         assert(false && "Unknown AxFilter type");
         break;
   }
   return (res<buflen);
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
   int size = 80;
   bool success = false;
   char* buf;

   while(!success)
   {
      buf = SecureMalloc(size);
      success =  AxFilterPrintBuf(buf, size, filter);
      if(success)
      {
         fprintf(out, "%s", buf);
      }
      FREE(buf);
      size *= 2;
   }
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

AxFilterSet_p AxFilterSetAlloc(void)
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
      AxFilterSetAddFilter(set, filter);
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
   Scanner_p in = CreateScanner(StreamTypeInternalString, str, true,
                                NULL, true);
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


/*-----------------------------------------------------------------------
//
// Function: AxFilterSetFindFilter()
//
//   Given a name, return the filter (or NULL).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

AxFilter_p AxFilterSetFindFilter(AxFilterSet_p set, char* name)
{
   PStackPointer i;
   AxFilter_p    res=NULL, tmp;

   for(i=0; i<PStackGetSP(set->set); i++)
   {
      tmp = AxFilterSetGetFilter(set, i);
      if(strcmp(tmp->name, name)==0)
      {
         res = tmp;
         break;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: AxFilterSetAddNames()
//
//   Add the names of all filters in the set to the provided DStr.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void AxFilterSetAddNames(DStr_p res, AxFilterSet_p filters)
{
   PStackPointer i;
   AxFilter_p    tmp;
   char*         sep = "";

   for(i=0; i<PStackGetSP(filters->set); i++)
   {
      tmp = AxFilterSetGetFilter(filters, i);

      DStrAppendStr(res, sep);
      DStrAppendStr(res, tmp->name);
      sep = ", ";
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
