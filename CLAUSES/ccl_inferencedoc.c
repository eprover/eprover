/*-----------------------------------------------------------------------

File  : ccl_inferencedoc.c

Author: Stephan Schulz

Contents
 
  Functions and constants for reporting on the proof process.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Tue Jan  5 16:15:48 MET 1999
    New

-----------------------------------------------------------------------*/

#include "ccl_inferencedoc.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

OutputFormatType OutputFormat   = pcl_format;
bool             PCLFullTerms   = true;
bool             PCLStepCompact = false;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: pcl_print_start()
//
//   Print the "<id> : <clause> : " part of a pcl step.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void pcl_print_start(FILE* out, Clause_p clause)
{
   fprintf(out, PCLStepCompact?"%ld:":"%6ld : ", clause->ident);
   ClausePCLPrint(out, clause, PCLFullTerms);
   fputs(" : ", out);
}

/*-----------------------------------------------------------------------
//
// Function: pcl_print_end()
//
//   Print the optional comment and new line
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void pcl_print_end(FILE* out, char* comment, Clause_p clause)
{
   if(ClauseQueryProp(clause, CPWatchOnly)&&comment)
   {
      fprintf(out, PCLStepCompact?":'wl,%s'" : ": 'wl,%s'",
	      comment);
   }
   else if(comment)
   {
      fprintf(out, PCLStepCompact?":'%s'":" : '%s'",comment);
   }
   else if(ClauseQueryProp(clause, CPWatchOnly))
   {
      fprintf(out, PCLStepCompact?":'wl'":" : 'wl'");
   }   
   fputc('\n', out);
}


/*-----------------------------------------------------------------------
//
// Function: tstp_print_end()
//
//   Print the optional comment and new line
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void tstp_print_end(FILE* out, char* comment, Clause_p clause)
{
   if(ClauseQueryProp(clause, CPWatchOnly)&&comment)
   {
      fprintf(out, ",['wl,%s']", comment);
   }
   else if(comment)
   {
      fprintf(out, ",['%s']", comment);
   }
   else if(ClauseQueryProp(clause, CPWatchOnly))
   {
      fprintf(out, ",['wl']");
   }   
   fputs(").\n", out);
}


/*-----------------------------------------------------------------------
//
// Function: print_initial()
//
//   Print an initial clause (axiom).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_initial(FILE* out, Clause_p clause, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, "initial");
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, ", unknown");
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }
}



/*-----------------------------------------------------------------------
//
// Function: print_paramod()
//
//   Print a clause creation by paramodulation (or superposition).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_paramod(FILE* out, Clause_p clause, Clause_p
			  parent1, Clause_p parent2, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_PM"(%ld,%ld)", parent1->ident,
		 parent2->ident);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_PM",[status(thm)],[c_0_%ld,c_0_%ld,theory(equality)])",
		 parent1->ident,
		 parent2->ident);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}


/*-----------------------------------------------------------------------
//
// Function: print_eres()
//
//   Print a clause creation by equality resolution.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_eres(FILE* out, Clause_p clause, Clause_p
			  parent1, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_ER"(%ld)", parent1->ident);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_ER",[status(thm)],[c_0_%ld,theory(equality)])",
		 parent1->ident);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}


/*-----------------------------------------------------------------------
//
// Function: print_efactor()
//
//   Print a clause creation by equality factoring.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_efactor(FILE* out, Clause_p clause, Clause_p
			  parent1, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_EF"(%ld)", parent1->ident);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_EF",[status(thm)],[c_0_%ld,theory(equality)])",
		 parent1->ident);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}


/*-----------------------------------------------------------------------
//
// Function: print_factor()
//
//   Print a clause creation by (ordinary) factoring.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_factor(FILE* out, Clause_p clause, Clause_p
			  parent1, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_OF"(%ld)", parent1->ident);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_OF",[status(thm)],[c_0_%ld,theory(equality:s)])",
		 parent1->ident);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}



/*-----------------------------------------------------------------------
//
// Function: print_split()
//
//   Print a clause creation by splitting.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_split(FILE* out, Clause_p clause, Clause_p
			  parent1, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_SPLIT"(%ld)", parent1->ident);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("
                 TSTP_SPLIT_BASE
                 ",["TSTP_SPLIT_BASE"("TSTP_SPLIT_REFINED
                 ",[])],[c_0_%ld])",
		 parent1->ident);
	 tstp_print_end(out, comment, clause);
	 break;
    default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}

/*-----------------------------------------------------------------------
//
// Function: print_simplify_reflect()
//
//   Print a clause modification by simplify-reflect.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_simplify_reflect(FILE* out, Clause_p clause, long
				   old_id, Clause_p partner, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_SR"(%ld,%ld)", old_id,
		 partner->ident);
	 pcl_print_end(out, comment, clause);
	 break;
  case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_SR",[status(thm)],[c_0_%ld,c_0_%ld,theory(equality)])",
		 old_id,
		 partner->ident);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}

/*-----------------------------------------------------------------------
//
// Function: print_context_simplify_reflect()
//
//   Print a clause modification by contextual simplify-reflect.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_context_simplify_reflect(FILE* out, Clause_p clause, long
				   old_id, Clause_p partner, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_CSR"(%ld,%ld)", old_id,
		 partner->ident);
	 pcl_print_end(out, comment, clause);
	 break;
 case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_CSR
                 ",[status(thm)],[c_0_%ld,c_0_%ld,theory(equality:s)])",
		 old_id,
		 partner->ident);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}


/*-----------------------------------------------------------------------
//
// Function: print_ac_res()
//
//   Print a clause modification by AC-resolution.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_ac_res(FILE* out, Clause_p clause, long
			 old_id, Sig_p sig, char* comment)
{
   PStackPointer i, sp;

   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_ACRES"(%ld", old_id);
	 assert(!PStackEmpty(sig->ac_axioms));
	 sp = PStackGetSP(sig->ac_axioms);
	 for(i=0; i< sp; i++)
	 {
	    fprintf(out, ",%ld", PStackElementInt(sig->ac_axioms,i));
	 }
	 fputc(')', out);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	 fprintf(out, 
		 ", inference("PCL_ACRES
                 ",[status(thm)],[c_0_%ld,theory(equality)", old_id);
	 assert(!PStackEmpty(sig->ac_axioms));
	 sp = PStackGetSP(sig->ac_axioms);
	 for(i=0; i< sp; i++)
	 {
	    fprintf(out, ",c_0_%ld", PStackElementInt(sig->ac_axioms,i));
	 }
	 fputs("])", out);
	 tstp_print_end(out, comment, clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}


/*-----------------------------------------------------------------------
//
// Function: print_minimize()
//
//   Print a clause modification by clause-internal simplification
//   (elemination of redundant literals)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_minimize(FILE* out, Clause_p clause, long
			 old_id, char* comment)
{
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, clause);
	 fprintf(out, PCL_CN"(%ld)", old_id);
	 pcl_print_end(out, comment, clause);
	 break;
   case tstp_format:
         ClauseTSTPPrint(out, clause, PCLFullTerms, false);
         fprintf(out, 
                 ", inference("PCL_CN",[status(thm)],[c_0_%ld, theory(equality:s)])", 
                 old_id);
         tstp_print_end(out, comment, clause);
         break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }   
}

/*-----------------------------------------------------------------------
//
// Function: print_rewrite()
//
//   Print a series of rewrite steps.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_rewrite(FILE* out, ClausePos_p rewritten, long
			  old_id, Term_p old_term, char* comment)
{
   PStack_p rwsteps = PStackAlloc();
   PStackPointer i;
   bool     tmp;
   Term_p   nf = ClausePosGetSide(rewritten);

   /* printf("Rewrite: ");
   TBPrintTerm(stdout, rewritten->literal->bank, old_term, true);
   printf(" => ");
   TBPrintTerm(stdout, rewritten->literal->bank, nf, true);
   printf(" \n"); */

   switch(OutputFormat)
   {
   case pcl_format:
	 tmp = TermComputeRWSequence(rwsteps, old_term, nf);
	 assert(tmp);
	 pcl_print_start(out, rewritten->clause);
	 for(i=0; i<PStackGetSP(rwsteps); i++)
	 {
	    fputs(PCL_RW"(", out);
	 }
	 fprintf(out, "%ld", old_id);
	 for(i=0; i<PStackGetSP(rwsteps); i++)
	 {
	    fprintf(out, ",%ld)", PStackElementInt(rwsteps,i));
	 }
	 pcl_print_end(out, comment, rewritten->clause);
	 break;
   case tstp_format:
	 tmp = TermComputeRWSequence(rwsteps, old_term, nf);
	 assert(tmp);
	 ClauseTSTPPrint(out, rewritten->clause, PCLFullTerms, false);
	 fputc(',', out);
	 for(i=0; i<PStackGetSP(rwsteps); i++)
	 {
	    fprintf(out,"inference("PCL_RW", [status(thm)],[");
	 }
	 fprintf(out, "c_0_%ld", old_id);
	 for(i=0; i<PStackGetSP(rwsteps); i++)
	 {
	    fprintf(out, ",c_0_%ld,theory(equality)])", 
		    PStackElementInt(rwsteps,i));
	 }
	 tstp_print_end(out, comment, rewritten->clause);
	 break;
   default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }
   PStackFree(rwsteps);
}


/*-----------------------------------------------------------------------
//
// Function: print_eq_unfold()
//
//   Print a series of eq-unfoldings with demod.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_eq_unfold(FILE* out, Clause_p rewritten, 
			    long old_id, ClausePos_p demod, 
			    PStack_p demod_pos)
{
   PStackPointer i;
   
   switch(OutputFormat)
   {
   case pcl_format:
	 pcl_print_start(out, rewritten);
	 for(i=0; i<PStackGetSP(demod_pos); i++)
	 {
	    fputs(PCL_RW"(", out);
	 }
	 fprintf(out, "%ld", old_id);
	 for(i=0; i<PStackGetSP(demod_pos); i++)
	 {
	    fprintf(out, ",%ld)", demod->clause->ident);
	 }
	 pcl_print_end(out, "unfolding", rewritten);
	 break;
   case tstp_format:
	 ClauseTSTPPrint(out, rewritten, PCLFullTerms, false);
	 fputc(',', out);
	 for(i=0; i<PStackGetSP(demod_pos); i++)
	 {
	    fprintf(out,"inference("PCL_RW", [status(thm)],[");
	 }
	 fprintf(out, "c_0_%ld", old_id);
	 for(i=0; i<PStackGetSP(demod_pos); i++)
	 {
	    fprintf(out, ",c_0_%ld,theory(equality)])", 
		    PStackElementInt(demod_pos,i));
	 }
	 tstp_print_end(out, "Unfolding", rewritten);
	 break;
    default:
	 fprintf(out, "# Output format not implemented.\n");
	 break;
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: DocClauseCreation()
//
//   Document the creation of a new clause if level >=2
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for clause
//
/----------------------------------------------------------------------*/

void DocClauseCreation(FILE* out, long level, Clause_p clause,
		       InfType op, Clause_p parent1,
		       Clause_p parent2, char* comment)
{
   if(level >= 2)
   {
      switch(op)
      {
      case inf_initial:
	    assert(!parent1);
	    assert(!parent2);
	    clause->ident = ++ClauseIdentCounter;
	    print_initial(out, clause, comment);
	    break;
      case inf_paramod:
	    assert(parent1);
	    assert(parent2);
	    clause->ident = ++ClauseIdentCounter;
	    print_paramod(out, clause, parent1, parent2, comment);
	    break;
      case inf_eres:
	    assert(parent1);
	    assert(!parent2);
	    clause->ident = ++ClauseIdentCounter;
	    print_eres(out, clause, parent1, comment);
	    break;
      case inf_efactor:
	    assert(parent1);
	    assert(!parent2);
	    clause->ident = ++ClauseIdentCounter;
	    print_efactor(out, clause, parent1, comment);
	    break;
      case inf_factor: /* Should not really occur, just for
			  completeness */
	    assert(parent1);
	    assert(!parent2);
	    clause->ident = ++ClauseIdentCounter;
	    print_factor(out, clause, parent1, comment);
	    break;
      case inf_split:
	    assert(parent1);
	    assert(!parent2);
	    clause->ident = ++ClauseIdentCounter;
	    print_split(out, clause, parent1, comment);
	    break;
      default:
	    assert(false&&"Unsupported clause creation operation???");
	    break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseModification()
//
//   Document the modification of a clause.
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for clause
//
/----------------------------------------------------------------------*/

void DocClauseModification(FILE* out, long level, Clause_p clause, InfType
		     op, Clause_p partner, Sig_p sig, char* comment)
{
   long old_id;

   if(level >= 2)
   {
      assert(clause);
      old_id = clause->ident;

      switch(op)
      {
      case inf_simplify_reflect:
	    assert(clause);
	    assert(partner);
	    clause->ident = ++ClauseIdentCounter;
	    print_simplify_reflect(out, clause, old_id, partner,
				   comment);
	    break;
      case inf_context_simplify_reflect:
	    assert(clause);
	    assert(partner);
	    clause->ident = ++ClauseIdentCounter;
	    print_context_simplify_reflect(out, clause, old_id, partner,
				   comment);
	    break;
      case inf_ac_resolution:
	    assert(clause);
	    assert(!partner);
	    clause->ident = ++ClauseIdentCounter;
	    print_ac_res(out, clause, old_id, sig, comment);
	    break;
      case inf_minimize:
	    assert(clause);
	    assert(!partner);
	    clause->ident = ++ClauseIdentCounter;
	    print_minimize(out, clause, old_id, comment);
	    break;	    
       /* inf_rewrite is special and handled below !*/
      default:
	    fprintf(out, "# Clause modification %d not yet implemented.\n",op);
	    break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseQuote()
//
//   Print the clause with a new id as a descendent of itself only. 
//   Useful for getting the comment out.
//
// Global Variables: ClauseIdentCounter
//
// Side Effects    : Output, generates new id for clause
//
/----------------------------------------------------------------------*/

void DocClauseQuote(FILE* out, long level, long target_level,
		    Clause_p clause, char* comment, Clause_p
		    opt_partner)
{
   long old_id;

   assert(clause);

   old_id = clause->ident;
   
   if(level >= target_level)
   {
      switch(OutputFormat)
      {
      case pcl_format:	    
	    clause->ident = ++ClauseIdentCounter;
	    pcl_print_start(out, clause);
	    fprintf(out, "%ld", old_id);
	    if(opt_partner)
	    {
	       assert(comment);
	       fprintf(out, " : '%s(%ld)'\n", comment, opt_partner->ident);
	    }
	    else
	    {
	       pcl_print_end(out, comment, clause);
	    }
	    break; 
      case tstp_format: 
	    clause->ident = ++ClauseIdentCounter;
	    ClauseTSTPPrint(out, clause, PCLFullTerms, false);
	    fprintf(out, ", c_0_%ld", old_id);
	    if(opt_partner)
	    {
	       assert(comment);
	       fprintf(out, ",['%s(c_0_%ld)']).\n", comment, opt_partner->ident);
	    }
	    else if(comment)
	    {
	       fprintf(out, ",['%s']).\n", comment);
	    }
	    else
	    {
	       fprintf(out, ").\n");
	    }
	    break;
   
      default:	 
	    fprintf(out, "# Output format not implemented.\n");
	    break;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseRewrite()
//
//   Document a series of rewrite steps performed on the literal
//   position described in pos, on the original term old_term. 
//
// Global Variables: -
//
// Side Effects    : Output, changes clause identifier.
//
/----------------------------------------------------------------------*/

void DocClauseRewrite(FILE* out, long level, ClausePos_p rewritten,
		      Term_p old_term, char* comment)
{
   long old_id;

   if(level >= 2)
   {
      assert(rewritten);
      assert(rewritten->clause);
      assert(rewritten->literal);
      old_id = rewritten->clause->ident;
      rewritten->clause->ident = ++ClauseIdentCounter;
      print_rewrite(out, rewritten, old_id, old_term, comment);
   }
}


/*-----------------------------------------------------------------------
//
// Function: DocClauseEqUnfold()
//
//   Document rewrite steps caused by definition unfolding. Ugly and
//   incomplete. 
//
// Global Variables: -
//
// Side Effects    :  Output, changes clause identifier.
//
/----------------------------------------------------------------------*/

void DocClauseEqUnfold(FILE* out, long level, Clause_p rewritten,
		       ClausePos_p demod, PStack_p demod_pos)
{
   long old_id;
   
   if(level >= 2)
   {
      assert(rewritten);
      old_id = rewritten->ident;
      rewritten->ident = ++ClauseIdentCounter;
      print_eq_unfold(out, rewritten, old_id, demod, demod_pos);
   }

}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


