/*-----------------------------------------------------------------------

File  : csl_auto_cases.c

Author: Stephan Schulz

Contents
 
  Case distinction for auto mode, included by both auto heuristic
  selection and auto ordering selection

  Copyright 1998, 1999, 2002 by the author.

Changes

<1> Tue May  2 22:07:17 GMT 2000
    Extracted from che_heuristics.c
<2> Fri Aug  2 10:56:06 MEST 2002
    Modified from HEURISTICS/che_auto_cases.c

-----------------------------------------------------------------------*/

/* The calling sequence for SpecFeaturesAddEval is
   spec
   NG_UNIT_FEW_LIMIT
   NG_UNIT_MANY_LIMIT
   Absolute limit?
   ax-few-limit
   ax-some-limit
   lit-few-limit
   lit-some-limit
   term-few-limit
   term-some-limit */
    

if(SpecAxiomsAreUnit(spec))
{
   OUTPRINT(1, "# Axioms are unit-clauses.\n");
   SpecFeaturesAddEval(spec,3, 14, true, 4, 15,
		       5, 15, 51, 126);
   if(OutputLevel)
   {
      fprintf(GlobalOut, "# Problem is type ");
      SpecTypePrint(GlobalOut, spec, "aaaaaaaaaa");
      fputc('\n', GlobalOut);
   }
   if(false)
   {
      assert(false);
   }
#include "csl_U_____auto.c"         
}
else if(SpecAxiomsAreHorn(spec))
{
   OUTPRINT(1, "# Axioms are Horn-clauses.\n");
   SpecFeaturesAddEval(spec, 1,3,true, 9, 20, 13, 44, 78, 179);
   if(OutputLevel)
   {
      fprintf(GlobalOut, "# Problem is type ");
      SpecTypePrint(GlobalOut, spec, "aaaaaaaaaa");
      fputc('\n', GlobalOut);
   }
   if(false)
   {
	 assert(false);
   }
#include "csl_H_____auto.c" 
}
else
{
   if(SpecNoEq(spec))
   {
      OUTPRINT(1, "# Axioms are general clauses.\n");      
      SpecFeaturesAddEval(spec, FEW_DEFAULT, MANY_DEFAULT, false,
			  30, 43, 77, 100, 250, 296);
      
      if(OutputLevel)
      {
	 fprintf(GlobalOut, "# Problem is type ");
	 SpecTypePrint(GlobalOut, spec, "aaaaaaaaaa");
	 fputc('\n', GlobalOut);
      }
      if(false)
      {
	 assert(false);
      }
#include "csl_G_N___auto.c" 
   }
   else
   {
      OUTPRINT(1, "# Axioms are general clauses.\n");      
      SpecFeaturesAddEval(spec, 2, 5, true,
			  113, 160, 219, 325, 1153, 1644);
      
      if(OutputLevel)
      {
	 fprintf(GlobalOut, "# Problem is type ");
	 SpecTypePrint(GlobalOut, spec, "aaaaaaaaaa");
	 fputc('\n', GlobalOut);
      }
      
      if(false)
      {
	 assert(false);
      }
#include "csl_G_E___auto.c"    
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

