/*-----------------------------------------------------------------------

File  : che_auto_cases.c

Author: Stephan Schulz

Contents
 
  Case distinction for auto mode, included by both auto heuristic
  selection and auto ordering selection

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue May  2 22:07:17 GMT 2000
    Extracted from che_heuristics.c

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
    
{
SpecFeaturesAddEval(spec,1, 3, true, 20, 100,
		       15, 100, 60, 1000);
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
#include "che_X_____auto_pp_opt.c"         
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

