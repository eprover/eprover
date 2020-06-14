
/* -------------------------------------------------------*/
/* The following code is generated automagically with     */
/* generate_auto.py. Yes, it is fairly ugly ;-)           */
/* -------------------------------------------------------*/

/* Class dir used: ../CLASS_LISTS_DUMMY/ */

/* CLASS_F------------- : protocol_H----_011_C07_F1_PI_SE_SP_S0V.csv 44   */

#ifdef CHE_PROOFCONTROL_INTERNAL

/* Strategies used:                                       */

"H_____011_C07_F1_PI_SE_SP_S0V = \n"
"(8.Refinedweight(PreferGoals,1,2,2,1,0.8),"
" 8.Refinedweight(PreferNonGoals,2,1,2,3,0.8),"
" 1.Clauseweight(ConstPrio,1,1,0.7),"
" 1.FIFOWeight(ByNegLitDist))\n"
/* Global best, protocol_H----_011_C07_F1_PI_SE_SP_S0V.csv, already defined */
#endif

#if defined(CHE_HEURISTICS_INTERNAL) || defined(TO_ORDERING_INTERNAL)

   else if(
      ( /* CLASS_F------------- Solved: 44 of 17354 */
       SpecIsFO(spec)))
   {
#ifdef CHE_HEURISTICS_INTERNAL
            res = "H_____011_C07_F1_PI_SE_SP_S0V";
      control->heuristic_parms.prefer_initial_clauses=true;
      control->heuristic_parms.selection_strategy=PSelectComplexExceptRRHorn;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_strong_destructive=true;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_aggressive=true;
      control->heuristic_parms.forward_demod=1;
      control->heuristic_parms.pm_type=ParamodSim;

#endif
#ifdef TO_ORDERING_INTERNAL
      oparms.ordertype=KBO6;
      oparms.to_weight_gen=WConstantWeight;
      oparms.to_prec_gen=PByInvFrequency;

#endif
   }
   else /* Default */
   {
#ifdef CHE_HEURISTICS_INTERNAL
  res = "H_____011_C07_F1_PI_SE_SP_S0V";
      control->heuristic_parms.prefer_initial_clauses=true;
      control->heuristic_parms.selection_strategy=PSelectComplexExceptRRHorn;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_strong_destructive=true;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_aggressive=true;
      control->heuristic_parms.forward_demod=1;
      control->heuristic_parms.pm_type=ParamodSim;

#endif
#ifdef TO_ORDERING_INTERNAL
      oparms.ordertype=KBO6;
      oparms.to_weight_gen=WConstantWeight;
      oparms.to_prec_gen=PByInvFrequency;

#endif
   }
#endif

/* Total solutions on test set: 44 */
/* -------------------------------------------------------*/
/*     End of automatically generated code.               */
/* -------------------------------------------------------*/

