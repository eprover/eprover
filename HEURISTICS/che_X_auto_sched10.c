
/* -------------------------------------------------------*/
/* The following code is generated automagically with     */
/* generate_auto.py. Yes, it is fairly ugly ;-)           */
/* -------------------------------------------------------*/

/* Class dir used: ../CLASS_LISTS_DUMMY/ */

/* CLASS_F------------- : protocol_G-E--_208_B00_00_F1_SE_CS_SP_PS_S064A.csv 48   */

#ifdef CHE_PROOFCONTROL_INTERNAL

/* Strategies used:                                       */

"G_E___208_B00_00_F1_SE_CS_SP_PS_S064A = \n"
"(1.ConjectureRelativeSymbolWeight(SimulateSOS,0.5,100,100,100,100,1.5,1.5,1),"
" 4.ConjectureRelativeSymbolWeight(ConstPrio,0.1,100,100,100,100,1.5,1.5,1.5),"
" 1.FIFOWeight(PreferProcessed),"
" 1.ConjectureRelativeSymbolWeight(PreferNonGoals,0.5,100,100,100,100,1.5,1.5,1),"
" 4.Refinedweight(SimulateSOS,3,2,2,1.5,2))\n"
/* Global best, protocol_G-E--_208_B00_00_F1_SE_CS_SP_PS_S064A.csv, already defined */
#endif

#if defined(CHE_HEURISTICS_INTERNAL) || defined(TO_ORDERING_INTERNAL)

   else if(
      ( /* CLASS_F------------- Solved: 48 of 17354 */
       SpecIsFO(spec)))
   {
#ifdef CHE_HEURISTICS_INTERNAL
            res = "G_E___208_B00_00_F1_SE_CS_SP_PS_S064A";
      control->heuristic_parms.forward_context_sr = true;
      control->heuristic_parms.selection_strategy=SelectComplexG;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_strong_destructive=true;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_aggressive=true;
      control->heuristic_parms.pm_type=ParamodSim;
      control->heuristic_parms.presat_interreduction=true;

#endif
#ifdef TO_ORDERING_INTERNAL
      oparms.lit_cmp=LCTFOEqMax;
      oparms.ordertype=LPO4;
      oparms.to_weight_gen=WSelectMaximal;
      oparms.to_prec_gen=PUnaryFirst;

#endif
   }
   else /* Default */
   {
#ifdef CHE_HEURISTICS_INTERNAL
  res = "G_E___208_B00_00_F1_SE_CS_SP_PS_S064A";
      control->heuristic_parms.forward_context_sr = true;
      control->heuristic_parms.selection_strategy=SelectComplexG;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_strong_destructive=true;
      control->heuristic_parms.er_varlit_destructive=true;
      control->heuristic_parms.er_aggressive=true;
      control->heuristic_parms.pm_type=ParamodSim;
      control->heuristic_parms.presat_interreduction=true;

#endif
#ifdef TO_ORDERING_INTERNAL
      oparms.lit_cmp=LCTFOEqMax;
      oparms.ordertype=LPO4;
      oparms.to_weight_gen=WSelectMaximal;
      oparms.to_prec_gen=PUnaryFirst;

#endif
   }
#endif

/* Total solutions on test set: 48 */
/* -------------------------------------------------------*/
/*     End of automatically generated code.               */
/* -------------------------------------------------------*/

