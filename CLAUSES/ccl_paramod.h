/*-----------------------------------------------------------------------

File  : ccl_paramod.h

Author: Stephan Schulz

Contents

  Interface for paramodulating termpairs into termpairs and clauses
  into clauses.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed May 20 15:14:26 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCL_PARAMOD

#define CCL_PARAMOD

#include <ccl_clausesets.h>
#include "ccl_clausecpos.h"
#include <cte_replace.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef enum
{
   ParamodPlain,                /* Use standard paramodulation */
   ParamodAlwaysSim,            /* Always use simultaneous paramod */
   ParamodOrientedSim,          /* Use simultaneous if rw-literal is
                                   oriented */
   /* The rest not yet implemented */
   ParamodDecreasingSim,        /* Use sim if rw-literal instance is
                                   orientable */
   ParamodSizeDecreasingSim     /* Use sim if instantiated RHS is
                                   smaller */
}ParamodulationType;

typedef struct
{
   TB_p        bank;
   OCB_p       ocb;
   VarBank_p   freshvars;
   Clause_p    new_orig;
   Clause_p    from;
   CompactPos  from_cpos;
   ClausePos_p from_pos;
   Clause_p    into;
   CompactPos  into_cpos;
   ClausePos_p into_pos;
}ParamodInfoCell, *ParamodInfo_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


void     ParamodInfoPrint(FILE* out, ParamodInfo_p info);

Clause_p ClausePlainParamodConstruct(ParamodInfo_p ol_desc);
Clause_p ClauseSimParamodConstruct(ParamodInfo_p ol_desc);
Clause_p ClauseParamodConstruct(ParamodInfo_p ol_desc, bool sim_pm);


Term_p ComputeOverlap(TB_p bank, OCB_p ocb, ClausePos_p from, Term_p
            into, TermPos_p pos,  Subst_p subst, VarBank_p
            freshvars);

Eqn_p  EqnOrderedParamod(TB_p bank, OCB_p ocb, ClausePos_p from,
          ClausePos_p into, Subst_p subst, VarBank_p
          freshvars);

Clause_p ClauseOrderedParamod(TB_p bank, OCB_p ocb, ClausePos_p
               from,ClausePos_p into, VarBank_p
               freshvars);

Clause_p ClauseOrderedSimParamod(TB_p bank, OCB_p ocb, ClausePos_p
                                 from,ClausePos_p into, VarBank_p
                                 freshvars);

Term_p   ClausePosFirstParamodInto(Clause_p clause, ClausePos_p pos,
               ClausePos_p from_pos, bool no_top,
                                   bool simu_paramod);
Term_p   ClausePosNextParamodInto(ClausePos_p pos, ClausePos_p
              from_pos, bool no_top);

Term_p   ClausePosFirstParamodFromSide(Clause_p from, ClausePos_p
                   from_pos);
Term_p   ClausePosNextParamodFromSide(ClausePos_p from_pos);

Term_p   ClausePosFirstParamodPair(Clause_p from, ClausePos_p
               from_pos, Clause_p into,
               ClausePos_p into_pos, bool no_top,
                                   bool simu_paramod);
Term_p   ClausePosNextParamodPair(ClausePos_p from_pos, ClausePos_p
              into_pos, bool no_top, bool simu_paramod);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





