/*-----------------------------------------------------------------------

File  : che_auto_cases.c

Author: Stephan Schulz

Contents

  Case distinction for auto mode, included by both auto heuristic
  selection and auto ordering selection

  Copyright 1998-2013 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
   SpecTypePrint(GlobalOut, spec, DEFAULT_CLASS_MASK);
   fputc('\n', GlobalOut);
}
if(false)
{
   assert(false);
}
#ifdef CHE_HEURISTICS_AUTO
#include "che_X_____auto.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_CASC
#include "che_X_____auto_casc.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_DEV
#include "che_X_____auto_dev.c"
#endif

#ifdef CHE_HEURISTICS_AUTO_SCHED0
#include "che_X_auto_sched0.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED1
#include "che_X_auto_sched1.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED2
#include "che_X_auto_sched2.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED3
#include "che_X_auto_sched3.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED4
#include "che_X_auto_sched4.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED5
#include "che_X_auto_sched5.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED6
#include "che_X_auto_sched6.c"
#endif
#ifdef CHE_HEURISTICS_AUTO_SCHED7
#include "che_X_auto_sched7.c"
#endif



}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

