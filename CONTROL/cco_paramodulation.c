/*-----------------------------------------------------------------------

File  : cco_paramodulation.c

Author: Stephan Schulz

Contents
 
  Controling paramodulation inferences.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 18:26:14 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "cco_paramodulation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


Clause_p variable_paramod(TB_p bank, OCB_p ocb, ClausePos_p from,
                          ClausePos_p into, VarBank_p freshvars, 
                          ParamodulationType pm_type, InfType *inf)
{
   Clause_p paramod = NULL;
   
   switch(pm_type)
   {
   case ParamodPlain:
         paramod = ClauseOrderedParamod(bank, ocb, from, into,
                                        freshvars);
         *inf = inf_paramod;
         break;
   case ParamodAlwaysSim:   
         paramod = ClauseOrderedSimParamod(bank, ocb, from, into,
                                           freshvars);
         *inf = inf_sim_paramod;         
         break;
   case ParamodOrientedSim:
         if(EqnIsOriented(from->literal))
         {
            paramod = ClauseOrderedSimParamod(bank, ocb, from, into,
                                              freshvars);
            *inf = inf_sim_paramod;         
         }
         else
         {
            paramod = ClauseOrderedParamod(bank, ocb, from, into,
                                           freshvars);
            *inf = inf_paramod;
         }
         break;
   default:
         assert(false && "Not yet implemented");
         break;
   }
   return paramod;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ComputeClauseClauseParamodulants()
//
//   Compute all (simultaneous) paramodulants between clause and with,
//   with terms from bank, and put them into store. Returns number of
//   paramodulants.
//
// Global Variables: -
//
// Side Effects    : As above...
//
/----------------------------------------------------------------------*/

long ComputeClauseClauseParamodulants(TB_p bank, OCB_p ocb, Clause_p
                                      clause, Clause_p parent_alias,
                                      Clause_p with, ClauseSet_p
                                      store, VarBank_p freshvars, 
                                      ParamodulationType pm_type)
{
   Clause_p    paramod;
   Term_p      test;
   ClausePos_p pos1, pos2;
   long        paramod_count = 0;
   InfType     inf_type = inf_noinf;


   if(ClauseQueryProp(clause, CPNoGeneration)||
      ClauseQueryProp(with, CPNoGeneration))
   {
      return 0;
   }

   pos1 = ClausePosAlloc();
   pos2 = ClausePosAlloc();

   test = ClausePosFirstParamodPair(clause, pos1, with, pos2,
				    false, pm_type != ParamodPlain);
   while(test)
   {
      assert(TermPosIsTopPos(pos1->pos));

      paramod = variable_paramod(bank, ocb, pos1, pos2,
                                 freshvars, pm_type, &inf_type);

      if(paramod)	    
      {
	 paramod_count++;
	 paramod->parent1 = parent_alias;	    
	 paramod->proof_size  =
	    parent_alias->proof_size+with->proof_size+1;
	 paramod->proof_depth = MAX(parent_alias->proof_depth,
				    with->proof_depth)+1;
	 ClauseSetTPTPType(paramod,
			   ClauseQueryTPTPType(parent_alias));
	 ClauseSetProp(paramod, 
		       ClauseGiveProps(parent_alias, CPIsSOS)|
		       ClauseGiveProps(with, CPIsSOS));
	 ClauseRegisterChild(parent_alias, paramod);
	 if(parent_alias!=with)
	 {
	    paramod->parent2 = with;
	    
	    ClauseSetTPTPType(paramod,
			      TPTPTypesCombine(
				 ClauseQueryTPTPType(paramod),
				 ClauseQueryTPTPType(with)));
	    
	    ClauseRegisterChild(with, paramod);
	 }
	 DocClauseCreationDefault(paramod, inf_type, with, 
				  parent_alias);
	 ClauseSetInsert(store, paramod);
      }
      test = ClausePosNextParamodPair(pos1, pos2, false, pm_type != ParamodPlain);
   }
   /* Paramod clause into with - no top positions this time ;-) */
   
   if((parent_alias==with)&&ClauseIsDemodulator(parent_alias))
   {
      /* Both clauses are identical, i.e. both cases are
	 symmetric. Ergo do nothing... */
   }
   else
   {
      test = ClausePosFirstParamodPair(with, pos1, clause, pos2, true, 
                                       pm_type != ParamodPlain); 
      
      while(test)
      {
	 assert(TermPosIsTopPos(pos1->pos));
         paramod = variable_paramod(bank, ocb, pos1, pos2,
                                    freshvars, pm_type, &inf_type);
	 if(paramod)
	 {
	    paramod_count++;
	    paramod->parent1 = with;
	    paramod->proof_size  =
	       parent_alias->proof_size+with->proof_size+1;
	    paramod->proof_depth = MAX(parent_alias->proof_depth,
				       with->proof_depth)+1;
	    ClauseSetTPTPType(paramod,
			      ClauseQueryTPTPType(with));
	    ClauseSetProp(paramod, 
			  ClauseGiveProps(parent_alias, CPIsSOS)|
			  ClauseGiveProps(with, CPIsSOS));
	    ClauseRegisterChild(with, paramod);
	    if(parent_alias!=with)
	    {
	       paramod->parent2 = parent_alias;
	       ClauseSetTPTPType(paramod,
				 TPTPTypesCombine(
				    ClauseQueryTPTPType(paramod),
				    ClauseQueryTPTPType(parent_alias)));
	       ClauseRegisterChild(parent_alias, paramod);
	    }
	    DocClauseCreationDefault(paramod, inf_type, parent_alias, with);
	    ClauseSetInsert(store, paramod);
	 }
	 test = ClausePosNextParamodPair(pos1, pos2, true, pm_type != ParamodPlain);
      }
   }
   ClausePosFree(pos1);
   ClausePosFree(pos2);
   
   return paramod_count;
}


/*-----------------------------------------------------------------------
//
// Function: ComputeAllParamodulants()
//
//   Compute all paramodulants between clause and with_set, put them
//   into store.
//
// Global Variables: -
//
// Side Effects    : By called functions, memory manangement.
//
/----------------------------------------------------------------------*/

long ComputeAllParamodulants(TB_p bank, OCB_p ocb, Clause_p clause,
			     Clause_p parent_alias, ClauseSet_p
			     with_set, ClauseSet_p store, VarBank_p
			     freshvars, ParamodulationType pm_type)
{
   Clause_p handle;
   long     paramod_count = 0;

   for(handle = with_set->anchor->succ; handle != with_set->anchor;
       handle = handle->succ)
   {
      paramod_count +=
	 ComputeClauseClauseParamodulants(bank, ocb, clause,
                                          parent_alias, handle,
                                          store, freshvars, pm_type);
   }
   return paramod_count;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


