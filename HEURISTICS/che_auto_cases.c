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

    
{
SpecFeaturesAddEval(spec, limits);
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
#ifdef CHE_HEURISTICS_AUTO
#include "che_X_____auto.c"         
#endif
#ifdef CHE_HEURISTICS_AUTO_071
#include "che_X_____auto_pp_opt_071.c"         
#endif
#ifdef CHE_HEURISTICS_AUTO_DEV
#include "che_X_____auto_dev.c"         
#endif



}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

