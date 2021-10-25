/*-----------------------------------------------------------------------

File  : cte_ho_bindings.c

Author: Petar Vukmirovic.

Contents

  Implementation of the module which creates higher-order variable
  bindings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> ma 25 okt 2021 10:35:21 CEST
    New

-----------------------------------------------------------------------*/

#include "cte_ho_bindings.h"

#define CONSTRAINT_STATE(c) ((c) & 3)
#define CONSTRAINT_COUNTER(c) ((c) >> 2) // c must be unisigned!!!
// #define CONSTRAINT_COUNTER_ADD(c,x) ( ((CONSTRAINT_COUNTER(c) + x) << 2) | (CONSTRAINT_STATE(c)) )
// #define CONSTRAINT_COUNTER_INC(c) ( CONSTRAINT_COUNTER_ADD(c, 1))
#define BUILD_CONSTR(c, s) (((c)<<2)|s)

#define IMIT_MASK (63U)
#define PROJ_MASK (IMIT_MASK << 6)
#define IDENT_MASK (PROJ_MASK << 6)
#define ELIM_MASK (IDENT_MASK << 6)

#define GET_IMIT(c) ( (c) & IMIT_MASK )
#define GET_PROJ(c) ( ((c) & PROJ_MASK) >> 6 )
#define GET_IDENT(c) ( ((c) & IDENT_MASK) >> 12 )
#define GET_ELIM(c) ( ((c) & ELIM_MASK) >> 18 )


#define INC_IMIT(c) ( (GET_IMIT(c)+1) | (~IMIT_MASK & c) )
#define INC_PROJ(c) ( (GET_PROJ(c)+1 << 6) | (~PROJ_MASK & c) )
#define INC_IDENT(c) ( (GET_IDENT(c)+1 << 12) | (~IDENT_MASK & c) )
#define INC_ELIM(c) ( (GET_ELIM(c)+1 << 18) | (~ELIM_MASK & c) )


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: build_imitation()
//
//   Builds imitation binding if rhs has a constant as the head.
//   Otherwise returns NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_imitation(TB_p bank, Term_p flex, Term_p rhs)
{
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: build_projection()
//
//   Projects onto argument idx if return type of variable at the head
//   of flex returns the same type as the argument. Otherwise returns NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_projection(TB_p bank, Term_p flex, Term_p rhs, int idx)
{
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: build_elim()
//
//   Eliminates argument idx. Always succeeds.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_elim(TB_p bank, Term_p flex, int idx)
{
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: build_ident()
//
//   Builds identification binding. Must be called with both lhs and 
//   rhs top-level free variables. Then it returns.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_ident(TB_p bank, Term_p lhs, Term_p rhs)
{
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: build_trivial_ident()
//
//   Builds trivial identification binding. Must be called with both lhs and 
//   rhs top-level free variables. Then it returns.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p build_trivial_ident(TB_p bank, Term_p lhs, Term_p rhs)
{
   return NULL;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: SubstComputeFixpointMgu()
//
//   Assuming that var is an (applied) variable and rhs an arbitrary term
//   which are normalized and to which substitution is applied generate
//   the next binding in an attempt to solve the problem var =?= rhs. 
//   What the next binding is is determined by the value of 'state'.
//   The last two bits of 'state' have special meaning (is the variable
//   pair already processed) and the remaining bits determine how far
//   in the enumeration of bindings we are. 'applied_bs' counts how
//   many bindings of a certain kind are applied. It is a value that
//   is inspected through bit masks that give value of particular bindings.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ConstraintTag_t ComputeNextBinding(Term_p var, Term_p rhs, 
                                   ConstraintTag_t state, Limits_t applied_bs,
                                   TB_p bank, Subst_p subst,
                                   HeuristicParms_p parms)
{
   assert(TermIsTopLevelFreeVar(var));
   ConstraintTag_t counter = CONSTRAINT_COUNTER(state);
   ConstraintTag_t is_solved = CONSTRAINT_STATE(state);
   ConstraintTag_t res = 0;

   if(is_solved != DECOMPOSED_VAR)
   {
      int num_args = TermIsPhonyApp(var) ? var->arity-1 : 0;
      Term_p hd_var = TermIsPhonyApp(var) ? var->args[0] : var;
      
      /* counter values : 0 -- imitation
                        : 1-num_args -- projection
                        : num_args+1 -- 2*num_args -- elimination
                        : 2*num_args+1 -- (trivial) identification
      */
      while(res == 0 && counter < 2*num_args+2)
      {
         if(counter == 0)
         {
            counter++;
            if(!TermIsAppliedFreeVar(rhs) &&
               GET_IMIT(applied_bs) < parms->imit_limit)
            {
               Term_p target = build_imitation(bank, var, rhs);
               if(target)
               {
                  // imitation building can fail if head is DB var
                  res = BUILD_CONSTR(counter, state);
                  SubstAddBinding(subst, var, target);
               }
            }
         }
         else if(num_args != 0 && counter <= num_args)
         {
            // applied variable so we do not subtract 1
            Term_p arg = var->args[counter]; 
            if(GetRetType(hd_var->type) == GetRetType(arg->type) &&
               (GET_PROJ(applied_bs) < parms->func_proj_limit
               || !TypeIsArrow(arg->type)))
            {
               Term_p target = build_projection(bank, var, rhs, counter-1);
               if(target)
               {
                  // building projection can fail if it is determined
                  // that it should not be generated
                  res = BUILD_CONSTR(counter+1, state);
                  SubstAddBinding(subst, var, target);
               }
            }
            counter++;
         }
         else if(num_args != 0 && counter <= 2*num_args)
         {
            // elimination -- currently computing only linear
            // applied variable so we do not subtract 1
            counter++;
            if(GET_ELIM(applied_bs) < parms->elim_limit)
            {
               Term_p target = build_elim(bank, var, counter-num_args-1);
               res = BUILD_CONSTR(counter, state);
               SubstAddBinding(subst, var, target);
            }
            else 
            {
               // skipping other arguments
               counter = 2*num_args+1;
            }
         }
         else if(counter == 2*num_args+1 && TermIsTopLevelFreeVar(rhs))
         {
            // identification
            counter++;
            Term_p target = 
               (GET_IDENT(applied_bs) < parms->ident_limit ? build_ident : build_trivial_ident)
               (bank, var, rhs);
            res = BUILD_CONSTR(counter, state);
            SubstAddBinding(subst, var, target);
         }
      }
   }
   return res;
}
