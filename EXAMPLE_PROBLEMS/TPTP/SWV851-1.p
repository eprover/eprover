%------------------------------------------------------------------------------
% File     : SWV851-1 : TPTP v6.4.0. Released v4.1.0.
% Domain   : Software Verification
% Problem  : Hoare logic with procedures 270_1
% Version  : Especial.
% English  : Completeness is taken relative to completeness of the underlying
%            logic. Two versions of completeness proof: nested single recursion
%            and simultaneous recursion in call rule.

% Refs     : [Nip10] Nipkow (2010), Email to Geoff Sutcliffe
%          : [BN10]  Boehme & Nipkow (2010), Sledgehammer: Judgement Day
% Source   : [Nip10]
% Names    : Hoare-270_1 [Nip10]

% Status   : Unsatisfiable
% Rating   : 1.00 v6.2.0, 0.90 v6.1.0, 0.93 v6.0.0, 0.90 v5.5.0, 1.00 v4.1.0
% Syntax   : Number of clauses     :  669 (  57 non-Horn; 181 unit; 348 RR)
%            Number of atoms       : 1451 ( 422 equality)
%            Maximal clause size   :    7 (   2 average)
%            Number of predicates  :   44 (   0 propositional; 1-4 arity)
%            Number of functors    :   76 (  19 constant; 0-6 arity)
%            Number of variables   : 2245 ( 285 singleton)
%            Maximal term depth    :   13 (   2 average)
% SPC      : CNF_UNS_RFO_SEQ_NHN

% Comments :
%------------------------------------------------------------------------------
cnf(cls_o__eq__dest__lhs_0,axiom,
    ( hAPP(V_a,hAPP(V_b,V_v)) = hAPP(c_Fun_Ocomp(V_a,V_b,T_c,T_b,T_a),V_v) )).

cnf(cls_o__apply_0,axiom,
    ( hAPP(c_Fun_Ocomp(V_f,V_g,T_b,T_a,T_c),V_x) = hAPP(V_f,hAPP(V_g,V_x)) )).

cnf(cls_Diff__cancel_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_A,V_A,tc_fun(T_a,tc_bool)) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_Diff__empty_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_A,c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = V_A )).

cnf(cls_inf__bot__left_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),c_Orderings_Obot__class_Obot(T_a)),V_x) = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_inf__bot__right_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),c_Orderings_Obot__class_Obot(T_a)) = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_finite__Diff2_1,axiom,
    ( c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(V_A,T_a)
    | ~ c_Finite__Set_Ofinite(V_B,T_a) )).

cnf(cls_finite__Diff2_0,axiom,
    ( c_Finite__Set_Ofinite(V_A,T_a)
    | ~ c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(V_B,T_a) )).

cnf(cls_Compl__Diff__eq_0,axiom,
    ( c_HOL_Ouminus__class_Ouminus(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool))),V_B) )).

cnf(cls_vimage__UNIV_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool))) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_Compl__eq__Diff__UNIV_0,axiom,
    ( c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)) = c_HOL_Ominus__class_Ominus(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_inf__sup__aci_I2_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)),V_z) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)) )).

cnf(cls_inf__sup__aci_I3_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_z)) )).

cnf(cls_inf__left__commute_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_z)) )).

cnf(cls_inf__assoc_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)),V_z) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)) )).

cnf(cls_empty__Diff_0,axiom,
    ( c_HOL_Ominus__class_Ominus(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),V_A,tc_fun(T_a,tc_bool)) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_inf__eq__top__eq2_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_A),V_B) != c_Orderings_Otop__class_Otop(T_a)
    | V_B = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_inf__eq__top__eq1_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_A),V_B) != c_Orderings_Otop__class_Otop(T_a)
    | V_A = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_fold__inf__insert_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_Finite__Set_Ofold(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_b,hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a,T_a) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),c_Finite__Set_Ofold(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_b,V_A,T_a,T_a))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_Diff__mono_0,axiom,
    ( c_lessequals(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),c_HOL_Ominus__class_Ominus(V_C,V_D,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_D,V_B,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_inf__top__right_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),c_Orderings_Otop__class_Otop(T_a)) = V_x )).

cnf(cls_inf__top__left_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),c_Orderings_Otop__class_Otop(T_a)),V_x) = V_x )).

cnf(cls_Compl__eq__Compl__iff_0,axiom,
    ( c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)) != c_HOL_Ouminus__class_Ouminus(V_B,tc_fun(T_a,tc_bool))
    | V_A = V_B )).

cnf(cls_evalc_OSkip_0,axiom,
    ( c_Natural_Oevalc(c_Com_Ocom_OSKIP,V_s,V_s) )).

cnf(cls_evaln_OSkip_0,axiom,
    ( c_Natural_Oevaln(c_Com_Ocom_OSKIP,V_s,V_n,V_s) )).

cnf(cls_fold__empty_0,axiom,
    ( c_Finite__Set_Ofold(V_f,V_z,c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),T_b,T_a) = V_z )).

cnf(cls_inf__sup__ord_I1_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y),V_x,T_a) )).

cnf(cls_inf__sup__ord_I2_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y),V_y,T_a) )).

cnf(cls_inf__greatest_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z),T_a)
    | ~ c_lessequals(V_x,V_z,T_a)
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_le__inf__iff_2,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z),T_a)
    | ~ c_lessequals(V_x,V_z,T_a)
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_le__infI_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),T_a)
    | ~ c_lessequals(V_x,V_b,T_a)
    | ~ c_lessequals(V_x,V_a,T_a) )).

cnf(cls_inf__le2_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y),V_y,T_a) )).

cnf(cls_inf__le1_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y),V_x,T_a) )).

cnf(cls_o__assoc_0,axiom,
    ( c_Fun_Ocomp(V_f,c_Fun_Ocomp(V_g,V_h,T_d,T_c,T_a),T_c,T_b,T_a) = c_Fun_Ocomp(c_Fun_Ocomp(V_f,V_g,T_c,T_b,T_d),V_h,T_d,T_b,T_a) )).

cnf(cls_com_Osimps_I13_J_0,axiom,
    ( c_Com_Ocom_OSemi(V_com1_H,V_com2_H) != c_Com_Ocom_OSKIP )).

cnf(cls_inf__sup__aci_I4_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) )).

cnf(cls_inf__left__idem_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) )).

cnf(cls_finite__set_0,axiom,
    ( c_Finite__Set_Ofinite(c_List_Oset(V_xs,T_a),T_a) )).

cnf(cls_vimage__UN_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_c,tc_fun(T_b,tc_bool))) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,hAPP(c_COMBB(c_Set_Ovimage(V_f,T_a,T_b),tc_fun(T_b,tc_bool),tc_fun(T_a,tc_bool),T_c),V_B),T_c,tc_fun(T_a,tc_bool)) )).

cnf(cls_inf__idem_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_x) = V_x )).

cnf(cls_o__eq__dest_0,axiom,
    ( c_Fun_Ocomp(V_a,V_b,T_c,T_b,T_a) != c_Fun_Ocomp(V_c,V_d,T_d,T_b,T_a)
    | hAPP(V_a,hAPP(V_b,V_v)) = hAPP(V_c,hAPP(V_d,V_v)) )).

cnf(cls_comp__cong_0,axiom,
    ( hAPP(V_f,hAPP(V_g,V_x)) != hAPP(V_f_H,hAPP(V_g_H,V_x_H))
    | hAPP(c_Fun_Ocomp(V_f,V_g,T_b,T_a,T_c),V_x) = hAPP(c_Fun_Ocomp(V_f_H,V_g_H,T_d,T_a,T_e),V_x_H) )).

cnf(cls_vimage__code_0,axiom,
    ( hBOOL(hAPP(V_A,hAPP(V_f,V_x)))
    | ~ hBOOL(hAPP(hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_A),V_x)) )).

cnf(cls_vimage__code_1,axiom,
    ( hBOOL(hAPP(hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_A),V_x))
    | ~ hBOOL(hAPP(V_A,hAPP(V_f,V_x))) )).

cnf(cls_double__diff_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_B,c_HOL_Ominus__class_Ominus(V_C,V_A,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = V_A
    | ~ c_lessequals(V_B,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_le__eqI_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_y,T_a) != c_HOL_Ominus__class_Ominus(V_x_H,V_y_H,T_a)
    | c_lessequals(V_y_H,V_x_H,T_a)
    | ~ c_lessequals(V_y,V_x,T_a) )).

cnf(cls_le__eqI_1,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_y,T_a) != c_HOL_Ominus__class_Ominus(V_x_H,V_y_H,T_a)
    | c_lessequals(V_y,V_x,T_a)
    | ~ c_lessequals(V_y_H,V_x_H,T_a) )).

cnf(cls_vimage__empty_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool))) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_com_Osimps_I12_J_0,axiom,
    ( c_Com_Ocom_OSKIP != c_Com_Ocom_OSemi(V_com1_H,V_com2_H) )).

cnf(cls_the__inv__into__comp_0,axiom,
    ( hAPP(c_Fun_Othe__inv__into(V_A,c_Fun_Ocomp(V_f,V_g,T_a,T_b,T_c),T_c,T_b),V_x) = hAPP(c_Fun_Ocomp(c_Fun_Othe__inv__into(V_A,V_g,T_c,T_a),c_Fun_Othe__inv__into(hAPP(c_Set_Oimage(V_g,T_c,T_a),V_A),V_f,T_a,T_b),T_a,T_c,T_b),V_x)
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),hAPP(c_Set_Oimage(V_f,T_a,T_b),hAPP(c_Set_Oimage(V_g,T_c,T_a),V_A))))
    | ~ c_Fun_Oinj__on(V_g,V_A,T_c,T_a)
    | ~ c_Fun_Oinj__on(V_f,hAPP(c_Set_Oimage(V_g,T_c,T_a),V_A),T_a,T_b) )).

cnf(cls_le__iff__diff__le__0_1,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(V_a,V_b,T_a)
    | ~ c_lessequals(c_HOL_Ominus__class_Ominus(V_a,V_b,T_a),c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_le__iff__diff__le__0_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ominus__class_Ominus(V_a,V_b,T_a),c_HOL_Ozero__class_Ozero(T_a),T_a)
    | ~ c_lessequals(V_a,V_b,T_a) )).

cnf(cls_insert__Diff__if_1,axiom,
    ( c_HOL_Ominus__class_Ominus(hAPP(c_Set_Oinsert(V_x,T_a),V_A),V_B,tc_fun(T_a,tc_bool)) = hAPP(c_Set_Oinsert(V_x,T_a),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B)) )).

cnf(cls_insert__Diff1_0,axiom,
    ( c_HOL_Ominus__class_Ominus(hAPP(c_Set_Oinsert(V_x,T_a),V_A),V_B,tc_fun(T_a,tc_bool)) = c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B)) )).

cnf(cls_distrib__inf__le_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_z)),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)),T_a) )).

cnf(cls_distrib__sup__le_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_z)),T_a) )).

cnf(cls_card__Diff__singleton__if_1,axiom,
    ( c_Finite__Set_Ocard(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),T_a) = c_Finite__Set_Ocard(V_A,T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_group__add__class_Odiff__0_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ominus__class_Ominus(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) = c_HOL_Ouminus__class_Ouminus(V_a,T_a) )).

cnf(cls_inf__0__imp__0_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_card__eq__0__iff_2,axiom,
    ( c_Finite__Set_Ocard(V_A,T_a) = c_HOL_Ozero__class_Ozero(tc_nat)
    | c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_card__empty_0,axiom,
    ( c_Finite__Set_Ocard(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),T_a) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_card__insert_0,axiom,
    ( c_Finite__Set_Ocard(hAPP(c_Set_Oinsert(V_x,T_a),V_A),T_a) = c_Suc(c_Finite__Set_Ocard(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),T_a))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_finite__Diff__insert_0,axiom,
    ( c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),V_B),tc_fun(T_a,tc_bool)),T_a) )).

cnf(cls_finite__Diff__insert_1,axiom,
    ( c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),V_B),tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),T_a) )).

cnf(cls_insert__Diff__single_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool))) = hAPP(c_Set_Oinsert(V_a,T_a),V_A) )).

cnf(cls_Diff__insert_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),V_B),tc_fun(T_a,tc_bool)) = c_HOL_Ominus__class_Ominus(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)) )).

cnf(cls_Diff__insert2_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),V_B),tc_fun(T_a,tc_bool)) = c_HOL_Ominus__class_Ominus(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__Compl__self__eq_1,axiom,
    ( c_lessequals(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),c_HOL_Ouminus__class_Ouminus(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__Compl__self__eq_0,axiom,
    ( V_A = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) )).

cnf(cls_inf__compl__bot_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),c_HOL_Ouminus__class_Ouminus(V_x,T_a)) = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_compl__inf__bot_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),c_HOL_Ouminus__class_Ouminus(V_x,T_a)),V_x) = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_the__inv__into__f__f_0,axiom,
    ( hAPP(c_Fun_Othe__inv__into(V_A,V_f,T_a,T_b),hAPP(V_f,V_x)) = V_x
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_the__inv__into__f__eq_0,axiom,
    ( ~ c_Fun_Oinj__on(V_f,V_A,T_aa,T_a)
    | hAPP(c_Fun_Othe__inv__into(V_A,V_f,T_aa,T_a),hAPP(V_f,V_x)) = V_x
    | ~ hBOOL(hAPP(hAPP(c_in(T_aa),V_x),V_A)) )).

cnf(cls_card__eq__UNIV__imp__eq__UNIV_0,axiom,
    ( c_Finite__Set_Ocard(V_A,T_a) != c_Finite__Set_Ocard(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a)
    | V_A = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_finite__compl_0,axiom,
    ( c_Finite__Set_Ofinite(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_finite__compl_1,axiom,
    ( c_Finite__Set_Ofinite(c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_neg__sup__eq__inf_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),T_a) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)),c_HOL_Ouminus__class_Ouminus(V_b,T_a)) )).

cnf(cls_compl__sup_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ouminus__class_Ouminus(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),T_a) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),c_HOL_Ouminus__class_Ouminus(V_x,T_a)),c_HOL_Ouminus__class_Ouminus(V_y,T_a)) )).

cnf(cls_neg__inf__eq__sup_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),T_a) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)),c_HOL_Ouminus__class_Ouminus(V_b,T_a)) )).

cnf(cls_compl__inf_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ouminus__class_Ouminus(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y),T_a) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),c_HOL_Ouminus__class_Ouminus(V_x,T_a)),c_HOL_Ouminus__class_Ouminus(V_y,T_a)) )).

cnf(cls_inf__eq__neg__sup_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b) = c_HOL_Ouminus__class_Ouminus(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)),c_HOL_Ouminus__class_Ouminus(V_b,T_a)),T_a) )).

cnf(cls_sup__eq__neg__inf_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b) = c_HOL_Ouminus__class_Ouminus(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)),c_HOL_Ouminus__class_Ouminus(V_b,T_a)),T_a) )).

cnf(cls_INT__extend__simps_I4_J_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_A,c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),V_B,T_b,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = V_A )).

cnf(cls_Compl__UNIV__eq_0,axiom,
    ( c_HOL_Ouminus__class_Ouminus(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_Diff__UNIV_0,axiom,
    ( c_HOL_Ominus__class_Ominus(V_A,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_Compl__empty__eq_0,axiom,
    ( c_HOL_Ouminus__class_Ouminus(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_image__diff__subset_0,axiom,
    ( c_lessequals(c_HOL_Ominus__class_Ominus(hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_B),tc_fun(T_a,tc_bool)),hAPP(c_Set_Oimage(V_f,T_b,T_a),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_b,tc_bool))),tc_fun(T_a,tc_bool)) )).

cnf(cls_image__vimage__subset_0,axiom,
    ( c_lessequals(hAPP(c_Set_Oimage(V_f,T_b,T_a),hAPP(c_Set_Ovimage(V_f,T_b,T_a),V_A)),V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_Diff__subset__conv_0,axiom,
    ( c_lessequals(V_A,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_C),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_Diff__subset__conv_1,axiom,
    ( c_lessequals(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_C),tc_fun(T_a,tc_bool)) )).

cnf(cls_Diff__partition_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_HOL_Ominus__class_Ominus(V_B,V_A,tc_fun(T_a,tc_bool))) = V_B
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_UN__extend__simps_I10_J_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,hAPP(c_COMBB(V_B,T_c,tc_fun(T_a,tc_bool),T_b),V_f),T_b,tc_fun(T_a,tc_bool)) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(hAPP(c_Set_Oimage(V_f,T_b,T_c),V_A),V_B,T_c,tc_fun(T_a,tc_bool)) )).

cnf(cls_image__UN_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_b,T_a),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_c,tc_fun(T_b,tc_bool))) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,hAPP(c_COMBB(c_Set_Oimage(V_f,T_b,T_a),tc_fun(T_b,tc_bool),tc_fun(T_a,tc_bool),T_c),V_B),T_c,tc_fun(T_a,tc_bool)) )).

cnf(cls_UN__simps_I10_J_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(hAPP(c_Set_Oimage(V_f,T_c,T_b),V_A),V_B,T_b,tc_fun(T_a,tc_bool)) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,hAPP(c_COMBB(V_B,T_b,tc_fun(T_a,tc_bool),T_c),V_f),T_c,tc_fun(T_a,tc_bool)) )).

cnf(cls_Compl__partition_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool))) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_Compl__partition2_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool))),V_A) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_fun__Compl__def_0,axiom,
    ( ~ class_HOL_Ouminus(T_b)
    | hAPP(c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(t_a,T_b)),v_x) = c_HOL_Ouminus__class_Ouminus(hAPP(V_A,v_x),T_b) )).

cnf(cls_comp__inj__on_0,axiom,
    ( c_Fun_Oinj__on(c_Fun_Ocomp(V_g,V_f,T_b,T_c,T_a),V_A,T_a,T_c)
    | ~ c_Fun_Oinj__on(V_g,hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),T_b,T_c)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_inj__on__imageI_0,axiom,
    ( c_Fun_Oinj__on(V_g,hAPP(c_Set_Oimage(V_f,T_a,T_c),V_A),T_c,T_b)
    | ~ c_Fun_Oinj__on(c_Fun_Ocomp(V_g,V_f,T_c,T_b,T_a),V_A,T_a,T_b) )).

cnf(cls_inj__on__the__inv__into_0,axiom,
    ( c_Fun_Oinj__on(c_Fun_Othe__inv__into(V_A,V_f,T_a,T_b),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),T_b,T_a)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_the__inv__into__onto_0,axiom,
    ( hAPP(c_Set_Oimage(c_Fun_Othe__inv__into(V_A,V_f,T_a,T_b),T_b,T_a),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)) = V_A
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_card__image_0,axiom,
    ( c_Finite__Set_Ocard(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),T_b) = c_Finite__Set_Ocard(V_A,T_a)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_the__inv__f__f_0,axiom,
    ( hAPP(c_Fun_Othe__inv__into(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_f,T_a,T_b),hAPP(V_f,V_x)) = V_x
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_K__record__comp_0,axiom,
    ( hAPP(c_Fun_Ocomp(c_COMBK(V_c,T_b,T_c),V_f,T_c,T_b,t_a),v_x) = V_c )).

cnf(cls_Ints__diff_0,axiom,
    ( ~ class_Ring__and__Field_Oring__1(T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),c_HOL_Ominus__class_Ominus(V_a,V_b,T_a)),c_Int_Oring__1__class_OInts(T_a)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_b),c_Int_Oring__1__class_OInts(T_a)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),c_Int_Oring__1__class_OInts(T_a))) )).

cnf(cls_com_Osize_I1_J_0,axiom,
    ( c_Com_Ocom_Ocom__size(c_Com_Ocom_OSKIP) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_com_Osize_I9_J_0,axiom,
    ( c_Nat_Osize__class_Osize(c_Com_Ocom_OSKIP,tc_Com_Ocom) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_card__Suc__Diff1_0,axiom,
    ( c_Suc(c_Finite__Set_Ocard(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),T_a)) = c_Finite__Set_Ocard(V_A,T_a)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_card__insert__if_1,axiom,
    ( c_Finite__Set_Ocard(hAPP(c_Set_Oinsert(V_x,T_a),V_A),T_a) = c_Suc(c_Finite__Set_Ocard(V_A,T_a))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_card__Suc__eq_5,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))
    | c_Finite__Set_Ocard(hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),T_a) = c_Suc(c_Finite__Set_Ocard(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),T_a)) )).

cnf(cls_com_Osimps_I3_J_0,axiom,
    ( c_Com_Ocom_OSemi(V_com1,V_com2) != c_Com_Ocom_OSemi(V_com1_H,V_com2_H)
    | V_com1 = V_com1_H )).

cnf(cls_com_Osimps_I3_J_1,axiom,
    ( c_Com_Ocom_OSemi(V_com1,V_com2) != c_Com_Ocom_OSemi(V_com1_H,V_com2_H)
    | V_com2 = V_com2_H )).

cnf(cls_Compl__subset__Compl__iff_1,axiom,
    ( c_lessequals(c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),c_HOL_Ouminus__class_Ouminus(V_B,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_B,V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_Compl__subset__Compl__iff_0,axiom,
    ( c_lessequals(V_B,V_A,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),c_HOL_Ouminus__class_Ouminus(V_B,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) )).

cnf(cls_Compl__anti__mono_0,axiom,
    ( c_lessequals(c_HOL_Ouminus__class_Ouminus(V_B,tc_fun(T_a,tc_bool)),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_vimage__Diff_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_b,tc_bool))) = c_HOL_Ominus__class_Ominus(hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_A),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_B),tc_fun(T_a,tc_bool)) )).

cnf(cls_inj__on__diff_0,axiom,
    ( c_Fun_Oinj__on(V_f,c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),T_a,T_b)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_finite__Diff_0,axiom,
    ( c_Finite__Set_Ofinite(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),T_a)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_Diff__idemp_0,axiom,
    ( c_HOL_Ominus__class_Ominus(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool)) = c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_fun__upd__comp_0,axiom,
    ( c_Fun_Ocomp(V_f,c_Fun_Ofun__upd(V_g,V_x,V_y,T_a,T_c),T_c,T_b,T_a) = c_Fun_Ofun__upd(c_Fun_Ocomp(V_f,V_g,T_c,T_b,T_a),V_x,hAPP(V_f,V_y),T_a,T_b) )).

cnf(cls_eq__eqI_1,axiom,
    ( ~ class_OrderedGroup_Oab__group__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_xa,V_y,T_a) != c_HOL_Ominus__class_Ominus(V_x,V_x,T_a)
    | V_xa = V_y )).

cnf(cls_eq__eqI_0,axiom,
    ( ~ class_OrderedGroup_Oab__group__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_x,T_a) != c_HOL_Ominus__class_Ominus(V_x_H,V_y_H,T_a)
    | V_x_H = V_y_H )).

cnf(cls_evaln_OSemi_0,axiom,
    ( c_Natural_Oevaln(c_Com_Ocom_OSemi(V_c0,V_c1),V_s0,V_n,V_s2)
    | ~ c_Natural_Oevaln(V_c1,V_s1,V_n,V_s2)
    | ~ c_Natural_Oevaln(V_c0,V_s0,V_n,V_s1) )).

cnf(cls_diff__eq_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_y,T_a) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),c_HOL_Ouminus__class_Ouminus(V_y,T_a)) )).

cnf(cls_evalc_OSemi_0,axiom,
    ( c_Natural_Oevalc(c_Com_Ocom_OSemi(V_c0,V_c1),V_s0,V_s2)
    | ~ c_Natural_Oevalc(V_c1,V_s1,V_s2)
    | ~ c_Natural_Oevalc(V_c0,V_s0,V_s1) )).

cnf(cls_le__iff__inf_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) = V_x
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_le__iff__inf_1,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) != V_x
    | c_lessequals(V_x,V_y,T_a) )).

cnf(cls_inf__absorb2_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) = V_y
    | ~ c_lessequals(V_y,V_x,T_a) )).

cnf(cls_inf__commute_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_x) )).

cnf(cls_inf__sup__aci_I1_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_x) )).

cnf(cls_Diff__subset_0,axiom,
    ( c_lessequals(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)),V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_vimage__Compl_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_b,tc_bool))) = c_HOL_Ouminus__class_Ouminus(hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_A),tc_fun(T_a,tc_bool)) )).

cnf(cls_vimage__mono_0,axiom,
    ( c_lessequals(hAPP(c_Set_Ovimage(V_f,T_b,T_a),V_A),hAPP(c_Set_Ovimage(V_f,T_b,T_a),V_B),tc_fun(T_b,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_fun__diff__def_0,axiom,
    ( ~ class_HOL_Ominus(T_b)
    | hAPP(c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(t_a,T_b)),v_x) = c_HOL_Ominus__class_Ominus(hAPP(V_A,v_x),hAPP(V_B,v_x),T_b) )).

cnf(cls_le__infE_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,V_a,T_a)
    | ~ c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),T_a) )).

cnf(cls_le__infE_1,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,V_b,T_a)
    | ~ c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),T_a) )).

cnf(cls_le__infI1_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),V_x,T_a)
    | ~ c_lessequals(V_a,V_x,T_a) )).

cnf(cls_le__infI2_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),V_x,T_a)
    | ~ c_lessequals(V_b,V_x,T_a) )).

cnf(cls_le__inf__iff_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,V_y,T_a)
    | ~ c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z),T_a) )).

cnf(cls_le__inf__iff_1,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(V_x,V_z,T_a)
    | ~ c_lessequals(V_x,hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z),T_a) )).

cnf(cls_fold__inf__le__inf_0,axiom,
    ( ~ class_Lattices_Olower__semilattice(T_a)
    | c_lessequals(c_Finite__Set_Ofold(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_b,V_A,T_a,T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_a),V_b),T_a)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_vimage__compose_0,axiom,
    ( hAPP(c_Set_Ovimage(c_Fun_Ocomp(V_g,V_f,T_c,T_b,T_a),T_a,T_b),V_x) = hAPP(c_Set_Ovimage(V_f,T_a,T_c),hAPP(c_Set_Ovimage(V_g,T_c,T_b),V_x)) )).

cnf(cls_evaln__elim__cases_I1_J_0,axiom,
    ( V_t = V_s
    | ~ c_Natural_Oevaln(c_Com_Ocom_OSKIP,V_s,V_n,V_t) )).

cnf(cls_double__complement_0,axiom,
    ( c_HOL_Ouminus__class_Ouminus(c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) = V_A )).

cnf(cls_minus__diff__eq_0,axiom,
    ( ~ class_OrderedGroup_Oab__group__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_HOL_Ominus__class_Ominus(V_a,V_b,T_a),T_a) = c_HOL_Ominus__class_Ominus(V_b,V_a,T_a) )).

cnf(cls_evalc__elim__cases_I1_J_0,axiom,
    ( V_t = V_s
    | ~ c_Natural_Oevalc(c_Com_Ocom_OSKIP,V_s,V_t) )).

cnf(cls_DiffE_1,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)))) )).

cnf(cls_DiffE_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool)))) )).

cnf(cls_ComplI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool))))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A)) )).

cnf(cls_ComplD_0,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool)))) )).

cnf(cls_Diff__iff_2,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool))))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A)) )).

cnf(cls_DiffI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool))))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A)) )).

cnf(cls_vimage__eq_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_B)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),V_B)) )).

cnf(cls_vimageI2_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),V_a),hAPP(c_Set_Ovimage(V_f,T_b,T_a),V_A)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_a)),V_A)) )).

cnf(cls_vimageI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),V_a),hAPP(c_Set_Ovimage(V_f,T_b,T_a),V_B)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_a)),V_B)) )).

cnf(cls_vimageE_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_B))) )).

cnf(cls_vimageD_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_A))) )).

cnf(cls_class__ringb_Osubr0__iff_1,axiom,
    ( ~ class_Ring__and__Field_Oidom(T_a)
    | ~ class_Int_Onumber__ring(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_x,T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_diff__self_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_a,V_a,T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_diff__0__right_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a) = V_a )).

cnf(cls_right__minus__eq_1,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_x,T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_eq__iff__diff__eq__0_0,axiom,
    ( ~ class_OrderedGroup_Oab__group__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_x,T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_class__ringb_Osubr0__iff_0,axiom,
    ( ~ class_Ring__and__Field_Oidom(T_a)
    | ~ class_Int_Onumber__ring(T_a)
    | c_HOL_Ominus__class_Ominus(V_x,V_y,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | V_x = V_y )).

cnf(cls_right__minus__eq_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_a,V_b,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = V_b )).

cnf(cls_eq__iff__diff__eq__0_1,axiom,
    ( ~ class_OrderedGroup_Oab__group__add(T_a)
    | c_HOL_Ominus__class_Ominus(V_a,V_b,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = V_b )).

cnf(cls_sup__inf__distrib1_0,axiom,
    ( ~ class_Lattices_Odistrib__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_z)) )).

cnf(cls_sup__inf__distrib2_0,axiom,
    ( ~ class_Lattices_Odistrib__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_z)),V_x) = hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_x)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_z),V_x)) )).

cnf(cls_inf__sup__absorb_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)) = V_x )).

cnf(cls_sup__inf__absorb_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)) = V_x )).

cnf(cls_inf__sup__distrib1_0,axiom,
    ( ~ class_Lattices_Odistrib__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y)),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_z)) )).

cnf(cls_inf__sup__distrib2_0,axiom,
    ( ~ class_Lattices_Odistrib__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)),V_x) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_y),V_x)),hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_z),V_x)) )).

cnf(cls_image__compose_0,axiom,
    ( hAPP(c_Set_Oimage(c_Fun_Ocomp(V_f,V_g,T_c,T_a,T_b),T_b,T_a),V_r) = hAPP(c_Set_Oimage(V_f,T_c,T_a),hAPP(c_Set_Oimage(V_g,T_b,T_c),V_r)) )).

cnf(cls_Un__Diff__cancel2_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_HOL_Ominus__class_Ominus(V_B,V_A,tc_fun(T_a,tc_bool))),V_A) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_A) )).

cnf(cls_Un__Diff__cancel_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_HOL_Ominus__class_Ominus(V_B,V_A,tc_fun(T_a,tc_bool))) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) )).

cnf(cls_Un__Diff_0,axiom,
    ( c_HOL_Ominus__class_Ominus(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_C,tc_fun(T_a,tc_bool)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_HOL_Ominus__class_Ominus(V_A,V_C,tc_fun(T_a,tc_bool))),c_HOL_Ominus__class_Ominus(V_B,V_C,tc_fun(T_a,tc_bool))) )).

cnf(cls_vimage__Un_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_b,tc_bool)),V_A),V_B)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_A)),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_B)) )).

cnf(cls_o__def_0,axiom,
    ( hAPP(c_Fun_Ocomp(V_f,V_g,T_c,T_b,t_a),v_x) = hAPP(V_f,hAPP(V_g,v_x)) )).

cnf(cls_com_Osimps_I19_J_0,axiom,
    ( hAPP(c_Com_Ocom_OBODY,V_pname_H) != c_Com_Ocom_OSKIP )).

cnf(cls_com_Osimps_I49_J_0,axiom,
    ( hAPP(c_Com_Ocom_OBODY,V_pname_H) != c_Com_Ocom_OSemi(V_com1,V_com2) )).

cnf(cls_com_Osimps_I48_J_0,axiom,
    ( c_Com_Ocom_OSemi(V_com1,V_com2) != hAPP(c_Com_Ocom_OBODY,V_pname_H) )).

cnf(cls_com_Osimps_I18_J_0,axiom,
    ( c_Com_Ocom_OSKIP != hAPP(c_Com_Ocom_OBODY,V_pname_H) )).

cnf(cls_card__insert__if_0,axiom,
    ( c_Finite__Set_Ocard(hAPP(c_Set_Oinsert(V_x,T_a),V_A),T_a) = c_Finite__Set_Ocard(V_A,T_a)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_insert__Diff_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool))) = V_A
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A)) )).

cnf(cls_vimage__singleton__eq_0,axiom,
    ( hAPP(V_f,V_a) = V_b
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Ovimage(V_f,T_a,T_b),hAPP(c_Set_Oinsert(V_b,T_b),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)))))) )).

cnf(cls_vimage__singleton__eq_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_aa),V_a),hAPP(c_Set_Ovimage(V_f,T_aa,T_a),hAPP(c_Set_Oinsert(hAPP(V_f,V_a),T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))))) )).

cnf(cls_Diff__insert__absorb_0,axiom,
    ( c_HOL_Ominus__class_Ominus(hAPP(c_Set_Oinsert(V_x,T_a),V_A),hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)) = V_A
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_card__eq__0__iff_0,axiom,
    ( c_Finite__Set_Ocard(V_A,T_a) != c_HOL_Ozero__class_Ozero(tc_nat)
    | ~ c_Finite__Set_Ofinite(V_A,T_a)
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_UN__insert__distrib_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,hAPP(c_COMBB(c_Set_Oinsert(V_a,T_b),tc_fun(T_b,tc_bool),tc_fun(T_b,tc_bool),T_a),V_B),T_a,tc_fun(T_b,tc_bool)) = hAPP(c_Set_Oinsert(V_a,T_b),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_a,tc_fun(T_b,tc_bool)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_u),V_A)) )).

cnf(cls_fold__sup__insert_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_Finite__Set_Ofold(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_b,hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a,T_a) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),c_Finite__Set_Ofold(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_b,V_A,T_a,T_a))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_vimage__const_1,axiom,
    ( hAPP(c_Set_Ovimage(c_COMBK(V_c,T_b,T_a),T_a,T_b),V_A) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | hBOOL(hAPP(hAPP(c_in(T_b),V_c),V_A)) )).

cnf(cls_subset__insert__iff_4,axiom,
    ( c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_vimage__insert_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),hAPP(c_Set_Oinsert(V_a,T_b),V_B)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(c_Set_Ovimage(V_f,T_a,T_b),hAPP(c_Set_Oinsert(V_a,T_b),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool))))),hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_B)) )).

cnf(cls_f__the__inv__into__f_0,axiom,
    ( hAPP(V_f,hAPP(c_Fun_Othe__inv__into(V_A,V_f,T_a,T_b),V_y)) = V_y
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_y),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_UN__simps_I1_J_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),hAPP(c_COMBB(c_Set_Oinsert(V_a,T_a),tc_fun(T_a,tc_bool),tc_fun(T_a,tc_bool),T_b),V_B),T_b,tc_fun(T_a,tc_bool)) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_UN__simps_I1_J_1,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,hAPP(c_COMBB(c_Set_Oinsert(V_a,T_a),tc_fun(T_a,tc_bool),tc_fun(T_a,tc_bool),T_b),V_B),T_b,tc_fun(T_a,tc_bool)) = hAPP(c_Set_Oinsert(V_a,T_a),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,V_B,T_b,tc_fun(T_a,tc_bool)))
    | V_C = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_UN__extend__simps_I1_J_1,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,V_B,T_b,tc_fun(T_a,tc_bool))) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,hAPP(c_COMBB(c_Set_Oinsert(V_a,T_a),tc_fun(T_a,tc_bool),tc_fun(T_a,tc_bool),T_b),V_B),T_b,tc_fun(T_a,tc_bool))
    | V_C = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_vimage__const_0,axiom,
    ( hAPP(c_Set_Ovimage(c_COMBK(V_c,T_b,T_a),T_a,T_b),V_A) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_c),V_A)) )).

cnf(cls_UN__extend__simps_I3_J_1,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,V_B,T_b,tc_fun(T_a,tc_bool))) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,hAPP(c_COMBB(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),tc_fun(T_a,tc_bool),tc_fun(T_a,tc_bool),T_b),V_B),T_b,tc_fun(T_a,tc_bool))
    | V_C = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_UN__simps_I3_J_1,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,hAPP(c_COMBB(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),tc_fun(T_a,tc_bool),tc_fun(T_a,tc_bool),T_b),V_B),T_b,tc_fun(T_a,tc_bool)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_C,V_B,T_b,tc_fun(T_a,tc_bool)))
    | V_C = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_UN__simps_I3_J_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),hAPP(c_COMBB(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),tc_fun(T_a,tc_bool),tc_fun(T_a,tc_bool),T_b),V_B),T_b,tc_fun(T_a,tc_bool)) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_inj__on__iff__eq__card_0,axiom,
    ( c_Finite__Set_Ocard(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),T_b) = c_Finite__Set_Ocard(V_A,T_a)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_eq__card__imp__inj__on_0,axiom,
    ( c_Finite__Set_Ocard(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),T_b) != c_Finite__Set_Ocard(V_A,T_a)
    | ~ c_Finite__Set_Ofinite(V_A,T_a)
    | c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_finite__vimageI_0,axiom,
    ( c_Finite__Set_Ofinite(hAPP(c_Set_Ovimage(V_h,T_b,T_a),V_F),T_b)
    | ~ c_Fun_Oinj__on(V_h,c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool)),T_b,T_a)
    | ~ c_Finite__Set_Ofinite(V_F,T_a) )).

cnf(cls_inj__on__image__set__diff_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_a,T_b),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool))) = c_HOL_Ominus__class_Ominus(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B),tc_fun(T_b,tc_bool))
    | ~ c_lessequals(V_B,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,V_C,T_a,T_b) )).

cnf(cls_inj__vimage__image__eq_0,axiom,
    ( hAPP(c_Set_Ovimage(V_f,T_a,T_b),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)) = V_A
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_image__set__diff_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_a,T_b),c_HOL_Ominus__class_Ominus(V_A,V_B,tc_fun(T_a,tc_bool))) = c_HOL_Ominus__class_Ominus(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B),tc_fun(T_b,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_o__inv__o__cancel_0,axiom,
    ( c_Fun_Ocomp(c_Fun_Ocomp(V_g,c_Hilbert__Choice_Oinv__into(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_f,T_a,T_b),T_a,T_c,T_b),V_f,T_b,T_c,T_a) = V_g
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inj__transfer_1,axiom,
    ( hBOOL(hAPP(V_P,V_x))
    | ~ hBOOL(hAPP(V_P,hAPP(c_Hilbert__Choice_Oinv__into(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_f,T_a,T_b),c_ATP__Linkup_Osko__Hilbert__Choice__Xinj__transfer__1__1(V_P,V_f,T_a,T_b))))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_comp__def__raw_0,axiom,
    ( hAPP(c_Fun_Ocomp(v_f,v_g,t_a,t_b,t_c),v_x) = hAPP(v_f,hAPP(v_g,v_x)) )).

cnf(cls_sup__le__fold__sup_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),c_Finite__Set_Ofold(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_b,V_A,T_a,T_a),T_a)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_card__Suc__eq_4,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_xa))
    | c_Finite__Set_Ocard(hAPP(c_Set_Oinsert(V_x,T_a),V_xa),T_a) = c_Suc(c_Finite__Set_Ocard(V_xa,T_a))
    | c_Finite__Set_Ocard(V_xa,T_a) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_subset__insert__iff_0,axiom,
    ( c_lessequals(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__insert__iff_3,axiom,
    ( c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_lessequals(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_diff__single__insert_0,axiom,
    ( c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_lessequals(c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_sup__SUPR__fold__sup_0,axiom,
    ( ~ class_Complete__Lattice_Ocomplete__lattice(T_b)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_b),V_B),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_f,T_a,T_b)) = c_Finite__Set_Ofold(hAPP(c_COMBB(c_Lattices_Oupper__semilattice__class_Osup(T_b),T_b,tc_fun(T_b,T_b),T_a),V_f),V_B,V_A,T_a,T_b)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_the__inv__into__into_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),hAPP(c_Fun_Othe__inv__into(V_A,V_f,T_a,T_b),V_x)),V_B))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_compl__unique_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) != c_Orderings_Otop__class_Otop(T_a)
    | hAPP(hAPP(c_Lattices_Olower__semilattice__class_Oinf(T_a),V_x),V_y) != c_Orderings_Obot__class_Obot(T_a)
    | c_HOL_Ouminus__class_Ouminus(V_x,T_a) = V_y )).

cnf(cls_range__ex1__eq_2,axiom,
    ( ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)),T_aa,T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_aa,T_a),c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)))))
    | hAPP(V_f,V_x) = hAPP(V_f,c_ATP__Linkup_Osko__Fun__Xrange__ex1__eq__1__2(hAPP(V_f,V_x),V_f,V_x,T_a,T_aa)) )).

cnf(cls_inj__transfer_0,axiom,
    ( hBOOL(hAPP(V_P,V_x))
    | hBOOL(hAPP(hAPP(c_in(T_b),c_ATP__Linkup_Osko__Hilbert__Choice__Xinj__transfer__1__1(V_P,V_f,T_a,T_b)),hAPP(c_Set_Oimage(V_f,T_a,T_b),c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)))))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_range__ex1__eq_3,axiom,
    ( c_ATP__Linkup_Osko__Fun__Xrange__ex1__eq__1__2(hAPP(V_f,V_x),V_f,V_x,T_a,T_aa) != V_x
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)),T_aa,T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_aa,T_a),c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool))))) )).

cnf(cls_range__ex1__eq_0,axiom,
    ( V_b = hAPP(V_f,c_ATP__Linkup_Osko__Fun__Xrange__ex1__eq__1__1(V_b,V_f,T_b,T_a))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_b),hAPP(c_Set_Oimage(V_f,T_a,T_b),c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)))))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_range__ex1__eq_1,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_y)),hAPP(c_Set_Oimage(V_f,T_aa,T_a),c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)))))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)),T_aa,T_a)
    | V_y = c_ATP__Linkup_Osko__Fun__Xrange__ex1__eq__1__1(hAPP(V_f,V_y),V_f,T_a,T_aa) )).

cnf(cls_card__bij__eq_0,axiom,
    ( c_Finite__Set_Ocard(V_A,T_a) = c_Finite__Set_Ocard(V_B,T_b)
    | ~ c_Finite__Set_Ofinite(V_B,T_b)
    | ~ c_Finite__Set_Ofinite(V_A,T_a)
    | ~ c_lessequals(hAPP(c_Set_Oimage(V_g,T_b,T_a),V_B),V_A,tc_fun(T_a,tc_bool))
    | ~ c_Fun_Oinj__on(V_g,V_B,T_b,T_a)
    | ~ c_lessequals(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),V_B,tc_fun(T_b,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_hoare__valids__def_2,axiom,
    ( c_Hoare__Mirabelle_Ohoare__valids(V_G,V_ts,t_a)
    | c_Hoare__Mirabelle_Otriple__valid(v_sko__Hoare__Mirabelle__Xhoare__valids__def__2(V_G,V_ts),V_x,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_x),V_G)) )).

cnf(cls_vimage__subsetI_0,axiom,
    ( c_lessequals(hAPP(c_Set_Ovimage(V_f,T_a,T_b),V_B),V_A,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_B,hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),tc_fun(T_b,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inj__image__Compl__subset_0,axiom,
    ( c_lessequals(hAPP(c_Set_Oimage(V_f,T_a,T_b),c_HOL_Ouminus__class_Ouminus(V_A,tc_fun(T_a,tc_bool))),c_HOL_Ouminus__class_Ouminus(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),tc_fun(T_b,tc_bool)),tc_fun(T_b,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inv__into__comp_0,axiom,
    ( hAPP(c_Hilbert__Choice_Oinv__into(V_A,c_Fun_Ocomp(V_f,V_g,T_a,T_b,T_c),T_c,T_b),V_x) = hAPP(c_Fun_Ocomp(c_Hilbert__Choice_Oinv__into(V_A,V_g,T_c,T_a),c_Hilbert__Choice_Oinv__into(hAPP(c_Set_Oimage(V_g,T_c,T_a),V_A),V_f,T_a,T_b),T_a,T_c,T_b),V_x)
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),hAPP(c_Set_Oimage(V_f,T_a,T_b),hAPP(c_Set_Oimage(V_g,T_c,T_a),V_A))))
    | ~ c_Fun_Oinj__on(V_g,V_A,T_c,T_a)
    | ~ c_Fun_Oinj__on(V_f,hAPP(c_Set_Oimage(V_g,T_c,T_a),V_A),T_a,T_b) )).

cnf(cls_Comp_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),c_Com_Ocom_OSemi(V_c,V_d)),V_R),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_Q),V_d),V_R),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),V_c),V_Q),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a) )).

cnf(cls_hoare__derivs_OSkip_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),c_Com_Ocom_OSKIP),V_P),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a) )).

cnf(cls_SUPR__fold__sup_0,axiom,
    ( ~ class_Complete__Lattice_Ocomplete__lattice(T_b)
    | c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_f,T_a,T_b) = c_Finite__Set_Ofold(hAPP(c_COMBB(c_Lattices_Oupper__semilattice__class_Osup(T_b),T_b,tc_fun(T_b,T_b),T_a),V_f),c_Orderings_Obot__class_Obot(T_b),V_A,T_a,T_b)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_inj__on__insert_1,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),hAPP(c_Set_Oimage(V_f,T_a,T_b),c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)))))
    | ~ c_Fun_Oinj__on(V_f,hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a,T_b) )).

cnf(cls_inj__on__insert_2,axiom,
    ( c_Fun_Oinj__on(V_f,hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a,T_b)
    | hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),hAPP(c_Set_Oimage(V_f,T_a,T_b),c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)))))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_fun__upd__image_0,axiom,
    ( hAPP(c_Set_Oimage(c_Fun_Ofun__upd(V_f,V_x,V_y,T_b,T_a),T_b,T_a),V_A) = hAPP(c_Set_Oinsert(V_y,T_a),hAPP(c_Set_Oimage(V_f,T_b,T_a),c_HOL_Ominus__class_Ominus(V_A,hAPP(c_Set_Oinsert(V_x,T_b),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool))),tc_fun(T_b,tc_bool))))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A)) )).

cnf(cls_SUP__UN__eq_0,axiom,
    ( hAPP(c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool)),hAPP(c_COMBB(c_COMBC(c_in(t_a),t_a,tc_fun(t_a,tc_bool),tc_bool),tc_fun(t_a,tc_bool),tc_fun(t_a,tc_bool),T_b),V_r),T_b,tc_fun(t_a,tc_bool)),v_x) = hAPP(hAPP(c_in(t_a),v_x),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool)),V_r,T_b,tc_fun(t_a,tc_bool))) )).

cnf(cls_SUPR__set__fold_0,axiom,
    ( ~ class_Complete__Lattice_Ocomplete__lattice(T_a)
    | c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_List_Oset(V_xs,T_b),V_f,T_b,T_a) = c_List_Ofoldl(c_COMBC(hAPP(c_COMBB(c_Lattices_Oupper__semilattice__class_Osup(T_a),T_a,tc_fun(T_a,T_a),T_b),V_f),T_b,T_a,T_a),c_Orderings_Obot__class_Obot(T_a),V_xs,T_a,T_b) )).

cnf(cls_ex__in__conv_0,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) )).

cnf(cls_set__mp_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_subsetD_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_set__rev__mp_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_subset__iff_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_t),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_t),V_A))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_insertCI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Oinsert(V_b,T_a),V_B)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_B)) )).

cnf(cls_insert__iff_2,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Oinsert(V_b,T_a),V_A)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A)) )).

cnf(cls_minus__le__iff_1,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),V_b,T_a)
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_b,T_a),V_a,T_a) )).

cnf(cls_minus__le__iff_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ouminus__class_Ouminus(V_b,T_a),V_a,T_a)
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),V_b,T_a) )).

cnf(cls_le__minus__iff_1,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(V_a,c_HOL_Ouminus__class_Ouminus(V_b,T_a),T_a)
    | ~ c_lessequals(V_b,c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) )).

cnf(cls_le__minus__iff_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(V_b,c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a)
    | ~ c_lessequals(V_a,c_HOL_Ouminus__class_Ouminus(V_b,T_a),T_a) )).

cnf(cls_le__funD_0,axiom,
    ( ~ class_HOL_Oord(T_b)
    | c_lessequals(hAPP(V_f,V_x),hAPP(V_g,V_x),T_b)
    | ~ c_lessequals(V_f,V_g,tc_fun(T_a,T_b)) )).

cnf(cls_eval__eq_1,axiom,
    ( c_Natural_Oevalc(V_c,V_s,V_t)
    | ~ c_Natural_Oevaln(V_c,V_s,V_x,V_t) )).

cnf(cls_evaln__evalc_0,axiom,
    ( c_Natural_Oevalc(V_c,V_s,V_t)
    | ~ c_Natural_Oevaln(V_c,V_s,V_n,V_t) )).

cnf(cls_minus__equation__iff_1,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_HOL_Ouminus__class_Ouminus(V_b,T_a),T_a) = V_b )).

cnf(cls_equation__minus__iff_1,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | V_a = c_HOL_Ouminus__class_Ouminus(c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) )).

cnf(cls_equation__minus__iff_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | V_b = c_HOL_Ouminus__class_Ouminus(c_HOL_Ouminus__class_Ouminus(V_b,T_a),T_a) )).

cnf(cls_double__compl_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_HOL_Ouminus__class_Ouminus(V_x,T_a),T_a) = V_x )).

cnf(cls_minus__minus_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) = V_a )).

cnf(cls_zero__le__imp__of__nat_0,axiom,
    ( ~ class_Ring__and__Field_Oordered__semidom(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),c_Nat_Osemiring__1__class_Oof__nat(V_m,T_a),T_a) )).

cnf(cls_of__nat__0__le__iff_0,axiom,
    ( ~ class_Ring__and__Field_Oordered__semidom(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),c_Nat_Osemiring__1__class_Oof__nat(V_n,T_a),T_a) )).

cnf(cls_finite__surj__inj_0,axiom,
    ( c_Fun_Oinj__on(V_f,V_A,T_a,T_a)
    | ~ c_lessequals(V_A,hAPP(c_Set_Oimage(V_f,T_a,T_a),V_A),tc_fun(T_a,tc_bool))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_fun__upd__idem__iff_0,axiom,
    ( c_Fun_Ofun__upd(V_f,V_x,V_y,T_a,T_b) != V_f
    | hAPP(V_f,V_x) = V_y )).

cnf(cls_power__inject__base_0,axiom,
    ( ~ class_Ring__and__Field_Oordered__semidom(T_a)
    | c_Power_Opower__class_Opower(V_a,c_Suc(V_n),T_a) != c_Power_Opower__class_Opower(V_b,c_Suc(V_n),T_a)
    | V_a = V_b
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_b,T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_evaln__elim__cases_I6_J_0,axiom,
    ( V_n = c_Suc(c_Natural_Osko__Natural__Xevaln__elim__cases__6__1(V_P,V_n,V_s,V_s1))
    | ~ c_Natural_Oevaln(hAPP(c_Com_Ocom_OBODY,V_P),V_s,V_n,V_s1) )).

cnf(cls_finite_0,axiom,
    ( ~ class_Finite__Set_Ofinite_Ofinite(T_a)
    | c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_zero__le__pprt_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),c_OrderedGroup_Olordered__ab__group__add__class_Opprt(V_a,T_a),T_a) )).

cnf(cls_image__inv__f__f_0,axiom,
    ( hAPP(c_Set_Oimage(c_Hilbert__Choice_Oinv__into(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_f,T_a,T_b),T_b,T_a),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)) = V_A
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inv__image__comp_0,axiom,
    ( hAPP(c_Set_Oimage(c_Hilbert__Choice_Oinv__into(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_f,T_a,T_b),T_b,T_a),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_X)) = V_X
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_insert__is__Un_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),V_A) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))),V_A) )).

cnf(cls_subset__insert_0,axiom,
    ( c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_subset__insert_1,axiom,
    ( c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_subset__insert__iff_1,axiom,
    ( c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__insert__iff_2,axiom,
    ( c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),V_B),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_insert__code_2,axiom,
    ( hBOOL(hAPP(hAPP(c_Set_Oinsert(V_y,T_a),V_A),V_x))
    | ~ hBOOL(hAPP(V_A,V_x)) )).

cnf(cls_SUP__const_0,axiom,
    ( ~ class_Complete__Lattice_Ocomplete__lattice(T_b)
    | c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,c_COMBK(V_M,T_b,T_a),T_a,T_b) = V_M
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_insert__subset_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B))
    | ~ c_lessequals(hAPP(c_Set_Oinsert(V_x,T_a),V_A),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_derivs__insertD_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(V_t,tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(V_t,tc_Hoare__Mirabelle_Otriple(T_a)),V_ts),T_a) )).

cnf(cls_comm__monoid__add_Ononempty__iff_2,axiom,
    ( hAPP(c_Set_Oinsert(V_x,T_a),V_xa) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_xa)) )).

cnf(cls_sup__compl__top_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),c_HOL_Ouminus__class_Ouminus(V_x,T_a)) = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_compl__sup__top_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),c_HOL_Ouminus__class_Ouminus(V_x,T_a)),V_x) = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_image__constant__conv_0,axiom,
    ( hAPP(c_Set_Oimage(c_COMBK(V_c,T_a,T_b),T_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool))) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_singleton__inject_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))) != hAPP(c_Set_Oinsert(V_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))
    | V_a = V_b )).

cnf(cls_Ints__minus_0,axiom,
    ( ~ class_Ring__and__Field_Oring__1(T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)),c_Int_Oring__1__class_OInts(T_a)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),c_Int_Oring__1__class_OInts(T_a))) )).

cnf(cls_inj__on__empty_0,axiom,
    ( c_Fun_Oinj__on(V_f,c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_doubleton__eq__iff_4,axiom,
    ( hAPP(c_Set_Oinsert(V_xa,T_a),hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) = hAPP(c_Set_Oinsert(V_x,T_a),hAPP(c_Set_Oinsert(V_xa,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) )).

cnf(cls_power__mono_0,axiom,
    ( ~ class_Ring__and__Field_Oordered__semidom(T_a)
    | c_lessequals(c_Power_Opower__class_Opower(V_a,V_n,T_a),c_Power_Opower__class_Opower(V_b,V_n,T_a),T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a)
    | ~ c_lessequals(V_a,V_b,T_a) )).

cnf(cls_neg__0__le__iff__le_1,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a)
    | ~ c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_neg__0__le__iff__le_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) )).

cnf(cls_COMBK__def_0,axiom,
    ( hAPP(c_COMBK(V_P,T_a,T_b),V_Q) = V_P )).

cnf(cls_COMBC__def_0,axiom,
    ( hAPP(hAPP(c_COMBC(V_P,T_b,T_c,T_a),V_Q),V_R) = hAPP(hAPP(V_P,V_R),V_Q) )).

cnf(cls_insert__commute_0,axiom,
    ( hAPP(c_Set_Oinsert(V_x,T_a),hAPP(c_Set_Oinsert(V_y,T_a),V_A)) = hAPP(c_Set_Oinsert(V_y,T_a),hAPP(c_Set_Oinsert(V_x,T_a),V_A)) )).

cnf(cls_le__zero__iff__pprt__id_1,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_OrderedGroup_Olordered__ab__group__add__class_Opprt(V_a,T_a) != V_a
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_le__zero__iff__pprt__id_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_OrderedGroup_Olordered__ab__group__add__class_Opprt(V_a,T_a) = V_a
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_inv__into__image__cancel_0,axiom,
    ( hAPP(c_Set_Oimage(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_a,T_b),T_b,T_a),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_S)) = V_S
    | ~ c_lessequals(V_S,V_A,tc_fun(T_a,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_singletonE_0,axiom,
    ( V_b = V_a
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_b),hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))) )).

cnf(cls_image__constant_0,axiom,
    ( hAPP(c_Set_Oimage(c_COMBK(V_c,T_b,T_a),T_a,T_b),V_A) = hAPP(c_Set_Oinsert(V_c,T_b),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_finite__surj_0,axiom,
    ( c_Finite__Set_Ofinite(V_B,T_b)
    | ~ c_lessequals(V_B,hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),tc_fun(T_b,tc_bool))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_inj__image__subset__iff_0,axiom,
    ( c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B),tc_fun(T_b,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inj__image__subset__iff_1,axiom,
    ( c_lessequals(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B),tc_fun(T_b,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inj__on__insert_0,axiom,
    ( c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | ~ c_Fun_Oinj__on(V_f,hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a,T_b) )).

cnf(cls_le__SUPI_0,axiom,
    ( ~ class_Complete__Lattice_Ocomplete__lattice(T_b)
    | c_lessequals(hAPP(V_M,V_i),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_M,T_a,T_b),T_b)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_i),V_A)) )).

cnf(cls_UN__upper_0,axiom,
    ( c_lessequals(hAPP(V_B,V_a),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_a,tc_fun(T_b,tc_bool)),tc_fun(T_b,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A)) )).

cnf(cls_insert__subset_1,axiom,
    ( c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(hAPP(c_Set_Oinsert(V_x,T_a),V_A),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__insertI2_0,axiom,
    ( c_lessequals(V_A,hAPP(c_Set_Oinsert(V_b,T_a),V_B),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_bot__fun__eq_0,axiom,
    ( ~ class_Orderings_Obot(T_b)
    | hAPP(c_Orderings_Obot__class_Obot(tc_fun(t_a,T_b)),v_x) = c_Orderings_Obot__class_Obot(T_b) )).

cnf(cls_zero__le__power_0,axiom,
    ( ~ class_Ring__and__Field_Oordered__semidom(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),c_Power_Opower__class_Opower(V_a,V_n,T_a),T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_neg__less__eq__nonneg_1,axiom,
    ( ~ class_OrderedGroup_Oordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),V_a,T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_neg__less__eq__nonneg_0,axiom,
    ( ~ class_OrderedGroup_Oordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a)
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),V_a,T_a) )).

cnf(cls_less__eq__neg__nonpos_1,axiom,
    ( ~ class_OrderedGroup_Oordered__ab__group__add(T_a)
    | c_lessequals(V_a,c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a)
    | ~ c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_less__eq__neg__nonpos_0,axiom,
    ( ~ class_OrderedGroup_Oordered__ab__group__add(T_a)
    | c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a)
    | ~ c_lessequals(V_a,c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) )).

cnf(cls_minus__le__self__iff_1,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),V_a,T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_minus__le__self__iff_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a)
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),V_a,T_a) )).

cnf(cls_le__minus__self__iff_1,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_lessequals(V_a,c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a)
    | ~ c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_le__minus__self__iff_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a)
    | ~ c_lessequals(V_a,c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) )).

cnf(cls_ball__empty_0,axiom,
    ( hBOOL(hAPP(V_P,V_x))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) )).

cnf(cls_empty__iff_0,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) )).

cnf(cls_emptyE_0,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) )).

cnf(cls_insertE_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A))
    | V_a = V_b
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Oinsert(V_b,T_a),V_A))) )).

cnf(cls_UNIV__I_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)))) )).

cnf(cls_insert__iff_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),hAPP(c_Set_Oinsert(V_x,T_a),V_A))) )).

cnf(cls_insertI1_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_a),hAPP(c_Set_Oinsert(V_a,T_a),V_B))) )).

cnf(cls_insertCI_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),hAPP(c_Set_Oinsert(V_x,T_a),V_B))) )).

cnf(cls_inj__on__contraD_0,axiom,
    ( hAPP(V_f,V_x) != hAPP(V_f,V_y)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_y),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | V_x = V_y
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_inj__on__iff_0,axiom,
    ( hAPP(V_f,V_x) != hAPP(V_f,V_y)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_y),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | V_x = V_y )).

cnf(cls_inj__on__def_0,axiom,
    ( hAPP(V_f,V_x) != hAPP(V_f,V_xa)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xa),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | V_x = V_xa )).

cnf(cls_inj__onD_0,axiom,
    ( hAPP(V_f,V_x) != hAPP(V_f,V_y)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | V_x = V_y
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_y),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_bex__empty_0,axiom,
    ( ~ hBOOL(hAPP(V_P,V_x))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) )).

cnf(cls_pred__equals__eq_0,axiom,
    ( hAPP(c_COMBC(c_in(T_a),T_a,tc_fun(T_a,tc_bool),tc_bool),V_R) != hAPP(c_COMBC(c_in(T_a),T_a,tc_fun(T_a,tc_bool),tc_bool),V_S)
    | V_R = V_S )).

cnf(cls_insert__ident_0,axiom,
    ( hAPP(c_Set_Oinsert(V_x,T_a),V_A) != hAPP(c_Set_Oinsert(V_x,T_a),V_B)
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | V_A = V_B )).

cnf(cls_UN__iff_2,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_b),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_b,tc_fun(T_a,tc_bool))))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_b),hAPP(V_B,V_x)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A)) )).

cnf(cls_UN__I_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),V_b),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_a,tc_fun(T_b,tc_bool))))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_b),hAPP(V_B,V_a)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A)) )).

cnf(cls_bex__Un_7,axiom,
    ( hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xbex__Un__1__3(V_A,V_B,V_P,T_a)))
    | ~ hBOOL(hAPP(V_P,V_x))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B)) )).

cnf(cls_bex__Un_5,axiom,
    ( hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xbex__Un__1__3(V_A,V_B,V_P,T_a)))
    | ~ hBOOL(hAPP(V_P,V_xa))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xa),V_A)) )).

cnf(cls_insert__absorb_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),V_A) = V_A
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A)) )).

cnf(cls_SUP1__iff_2,axiom,
    ( hBOOL(hAPP(c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_a,tc_fun(T_b,tc_bool)),V_b))
    | ~ hBOOL(hAPP(hAPP(V_B,V_x),V_b))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_SUP1__I_0,axiom,
    ( hBOOL(hAPP(c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_a,tc_fun(T_b,tc_bool)),V_b))
    | ~ hBOOL(hAPP(hAPP(V_B,V_a),V_b))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A)) )).

cnf(cls_minus__zero_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_HOL_Ozero__class_Ozero(T_a),T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_neg__equal__zero_1,axiom,
    ( ~ class_OrderedGroup_Oordered__ab__group__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_HOL_Ozero__class_Ozero(T_a),T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_power__eq__0__iff_0,axiom,
    ( ~ class_Ring__and__Field_Ozero__neq__one(T_a)
    | ~ class_Ring__and__Field_Ono__zero__divisors(T_a)
    | ~ class_Ring__and__Field_Omult__zero(T_a)
    | ~ class_Power_Opower(T_a)
    | c_Power_Opower__class_Opower(V_a,V_n,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_field__power__not__zero_0,axiom,
    ( ~ class_Ring__and__Field_Oring__1__no__zero__divisors(T_a)
    | c_Power_Opower__class_Opower(V_a,V_n,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_neg__0__equal__iff__equal_1,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ozero__class_Ozero(T_a) = c_HOL_Ouminus__class_Ouminus(c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_neg__equal__0__iff__equal_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(V_a,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_pprt__0_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_OrderedGroup_Olordered__ab__group__add__class_Opprt(c_HOL_Ozero__class_Ozero(T_a),T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_neg__0__equal__iff__equal_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ozero__class_Ozero(T_a) != c_HOL_Ouminus__class_Ouminus(V_a,T_a)
    | c_HOL_Ozero__class_Ozero(T_a) = V_a )).

cnf(cls_neg__equal__zero_0,axiom,
    ( ~ class_OrderedGroup_Oordered__ab__group__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(V_a,T_a) != V_a
    | V_a = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_inj__on__fun__updI_0,axiom,
    ( c_Fun_Oinj__on(c_Fun_Ofun__upd(V_f,V_x,V_y,T_a,T_b),V_A,T_a,T_b)
    | hBOOL(hAPP(hAPP(c_in(T_b),V_y),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_inj__image__mem__iff_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inj__image__mem__iff_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_a),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_a)),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_Suc__neq__Zero_0,axiom,
    ( c_Suc(V_m) != c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_nat_Osimps_I3_J_0,axiom,
    ( c_Suc(V_nat_H) != c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_Zero__neq__Suc_0,axiom,
    ( c_HOL_Ozero__class_Ozero(tc_nat) != c_Suc(V_m) )).

cnf(cls_nat_Osimps_I2_J_0,axiom,
    ( c_HOL_Ozero__class_Ozero(tc_nat) != c_Suc(V_nat_H) )).

cnf(cls_finite__range__imageI_0,axiom,
    ( c_Finite__Set_Ofinite(hAPP(c_Set_Oimage(hAPP(c_COMBB(V_f,T_a,T_c,T_b),V_g),T_b,T_c),c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool))),T_c)
    | ~ c_Finite__Set_Ofinite(hAPP(c_Set_Oimage(V_g,T_b,T_a),c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool))),T_a) )).

cnf(cls_le__supE_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_a,V_x,T_a)
    | ~ c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),V_x,T_a) )).

cnf(cls_le__supE_1,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_b,V_x,T_a)
    | ~ c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),V_x,T_a) )).

cnf(cls_le__supI1_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),T_a)
    | ~ c_lessequals(V_x,V_a,T_a) )).

cnf(cls_le__supI2_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),T_a)
    | ~ c_lessequals(V_x,V_b,T_a) )).

cnf(cls_le__sup__iff_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_x,V_z,T_a)
    | ~ c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),V_z,T_a) )).

cnf(cls_le__sup__iff_1,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_y,V_z,T_a)
    | ~ c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),V_z,T_a) )).

cnf(cls_le__iff__sup_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) = V_y
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_le__iff__sup_1,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) != V_y
    | c_lessequals(V_x,V_y,T_a) )).

cnf(cls_sup__absorb1_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) = V_x
    | ~ c_lessequals(V_y,V_x,T_a) )).

cnf(cls_sup__bot__left_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),c_Orderings_Obot__class_Obot(T_a)),V_x) = V_x )).

cnf(cls_sup__bot__right_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),c_Orderings_Obot__class_Obot(T_a)) = V_x )).

cnf(cls_sup__top__left_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),c_Orderings_Otop__class_Otop(T_a)),V_x) = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_sup__top__right_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),c_Orderings_Otop__class_Otop(T_a)) = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_le__supI_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),V_b),V_x,T_a)
    | ~ c_lessequals(V_b,V_x,T_a)
    | ~ c_lessequals(V_a,V_x,T_a) )).

cnf(cls_sup__ge1_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),T_a) )).

cnf(cls_sup__ge2_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(V_y,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),T_a) )).

cnf(cls_sup__least_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z),V_x,T_a)
    | ~ c_lessequals(V_z,V_x,T_a)
    | ~ c_lessequals(V_y,V_x,T_a) )).

cnf(cls_le__sup__iff_2,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),V_z,T_a)
    | ~ c_lessequals(V_y,V_z,T_a)
    | ~ c_lessequals(V_x,V_z,T_a) )).

cnf(cls_inf__sup__ord_I4_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | c_lessequals(V_y,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),T_a) )).

cnf(cls_inf__sup__ord_I3_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | c_lessequals(V_x,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y),T_a) )).

cnf(cls_sup__eq__bot__eq1_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_A),V_B) != c_Orderings_Obot__class_Obot(T_a)
    | V_A = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_sup__eq__bot__eq2_0,axiom,
    ( ~ class_Lattices_Obounded__lattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_A),V_B) != c_Orderings_Obot__class_Obot(T_a)
    | V_B = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_subset__image__iff_2,axiom,
    ( ~ c_lessequals(V_x,V_A,tc_fun(T_b,tc_bool))
    | c_lessequals(hAPP(c_Set_Oimage(V_f,T_b,T_a),V_x),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A),tc_fun(T_a,tc_bool)) )).

cnf(cls_image__mono_0,axiom,
    ( c_lessequals(hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B),tc_fun(T_b,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_image__cong_1,axiom,
    ( hAPP(V_f,c_ATP__Linkup_Osko__Set__Ximage__cong__1__1(V_x,V_f,V_g,T_a,T_b)) != hAPP(V_g,c_ATP__Linkup_Osko__Set__Ximage__cong__1__1(V_x,V_f,V_g,T_a,T_b))
    | hAPP(c_Set_Oimage(V_f,T_a,T_b),V_x) = hAPP(c_Set_Oimage(V_g,T_a,T_b),V_x) )).

cnf(cls_image__is__empty_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_image__insert_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_b,T_a),hAPP(c_Set_Oinsert(V_a,T_b),V_B)) = hAPP(c_Set_Oinsert(hAPP(V_f,V_a),T_a),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_B)) )).

cnf(cls_finite__imageI_0,axiom,
    ( c_Finite__Set_Ofinite(hAPP(c_Set_Oimage(V_h,T_a,T_b),V_F),T_b)
    | ~ c_Finite__Set_Ofinite(V_F,T_a) )).

cnf(cls_empty__is__image_0,axiom,
    ( c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) != hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A)
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_image__empty_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool))) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_empty__is__image_1,axiom,
    ( c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) = hAPP(c_Set_Oimage(V_f,T_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool))) )).

cnf(cls_Un__empty_2,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_UN__Un_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_b,tc_bool)),V_A),V_B),V_M,T_b,tc_fun(T_a,tc_bool)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_M,T_b,tc_fun(T_a,tc_bool))),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_B,V_M,T_b,tc_fun(T_a,tc_bool))) )).

cnf(cls_Un__subset__iff_1,axiom,
    ( c_lessequals(V_B,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__subset__iff_0,axiom,
    ( c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__mono_0,axiom,
    ( c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_C),V_D),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_B,V_D,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__Un__eq_1,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) != V_B
    | c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__absorb2_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) = V_A
    | ~ c_lessequals(V_B,V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__absorb1_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) = V_B
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__empty__right_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))) = V_A )).

cnf(cls_Un__empty__left_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),V_B) = V_B )).

cnf(cls_finite__Un_1,axiom,
    ( c_Finite__Set_Ofinite(V_G,T_a)
    | ~ c_Finite__Set_Ofinite(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_F),V_G),T_a) )).

cnf(cls_finite__Un_0,axiom,
    ( c_Finite__Set_Ofinite(V_F,T_a)
    | ~ c_Finite__Set_Ofinite(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_F),V_G),T_a) )).

cnf(cls_finite__UnI_0,axiom,
    ( c_Finite__Set_Ofinite(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_F),V_G),T_a)
    | ~ c_Finite__Set_Ofinite(V_G,T_a)
    | ~ c_Finite__Set_Ofinite(V_F,T_a) )).

cnf(cls_finite__Un_2,axiom,
    ( c_Finite__Set_Ofinite(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_F),V_G),T_a)
    | ~ c_Finite__Set_Ofinite(V_G,T_a)
    | ~ c_Finite__Set_Ofinite(V_F,T_a) )).

cnf(cls_Un__UNIV__right_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool))) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__UNIV__left_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool))),V_B) = c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__insert__right_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),hAPP(c_Set_Oinsert(V_a,T_a),V_B)) = hAPP(c_Set_Oinsert(V_a,T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)) )).

cnf(cls_Un__insert__left_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(c_Set_Oinsert(V_a,T_a),V_B)),V_C) = hAPP(c_Set_Oinsert(V_a,T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_C)) )).

cnf(cls_Un__subset__iff_2,axiom,
    ( c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_B,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__upper2_0,axiom,
    ( c_lessequals(V_B,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__upper1_0,axiom,
    ( c_lessequals(V_A,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__least_0,axiom,
    ( c_lessequals(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_B,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__empty_1,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | V_B = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_Un__empty_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_inj__on__Un_1,axiom,
    ( c_Fun_Oinj__on(V_f,V_B,T_a,T_b)
    | ~ c_Fun_Oinj__on(V_f,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),T_a,T_b) )).

cnf(cls_inj__on__Un_0,axiom,
    ( c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | ~ c_Fun_Oinj__on(V_f,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),T_a,T_b) )).

cnf(cls_COMBC__def__raw_0,axiom,
    ( hAPP(hAPP(c_COMBC(v_P,t_a,t_b,t_c),v_Q),v_R) = hAPP(hAPP(v_P,v_R),v_Q) )).

cnf(cls_triple__valid__Suc_0,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_t,T_a)
    | ~ c_Hoare__Mirabelle_Otriple__valid(c_Suc(V_n),V_t,T_a) )).

cnf(cls_hoare__valids__def_0,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_na,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_na),V_ts))
    | hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),v_sko__Hoare__Mirabelle__Xhoare__valids__def__1(V_G,V_n)),V_G))
    | ~ c_Hoare__Mirabelle_Ohoare__valids(V_G,V_ts,t_a) )).

cnf(cls_triples__valid__Suc_1,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_xa,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_xa),V_ts))
    | ~ c_Hoare__Mirabelle_Otriple__valid(c_Suc(V_n),v_sko__Hoare__Mirabelle__Xtriples__valid__Suc__1(V_n,V_ts),t_a) )).

cnf(cls_of__nat__in__Nats_0,axiom,
    ( ~ class_Ring__and__Field_Osemiring__1(T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),c_Nat_Osemiring__1__class_Oof__nat(V_n,T_a)),c_Nat_Osemiring__1__class_ONats(T_a))) )).

cnf(cls_power__0__Suc_0,axiom,
    ( ~ class_Ring__and__Field_Osemiring__0(T_a)
    | ~ class_Power_Opower(T_a)
    | c_Power_Opower__class_Opower(c_HOL_Ozero__class_Ozero(T_a),c_Suc(V_n),T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_neg__le__iff__le_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(V_a,V_b,T_a)
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_b,T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a) )).

cnf(cls_le__imp__neg__le_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ouminus__class_Ouminus(V_b,T_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a),T_a)
    | ~ c_lessequals(V_a,V_b,T_a) )).

cnf(cls_empty__subsetI_0,axiom,
    ( c_lessequals(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_bot__least_0,axiom,
    ( ~ class_Orderings_Obot(T_a)
    | c_lessequals(c_Orderings_Obot__class_Obot(T_a),V_x,T_a) )).

cnf(cls_insert__not__empty_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),V_A) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_le__zero__iff__zero__pprt_1,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_OrderedGroup_Olordered__ab__group__add__class_Opprt(V_a,T_a) != c_HOL_Ozero__class_Ozero(T_a)
    | c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_le__zero__iff__zero__pprt_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | c_OrderedGroup_Olordered__ab__group__add__class_Opprt(V_a,T_a) = c_HOL_Ozero__class_Ozero(T_a)
    | ~ c_lessequals(V_a,c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_insert__absorb2_0,axiom,
    ( hAPP(c_Set_Oinsert(V_x,T_a),hAPP(c_Set_Oinsert(V_x,T_a),V_A)) = hAPP(c_Set_Oinsert(V_x,T_a),V_A) )).

cnf(cls_finite__UN_0,axiom,
    ( c_Finite__Set_Ofinite(hAPP(V_B,V_x),T_b)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Finite__Set_Ofinite(c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_a,tc_fun(T_b,tc_bool)),T_b)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_insert__code_0,axiom,
    ( hBOOL(hAPP(V_A,V_x))
    | V_y = V_x
    | ~ hBOOL(hAPP(hAPP(c_Set_Oinsert(V_y,T_a),V_A),V_x)) )).

cnf(cls_UNION__empty__conv_I2_J_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_b,tc_fun(T_a,tc_bool)) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | hAPP(V_B,V_x) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A)) )).

cnf(cls_inv__into__f__f_0,axiom,
    ( hAPP(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_a,T_b),hAPP(V_f,V_x)) = V_x
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_b) )).

cnf(cls_inv__into__f__eq_0,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_aa),V_x),V_A))
    | ~ c_Fun_Oinj__on(V_f,V_A,T_aa,T_a)
    | hAPP(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_aa,T_a),hAPP(V_f,V_x)) = V_x )).

cnf(cls_UN__subset__iff_0,axiom,
    ( c_lessequals(hAPP(V_A,V_x),V_B,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_I))
    | ~ c_lessequals(c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_I,V_A,T_b,tc_fun(T_a,tc_bool)),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__insertI_0,axiom,
    ( c_lessequals(V_B,hAPP(c_Set_Oinsert(V_a,T_a),V_B),tc_fun(T_a,tc_bool)) )).

cnf(cls_finite__insert_0,axiom,
    ( c_Finite__Set_Ofinite(V_A,T_a)
    | ~ c_Finite__Set_Ofinite(hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a) )).

cnf(cls_finite__insert_1,axiom,
    ( c_Finite__Set_Ofinite(hAPP(c_Set_Oinsert(V_a,T_a),V_A),T_a)
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_finite_OemptyI_0,axiom,
    ( c_Finite__Set_Ofinite(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),T_a) )).

cnf(cls_of__nat__eq__iff_0,axiom,
    ( ~ class_Nat_Osemiring__char__0(T_a)
    | c_Nat_Osemiring__1__class_Oof__nat(V_m,T_a) != c_Nat_Osemiring__1__class_Oof__nat(V_n,T_a)
    | V_m = V_n )).

cnf(cls_option_Oinject_0,axiom,
    ( c_Option_Ooption_OSome(V_a,T_a) != c_Option_Ooption_OSome(V_a_H,T_a)
    | V_a = V_a_H )).

cnf(cls_Suc__inject_0,axiom,
    ( c_Suc(V_x) != c_Suc(V_y)
    | V_x = V_y )).

cnf(cls_nat_Oinject_0,axiom,
    ( c_Suc(V_nat) != c_Suc(V_nat_H)
    | V_nat = V_nat_H )).

cnf(cls_fun__upd__twist_0,axiom,
    ( c_Fun_Ofun__upd(c_Fun_Ofun__upd(V_m,V_a,V_b,T_a,T_b),V_c,V_d,T_a,T_b) = c_Fun_Ofun__upd(c_Fun_Ofun__upd(V_m,V_c,V_d,T_a,T_b),V_a,V_b,T_a,T_b)
    | V_a = V_c )).

cnf(cls_injD_0,axiom,
    ( hAPP(V_f,V_x) != hAPP(V_f,V_y)
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b)
    | V_x = V_y )).

cnf(cls_endo__inj__surj_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_a,T_a),V_A) = V_A
    | ~ c_Fun_Oinj__on(V_f,V_A,T_a,T_a)
    | ~ c_lessequals(hAPP(c_Set_Oimage(V_f,T_a,T_a),V_A),V_A,tc_fun(T_a,tc_bool))
    | ~ c_Finite__Set_Ofinite(V_A,T_a) )).

cnf(cls_UNION__empty__conv_I1_J_0,axiom,
    ( c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) != c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_b,tc_fun(T_a,tc_bool))
    | hAPP(V_B,V_x) = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A)) )).

cnf(cls_UN__extend__simps_I2_J_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),V_A,T_b,tc_fun(T_a,tc_bool))),V_B) = V_B )).

cnf(cls_UN__extend__simps_I3_J_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),V_B,T_b,tc_fun(T_a,tc_bool))) = V_A )).

cnf(cls_inj__image__eq__iff_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A) != hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B)
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b)
    | V_A = V_B )).

cnf(cls_inj__on__inv__into_0,axiom,
    ( c_Fun_Oinj__on(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_b,T_a),V_B,T_a,T_b)
    | ~ c_lessequals(V_B,hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A),tc_fun(T_a,tc_bool)) )).

cnf(cls_empty_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool)),T_a) )).

cnf(cls_compl__top__eq_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_Orderings_Otop__class_Otop(T_a),T_a) = c_Orderings_Obot__class_Obot(T_a) )).

cnf(cls_bot1E_0,axiom,
    ( ~ hBOOL(hAPP(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),V_x)) )).

cnf(cls_image__constant__conv_1,axiom,
    ( hAPP(c_Set_Oimage(c_COMBK(V_c,T_a,T_b),T_b,T_a),V_A) = hAPP(c_Set_Oinsert(V_c,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)) )).

cnf(cls_UNIV__not__empty_0,axiom,
    ( c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)) != c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) )).

cnf(cls_fun__upd__upd_0,axiom,
    ( c_Fun_Ofun__upd(c_Fun_Ofun__upd(V_f,V_x,V_y,T_a,T_b),V_x,V_z,T_a,T_b) = c_Fun_Ofun__upd(V_f,V_x,V_z,T_a,T_b) )).

cnf(cls_evaln__Suc_0,axiom,
    ( c_Natural_Oevaln(V_c,V_s,c_Suc(V_n),V_s_H)
    | ~ c_Natural_Oevaln(V_c,V_s,V_n,V_s_H) )).

cnf(cls_top1I_0,axiom,
    ( hBOOL(hAPP(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_x)) )).

cnf(cls_compl__eq__compl__iff_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ouminus__class_Ouminus(V_x,T_a) != c_HOL_Ouminus__class_Ouminus(V_y,T_a)
    | V_x = V_y )).

cnf(cls_neg__equal__iff__equal_0,axiom,
    ( ~ class_OrderedGroup_Ogroup__add(T_a)
    | c_HOL_Ouminus__class_Ouminus(V_a,T_a) != c_HOL_Ouminus__class_Ouminus(V_b,T_a)
    | V_a = V_b )).

cnf(cls_top__fun__eq_0,axiom,
    ( ~ class_Orderings_Otop(T_b)
    | hAPP(c_Orderings_Otop__class_Otop(tc_fun(t_a,T_b)),v_x) = c_Orderings_Otop__class_Otop(T_b) )).

cnf(cls_pred__subset__eq_0,axiom,
    ( c_lessequals(V_R,V_S,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(hAPP(c_COMBC(c_in(T_a),T_a,tc_fun(T_a,tc_bool),tc_bool),V_R),hAPP(c_COMBC(c_in(T_a),T_a,tc_fun(T_a,tc_bool),tc_bool),V_S),tc_fun(T_a,tc_bool)) )).

cnf(cls_pred__subset__eq_1,axiom,
    ( c_lessequals(hAPP(c_COMBC(c_in(T_a),T_a,tc_fun(T_a,tc_bool),tc_bool),V_R),hAPP(c_COMBC(c_in(T_a),T_a,tc_fun(T_a,tc_bool),tc_bool),V_S),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_R,V_S,tc_fun(T_a,tc_bool)) )).

cnf(cls_equalityI_0,axiom,
    ( V_A = V_B
    | ~ c_lessequals(V_B,V_A,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_com__det_0,axiom,
    ( V_u = V_t
    | ~ c_Natural_Oevalc(V_c,V_s,V_u)
    | ~ c_Natural_Oevalc(V_c,V_s,V_t) )).

cnf(cls_set__eq__subset_2,axiom,
    ( V_A = V_B
    | ~ c_lessequals(V_B,V_A,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_order__eq__iff_2,axiom,
    ( ~ class_Orderings_Oorder(T_a)
    | V_x = V_y
    | ~ c_lessequals(V_y,V_x,T_a)
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_order__antisym_0,axiom,
    ( ~ class_Orderings_Oorder(T_a)
    | V_x = V_y
    | ~ c_lessequals(V_y,V_x,T_a)
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_order__antisym__conv_0,axiom,
    ( ~ class_Orderings_Oorder(T_a)
    | V_x = V_y
    | ~ c_lessequals(V_x,V_y,T_a)
    | ~ c_lessequals(V_y,V_x,T_a) )).

cnf(cls_doubleton__eq__iff_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),hAPP(c_Set_Oinsert(V_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) != hAPP(c_Set_Oinsert(V_c,T_a),hAPP(c_Set_Oinsert(V_d,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))
    | V_a = V_d
    | V_a = V_c )).

cnf(cls_doubleton__eq__iff_1,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),hAPP(c_Set_Oinsert(V_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) != hAPP(c_Set_Oinsert(V_c,T_a),hAPP(c_Set_Oinsert(V_d,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))
    | V_b = V_c
    | V_a = V_c )).

cnf(cls_doubleton__eq__iff_2,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),hAPP(c_Set_Oinsert(V_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) != hAPP(c_Set_Oinsert(V_c,T_a),hAPP(c_Set_Oinsert(V_d,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))
    | V_a = V_d
    | V_b = V_d )).

cnf(cls_doubleton__eq__iff_3,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),hAPP(c_Set_Oinsert(V_b,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))) != hAPP(c_Set_Oinsert(V_c,T_a),hAPP(c_Set_Oinsert(V_d,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))
    | V_b = V_c
    | V_b = V_d )).

cnf(cls_UN__insert_0,axiom,
    ( c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(hAPP(c_Set_Oinsert(V_a,T_b),V_A),V_B,T_b,tc_fun(T_a,tc_bool)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(V_B,V_a)),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_A,V_B,T_b,tc_fun(T_a,tc_bool))) )).

cnf(cls_fun__upd__idem_0,axiom,
    ( c_Fun_Ofun__upd(V_f,V_x,hAPP(V_f,V_x),T_b,T_a) = V_f )).

cnf(cls_fun__upd__same_0,axiom,
    ( hAPP(c_Fun_Ofun__upd(V_f,V_x,V_y,T_b,T_a),V_x) = V_y )).

cnf(cls_fun__upd__triv_0,axiom,
    ( c_Fun_Ofun__upd(V_f,V_x,hAPP(V_f,V_x),T_a,T_b) = V_f )).

cnf(cls_fun__upd__apply_0,axiom,
    ( hAPP(c_Fun_Ofun__upd(V_f,V_x,V_y,T_a,T_aa),V_x) = V_y )).

cnf(cls_fun__upd__idem__iff_1,axiom,
    ( c_Fun_Ofun__upd(V_f,V_x,hAPP(V_f,V_x),T_aa,T_a) = V_f )).

cnf(cls_subset__empty_0,axiom,
    ( V_A = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__empty_1,axiom,
    ( c_lessequals(c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) )).

cnf(cls_power__le__imp__le__base_0,axiom,
    ( ~ class_Ring__and__Field_Oordered__semidom(T_a)
    | c_lessequals(V_a,V_b,T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_b,T_a)
    | ~ c_lessequals(c_Power_Opower__class_Opower(V_a,c_Suc(V_n),T_a),c_Power_Opower__class_Opower(V_b,c_Suc(V_n),T_a),T_a) )).

cnf(cls_empty__not__insert_0,axiom,
    ( c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)) != hAPP(c_Set_Oinsert(V_a,T_a),V_A) )).

cnf(cls_neg__le__0__iff__le_1,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),c_HOL_Ozero__class_Ozero(T_a),T_a)
    | ~ c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a) )).

cnf(cls_neg__le__0__iff__le_0,axiom,
    ( ~ class_OrderedGroup_Opordered__ab__group__add(T_a)
    | c_lessequals(c_HOL_Ozero__class_Ozero(T_a),V_a,T_a)
    | ~ c_lessequals(c_HOL_Ouminus__class_Ouminus(V_a,T_a),c_HOL_Ozero__class_Ozero(T_a),T_a) )).

cnf(cls_cut_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,V_ts,T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,V_G_H,T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G_H,V_ts,T_a) )).

cnf(cls_subset__UNIV_0,axiom,
    ( c_lessequals(V_A,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),tc_fun(T_a,tc_bool)) )).

cnf(cls_top__greatest_0,axiom,
    ( ~ class_Orderings_Otop(T_a)
    | c_lessequals(V_x,c_Orderings_Otop__class_Otop(T_a),T_a) )).

cnf(cls_UN__extend__simps_I1_J_0,axiom,
    ( hAPP(c_Set_Oinsert(V_a,T_a),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(c_Orderings_Obot__class_Obot(tc_fun(T_b,tc_bool)),V_B,T_b,tc_fun(T_a,tc_bool))) = hAPP(c_Set_Oinsert(V_a,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))) )).

cnf(cls_insert__code_1,axiom,
    ( hBOOL(hAPP(hAPP(c_Set_Oinsert(V_x,T_a),V_A),V_x)) )).

cnf(cls_insert__subset_2,axiom,
    ( c_lessequals(hAPP(c_Set_Oinsert(V_x,T_a),V_A),V_B,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B)) )).

cnf(cls_compl__bot__eq_0,axiom,
    ( ~ class_Lattices_Oboolean__algebra(T_a)
    | c_HOL_Ouminus__class_Ouminus(c_Orderings_Obot__class_Obot(T_a),T_a) = c_Orderings_Otop__class_Otop(T_a) )).

cnf(cls_finite__UNIV_0,axiom,
    ( ~ class_Finite__Set_Ofinite_Ofinite(T_a)
    | c_Finite__Set_Ofinite(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a) )).

cnf(cls_Suc__n__not__n_0,axiom,
    ( c_Suc(V_n) != V_n )).

cnf(cls_n__not__Suc__n_0,axiom,
    ( V_n != c_Suc(V_n) )).

cnf(cls_xt1_I6_J_0,axiom,
    ( ~ class_Orderings_Oorder(T_a)
    | c_lessequals(V_z,V_x,T_a)
    | ~ c_lessequals(V_z,V_y,T_a)
    | ~ c_lessequals(V_y,V_x,T_a) )).

cnf(cls_order__trans_0,axiom,
    ( ~ class_Orderings_Opreorder(T_a)
    | c_lessequals(V_x,V_z,T_a)
    | ~ c_lessequals(V_y,V_z,T_a)
    | ~ c_lessequals(V_x,V_y,T_a) )).

cnf(cls_equalityE_0,axiom,
    ( c_lessequals(V_x,V_x,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__refl_0,axiom,
    ( c_lessequals(V_A,V_A,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__trans_0,axiom,
    ( c_lessequals(V_A,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_B,V_C,tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_subset__inj__on_0,axiom,
    ( c_Fun_Oinj__on(V_f,V_A,T_a,T_b)
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_Fun_Oinj__on(V_f,V_B,T_a,T_b) )).

cnf(cls_predicate1D_0,axiom,
    ( hBOOL(hAPP(V_Q,V_x))
    | ~ hBOOL(hAPP(V_P,V_x))
    | ~ c_lessequals(V_P,V_Q,tc_fun(T_a,tc_bool)) )).

cnf(cls_order__eq__iff_0,axiom,
    ( ~ class_Orderings_Oorder(T_a)
    | c_lessequals(V_x,V_x,T_a) )).

cnf(cls_order__eq__refl_0,axiom,
    ( ~ class_Orderings_Opreorder(T_a)
    | c_lessequals(V_x,V_x,T_a) )).

cnf(cls_finite__subset_0,axiom,
    ( c_Finite__Set_Ofinite(V_A,T_a)
    | ~ c_Finite__Set_Ofinite(V_B,T_a)
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_rev__predicate1D_0,axiom,
    ( hBOOL(hAPP(V_Q,V_x))
    | ~ c_lessequals(V_P,V_Q,tc_fun(T_a,tc_bool))
    | ~ hBOOL(hAPP(V_P,V_x)) )).

cnf(cls_rev__finite__subset_0,axiom,
    ( c_Finite__Set_Ofinite(V_A,T_a)
    | ~ c_lessequals(V_A,V_B,tc_fun(T_a,tc_bool))
    | ~ c_Finite__Set_Ofinite(V_B,T_a) )).

cnf(cls_inv__f__f_0,axiom,
    ( hAPP(c_Hilbert__Choice_Oinv__into(c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),V_f,T_a,T_b),hAPP(V_f,V_x)) = V_x
    | ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_a,tc_bool)),T_a,T_b) )).

cnf(cls_inv__f__eq_0,axiom,
    ( ~ c_Fun_Oinj__on(V_f,c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)),T_aa,T_a)
    | hAPP(c_Hilbert__Choice_Oinv__into(c_Orderings_Otop__class_Otop(tc_fun(T_aa,tc_bool)),V_f,T_aa,T_a),hAPP(V_f,V_x)) = V_x )).

cnf(cls_singleton__iff_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))))) )).

cnf(cls_subset__singletonD_0,axiom,
    ( V_A = hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool)))
    | V_A = c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_A,hAPP(c_Set_Oinsert(V_x,T_a),c_Orderings_Obot__class_Obot(tc_fun(T_a,tc_bool))),tc_fun(T_a,tc_bool)) )).

cnf(cls_derivs__insertD_1,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,V_ts,T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(V_t,tc_Hoare__Mirabelle_Otriple(T_a)),V_ts),T_a) )).

cnf(cls_hoare__derivs_Oinsert_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(V_t,tc_Hoare__Mirabelle_Otriple(T_a)),V_ts),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,V_ts,T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(V_t,tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a) )).

cnf(cls_fun__upd__apply_1,axiom,
    ( hAPP(c_Fun_Ofun__upd(V_f,V_x,V_y,T_b,T_a),V_z) = hAPP(V_f,V_z)
    | V_z = V_x )).

cnf(cls_fun__upd__other_0,axiom,
    ( hAPP(c_Fun_Ofun__upd(V_f,V_x,V_y,T_a,T_b),V_z) = hAPP(V_f,V_z)
    | V_z = V_x )).

cnf(cls_insert__mono_0,axiom,
    ( c_lessequals(hAPP(c_Set_Oinsert(V_a,T_a),V_C),hAPP(c_Set_Oinsert(V_a,T_a),V_D),tc_fun(T_a,tc_bool))
    | ~ c_lessequals(V_C,V_D,tc_fun(T_a,tc_bool)) )).

cnf(cls_finite__imageD_0,axiom,
    ( c_Finite__Set_Ofinite(V_A,T_b)
    | ~ c_Fun_Oinj__on(V_f,V_A,T_b,T_a)
    | ~ c_Finite__Set_Ofinite(hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A),T_a) )).

cnf(cls_linorder__linear_0,axiom,
    ( ~ class_Orderings_Olinorder(T_a)
    | c_lessequals(V_y,V_x,T_a)
    | c_lessequals(V_x,V_y,T_a) )).

cnf(cls_hoare__valids__def_1,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_nb,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_nb),V_ts))
    | ~ c_Hoare__Mirabelle_Otriple__valid(V_n,v_sko__Hoare__Mirabelle__Xhoare__valids__def__1(V_G,V_n),t_a)
    | ~ c_Hoare__Mirabelle_Ohoare__valids(V_G,V_ts,t_a) )).

cnf(cls_the_Osimps_0,axiom,
    ( hAPP(c_Option_Othe(T_a),c_Option_Ooption_OSome(V_x,T_a)) = V_x )).

cnf(cls_finite__pointwise_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_b),tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),t_a),V_P),V_c0,t_a,tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),V_Q,t_a,tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b)),t_a,tc_Hoare__Mirabelle_Otriple(t_b)),V_U),t_b)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_b),tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),t_a),V_P_H),V_c0,t_a,tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),V_Q_H,t_a,tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b)),t_a,tc_Hoare__Mirabelle_Otriple(t_b)),V_U),t_b)
    | c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(t_b),hAPP(V_P_H,v_sko__Hoare__Mirabelle__Xfinite__pointwise__1(V_G,V_P,V_P_H,V_Q,V_Q_H,V_c0))),hAPP(V_c0,v_sko__Hoare__Mirabelle__Xfinite__pointwise__1(V_G,V_P,V_P_H,V_Q,V_Q_H,V_c0))),hAPP(V_Q_H,v_sko__Hoare__Mirabelle__Xfinite__pointwise__1(V_G,V_P,V_P_H,V_Q,V_Q_H,V_c0))),tc_Hoare__Mirabelle_Otriple(t_b)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(t_b),tc_bool))),t_b)
    | ~ c_Finite__Set_Ofinite(V_U,t_a) )).

cnf(cls_finite__pointwise_1,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_b),tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),t_a),V_P),V_c0,t_a,tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),V_Q,t_a,tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b)),t_a,tc_Hoare__Mirabelle_Otriple(t_b)),V_U),t_b)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_b),tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),t_a),V_P_H),V_c0,t_a,tc_Com_Ocom,tc_fun(tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b))),V_Q_H,t_a,tc_fun(t_b,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_b)),t_a,tc_Hoare__Mirabelle_Otriple(t_b)),V_U),t_b)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(t_b),hAPP(V_P,v_sko__Hoare__Mirabelle__Xfinite__pointwise__1(V_G,V_P,V_P_H,V_Q,V_Q_H,V_c0))),hAPP(V_c0,v_sko__Hoare__Mirabelle__Xfinite__pointwise__1(V_G,V_P,V_P_H,V_Q,V_Q_H,V_c0))),hAPP(V_Q,v_sko__Hoare__Mirabelle__Xfinite__pointwise__1(V_G,V_P,V_P_H,V_Q,V_Q_H,V_c0))),tc_Hoare__Mirabelle_Otriple(t_b)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(t_b),tc_bool))),t_b)
    | ~ c_Finite__Set_Ofinite(V_U,t_a) )).

cnf(cls_foldl__apply__inv_0,axiom,
    ( hAPP(V_g,hAPP(V_h,c_List_Osko__List__Xfoldl__apply__inv__1__1(V_g,V_h,T_b,T_a))) != c_List_Osko__List__Xfoldl__apply__inv__1__1(V_g,V_h,T_b,T_a)
    | c_List_Ofoldl(V_f,hAPP(V_g,V_s),V_xs,T_a,T_c) = hAPP(V_g,c_List_Ofoldl(hAPP(c_COMBB(c_COMBB(V_h,T_a,T_b,T_c),tc_fun(T_c,T_a),tc_fun(T_c,T_b),T_b),hAPP(c_COMBB(V_f,T_a,tc_fun(T_c,T_a),T_b),V_g)),V_s,V_xs,T_b,T_c)) )).

cnf(cls_sup__fun__eq__raw_0,axiom,
    ( ~ class_Lattices_Olattice(t_b)
    | hAPP(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(t_a,t_b)),v_f),v_g),v_x) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(t_b),hAPP(v_f,v_x)),hAPP(v_g,v_x)) )).

cnf(cls_BodyN_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Com_Ocom_OBODY,V_pn)),V_Q),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Com_Ocom_OBODY,V_pn)),V_Q),tc_Hoare__Mirabelle_Otriple(T_a)),V_G),hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_pn))),V_Q),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a) )).

cnf(cls_weak__Body_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Com_Ocom_OBODY,V_pn)),V_Q),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_pn))),V_Q),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a) )).

cnf(cls_evaln__elim__cases_I6_J_1,axiom,
    ( c_Natural_Oevaln(hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_P)),V_s,c_Natural_Osko__Natural__Xevaln__elim__cases__6__1(V_P,V_n,V_s,V_s1),V_s1)
    | ~ c_Natural_Oevaln(hAPP(c_Com_Ocom_OBODY,V_P),V_s,V_n,V_s1) )).

cnf(cls_evaln_OBody_0,axiom,
    ( c_Natural_Oevaln(hAPP(c_Com_Ocom_OBODY,V_pn),V_s0,c_Suc(V_n),V_s1)
    | ~ c_Natural_Oevaln(hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_pn)),V_s0,V_n,V_s1) )).

cnf(cls_Nats__0_0,axiom,
    ( ~ class_Ring__and__Field_Osemiring__1(T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),c_HOL_Ozero__class_Ozero(T_a)),c_Nat_Osemiring__1__class_ONats(T_a))) )).

cnf(cls_Ints__0_0,axiom,
    ( ~ class_Ring__and__Field_Oring__1(T_a)
    | hBOOL(hAPP(hAPP(c_in(T_a),c_HOL_Ozero__class_Ozero(T_a)),c_Int_Oring__1__class_OInts(T_a))) )).

cnf(cls_power__eq__0__iff_2,axiom,
    ( ~ class_Ring__and__Field_Ozero__neq__one(T_a)
    | ~ class_Ring__and__Field_Ono__zero__divisors(T_a)
    | ~ class_Ring__and__Field_Omult__zero(T_a)
    | ~ class_Power_Opower(T_a)
    | c_Power_Opower__class_Opower(c_HOL_Ozero__class_Ozero(T_a),V_n,T_a) = c_HOL_Ozero__class_Ozero(T_a)
    | V_n = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_power__0__left_1,axiom,
    ( ~ class_Ring__and__Field_Osemiring__0(T_a)
    | ~ class_Power_Opower(T_a)
    | c_Power_Opower__class_Opower(c_HOL_Ozero__class_Ozero(T_a),V_n,T_a) = c_HOL_Ozero__class_Ozero(T_a)
    | V_n = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_power__eq__0__iff_1,axiom,
    ( ~ class_Ring__and__Field_Ozero__neq__one(T_a)
    | ~ class_Ring__and__Field_Ono__zero__divisors(T_a)
    | ~ class_Ring__and__Field_Omult__zero(T_a)
    | ~ class_Power_Opower(T_a)
    | c_Power_Opower__class_Opower(V_a,c_HOL_Ozero__class_Ozero(tc_nat),T_a) != c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_of__nat__0_0,axiom,
    ( ~ class_Ring__and__Field_Osemiring__1(T_a)
    | c_Nat_Osemiring__1__class_Oof__nat(c_HOL_Ozero__class_Ozero(tc_nat),T_a) = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_sup__0__imp__0_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_a),c_HOL_Ouminus__class_Ouminus(V_a,T_a)) != c_HOL_Ozero__class_Ozero(T_a)
    | V_a = c_HOL_Ozero__class_Ozero(T_a) )).

cnf(cls_imageE_0,axiom,
    ( V_b = hAPP(V_f,c_ATP__Linkup_Osko__Set__XimageE__1__1(V_A,V_b,V_f,T_b,T_a))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_b),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_fun__upd__image_1,axiom,
    ( hAPP(c_Set_Oimage(c_Fun_Ofun__upd(V_f,V_x,V_y,T_b,T_a),T_b,T_a),V_A) = hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A)
    | hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A)) )).

cnf(cls_image__iff_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),c_ATP__Linkup_Osko__Set__Ximage__iff__1__1(V_A,V_f,V_z,T_b,T_a)),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_z),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_f__inv__into__f_0,axiom,
    ( hAPP(V_f,hAPP(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_b,T_a),V_y)) = V_y
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_y),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_image__cong_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_a,T_b),V_x) = hAPP(c_Set_Oimage(V_g,T_a,T_b),V_x)
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Ximage__cong__1__1(V_x,V_f,V_g,T_a,T_b)),V_x)) )).

cnf(cls_image__iff_1,axiom,
    ( V_z = hAPP(V_f,c_ATP__Linkup_Osko__Set__Ximage__iff__1__1(V_A,V_f,V_z,T_b,T_a))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_z),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_imageE_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),c_ATP__Linkup_Osko__Set__XimageE__1__1(V_A,V_b,V_f,T_b,T_a)),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_b),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_image__subset__iff_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),V_B))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A))
    | ~ c_lessequals(hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A),V_B,tc_fun(T_a,tc_bool)) )).

cnf(cls_inv__into__injective_0,axiom,
    ( hAPP(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_a,T_b),V_x) != hAPP(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_a,T_b),V_y)
    | V_x = V_y
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_y),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A))) )).

cnf(cls_inv__into__into_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),hAPP(c_Hilbert__Choice_Oinv__into(V_A,V_f,T_b,T_a),V_x)),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_rangeI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_b,T_a),c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool))))) )).

cnf(cls_insert__image_0,axiom,
    ( hAPP(c_Set_Oinsert(hAPP(V_f,V_x),T_b),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)) = hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_ball__Un_3,axiom,
    ( hBOOL(hAPP(V_P,V_xf))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xf),V_B))
    | ~ hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xball__Un__1__3(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_P,T_a))) )).

cnf(cls_ball__Un_2,axiom,
    ( hBOOL(hAPP(V_P,V_xg))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xg),V_A))
    | ~ hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xball__Un__1__3(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_P,T_a))) )).

cnf(cls_bex__Un_6,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xbex__Un__1__3(V_A,V_B,V_P,T_a)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | ~ hBOOL(hAPP(V_P,V_x))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_B)) )).

cnf(cls_bex__Un_4,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xbex__Un__1__3(V_A,V_B,V_P,T_a)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | ~ hBOOL(hAPP(V_P,V_xa))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xa),V_A)) )).

cnf(cls_ball__Un_1,axiom,
    ( hBOOL(hAPP(V_P,V_xd))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xd),V_B))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xball__Un__1__3(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_P,T_a)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_ball__Un_0,axiom,
    ( hBOOL(hAPP(V_P,V_xe))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xe),V_A))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xball__Un__1__3(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_P,T_a)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_ball__Un_7,axiom,
    ( hBOOL(hAPP(V_P,V_xc))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xc),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | ~ hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_B,V_P,T_a)))
    | ~ hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_A,V_P,T_a))) )).

cnf(cls_ball__Un_4,axiom,
    ( hBOOL(hAPP(V_P,V_x))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_B,V_P,T_a)),V_B))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_A,V_P,T_a)),V_A)) )).

cnf(cls_bex__Un_2,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_B,V_P,T_a)),V_B))
    | hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_A,V_P,T_a)))
    | ~ hBOOL(hAPP(V_P,V_xb))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xb),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_bex__Un_1,axiom,
    ( hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_B,V_P,T_a)))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_A,V_P,T_a)),V_A))
    | ~ hBOOL(hAPP(V_P,V_xb))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xb),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_UN__absorb_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_b,tc_bool)),hAPP(V_A,V_k)),c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_I,V_A,T_a,tc_fun(T_b,tc_bool))) = c_Complete__Lattice_Ocomplete__lattice__class_OSUPR(V_I,V_A,T_a,tc_fun(T_b,tc_bool))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_k),V_I)) )).

cnf(cls_bex__Un_3,axiom,
    ( hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_B,V_P,T_a)))
    | hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_A,V_P,T_a)))
    | ~ hBOOL(hAPP(V_P,V_xb))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xb),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_bex__Un_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_B,V_P,T_a)),V_B))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xbex__Un__1__2(V_A,V_P,T_a)),V_A))
    | ~ hBOOL(hAPP(V_P,V_xb))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xb),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_ball__Un_6,axiom,
    ( hBOOL(hAPP(V_P,V_xb))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xb),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_B,V_P,T_a)),V_B))
    | ~ hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_A,V_P,T_a))) )).

cnf(cls_ball__Un_5,axiom,
    ( hBOOL(hAPP(V_P,V_xa))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_xa),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | ~ hBOOL(hAPP(V_P,c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_B,V_P,T_a)))
    | hBOOL(hAPP(hAPP(c_in(T_a),c_ATP__Linkup_Osko__Set__Xball__Un__1__3(V_A,V_P,T_a)),V_A)) )).

cnf(cls_range__composition_0,axiom,
    ( hAPP(c_Set_Oimage(hAPP(c_COMBB(V_f,T_c,T_a,T_b),V_g),T_b,T_a),c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool))) = hAPP(c_Set_Oimage(V_f,T_c,T_a),hAPP(c_Set_Oimage(V_g,T_b,T_c),c_Orderings_Otop__class_Otop(tc_fun(T_b,tc_bool)))) )).

cnf(cls_bot__empty__eq_0,axiom,
    ( hAPP(c_Orderings_Obot__class_Obot(tc_fun(t_a,tc_bool)),v_x) = hAPP(hAPP(c_in(t_a),v_x),c_Orderings_Obot__class_Obot(tc_fun(t_a,tc_bool))) )).

cnf(cls_inj__on__Un__image__eq__iff_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A) != hAPP(c_Set_Oimage(V_f,T_a,T_b),V_B)
    | ~ c_Fun_Oinj__on(V_f,hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),T_a,T_b)
    | V_A = V_B )).

cnf(cls_com_Osize_I15_J_0,axiom,
    ( c_Nat_Osize__class_Osize(hAPP(c_Com_Ocom_OBODY,V_pname),tc_Com_Ocom) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_com_Osize_I7_J_0,axiom,
    ( c_Com_Ocom_Ocom__size(hAPP(c_Com_Ocom_OBODY,V_pname)) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_triple_Osize_I2_J_0,axiom,
    ( c_Nat_Osize__class_Osize(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1),V_com),V_fun2),tc_Hoare__Mirabelle_Otriple(T_a)) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_triple_Osize_I1_J_0,axiom,
    ( c_Hoare__Mirabelle_Otriple_Otriple__size(V_fa,hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1),V_com),V_fun2),T_a) = c_HOL_Ozero__class_Ozero(tc_nat) )).

cnf(cls_triple__valid__def2_0,axiom,
    ( hBOOL(hAPP(hAPP(V_Q,V_Z),V_s_H))
    | ~ c_Natural_Oevaln(V_c,V_s,V_n,V_s_H)
    | ~ hBOOL(hAPP(hAPP(V_P,V_Z),V_s))
    | ~ c_Hoare__Mirabelle_Otriple__valid(V_n,hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(t_a),V_P),V_c),V_Q),t_a) )).

cnf(cls_COMBK__def__raw_0,axiom,
    ( hAPP(c_COMBK(v_P,t_a,t_b),v_Q) = v_P )).

cnf(cls_triples__valid__Suc_0,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_x,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_x),V_ts))
    | hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),v_sko__Hoare__Mirabelle__Xtriples__valid__Suc__1(V_n,V_ts)),V_ts)) )).

cnf(cls_pprt__def__raw_0,axiom,
    ( ~ class_OrderedGroup_Olordered__ab__group__add(t_a)
    | c_OrderedGroup_Olordered__ab__group__add__class_Opprt(v_x,t_a) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(t_a),v_x),c_HOL_Ozero__class_Ozero(t_a)) )).

cnf(cls_sup__Un__eq_0,axiom,
    ( hAPP(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(t_a,tc_bool)),hAPP(c_COMBC(c_in(t_a),t_a,tc_fun(t_a,tc_bool),tc_bool),V_R)),hAPP(c_COMBC(c_in(t_a),t_a,tc_fun(t_a,tc_bool),tc_bool),V_S)),v_x) = hAPP(hAPP(c_in(t_a),v_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(t_a,tc_bool)),V_R),V_S)) )).

cnf(cls_COMBB__def__raw_0,axiom,
    ( hAPP(hAPP(c_COMBB(v_P,t_a,t_b,t_c),v_Q),v_R) = hAPP(v_P,hAPP(v_Q,v_R)) )).

cnf(cls_COMBS__def__raw_0,axiom,
    ( hAPP(c_COMBS(v_P,v_Q,t_a,t_b,t_c),v_R) = hAPP(hAPP(v_P,v_R),hAPP(v_Q,v_R)) )).

cnf(cls_evalc_OBody_0,axiom,
    ( c_Natural_Oevalc(hAPP(c_Com_Ocom_OBODY,V_pn),V_s0,V_s1)
    | ~ c_Natural_Oevalc(hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_pn)),V_s0,V_s1) )).

cnf(cls_evalc__elim__cases_I6_J_0,axiom,
    ( c_Natural_Oevalc(hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_P)),V_s,V_s1)
    | ~ c_Natural_Oevalc(hAPP(c_Com_Ocom_OBODY,V_P),V_s,V_s1) )).

cnf(cls_Body1_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oinsert(hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),hAPP(V_P,V_pn)),hAPP(c_Com_Ocom_OBODY,V_pn)),hAPP(V_Q,V_pn)),tc_Hoare__Mirabelle_Otriple(T_a)),c_Orderings_Obot__class_Obot(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool))),T_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Com_Opname),V_pn),V_Procs))
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool)),V_G),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(T_a),tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),tc_Com_Opname),V_P),c_Com_Ocom_OBODY,tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),V_Q,tc_Com_Opname,tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(T_a)),V_Procs)),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(T_a),tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),tc_Com_Opname),V_P),hAPP(c_COMBB(c_Option_Othe(tc_Com_Ocom),tc_Option_Ooption(tc_Com_Ocom),tc_Com_Ocom,tc_Com_Opname),c_Com_Obody),tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),V_Q,tc_Com_Opname,tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(T_a)),V_Procs),T_a) )).

cnf(cls_Body__triple__valid__Suc_0,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(c_Suc(V_n),hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Com_Ocom_OBODY,V_pn)),V_Q),T_a)
    | ~ c_Hoare__Mirabelle_Otriple__valid(V_n,hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_pn))),V_Q),T_a) )).

cnf(cls_Body__triple__valid__Suc_1,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Option_Othe(tc_Com_Ocom),hAPP(c_Com_Obody,V_pn))),V_Q),T_a)
    | ~ c_Hoare__Mirabelle_Otriple__valid(c_Suc(V_n),hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Com_Ocom_OBODY,V_pn)),V_Q),T_a) )).

cnf(cls_hoare__derivs_OBody_0,axiom,
    ( c_Hoare__Mirabelle_Ohoare__derivs(V_G,hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(T_a),tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),tc_Com_Opname),V_P),c_Com_Ocom_OBODY,tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),V_Q,tc_Com_Opname,tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(T_a)),V_Procs),T_a)
    | ~ c_Hoare__Mirabelle_Ohoare__derivs(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(tc_Hoare__Mirabelle_Otriple(T_a),tc_bool)),V_G),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(T_a),tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),tc_Com_Opname),V_P),c_Com_Ocom_OBODY,tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),V_Q,tc_Com_Opname,tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(T_a)),V_Procs)),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(T_a),tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),tc_Com_Opname),V_P),hAPP(c_COMBB(c_Option_Othe(tc_Com_Ocom),tc_Option_Ooption(tc_Com_Ocom),tc_Com_Ocom,tc_Com_Opname),c_Com_Obody),tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a))),V_Q,tc_Com_Opname,tc_fun(T_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(T_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(T_a)),V_Procs),T_a) )).

cnf(cls_image__Un_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_b,T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_b,tc_bool)),V_A),V_B)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A)),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_B)) )).

cnf(cls_UnCI_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A)) )).

cnf(cls_UnCI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_B)) )).

cnf(cls_image__image_0,axiom,
    ( hAPP(c_Set_Oimage(V_f,T_b,T_a),hAPP(c_Set_Oimage(V_g,T_c,T_b),V_A)) = hAPP(c_Set_Oimage(hAPP(c_COMBB(V_f,T_b,T_a,T_c),V_g),T_c,T_a),V_A) )).

cnf(cls_triple_Oinject_2,axiom,
    ( hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1),V_com),V_fun2) != hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1_H),V_com_H),V_fun2_H)
    | V_fun2 = V_fun2_H )).

cnf(cls_triple_Oinject_1,axiom,
    ( hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1),V_com),V_fun2) != hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1_H),V_com_H),V_fun2_H)
    | V_com = V_com_H )).

cnf(cls_triple_Oinject_0,axiom,
    ( hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1),V_com),V_fun2) != hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_fun1_H),V_com_H),V_fun2_H)
    | V_fun1 = V_fun1_H )).

cnf(cls_UnE_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_B))
    | hBOOL(hAPP(hAPP(c_in(T_a),V_c),V_A))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_c),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B))) )).

cnf(cls_sup1CI_1,axiom,
    ( hBOOL(hAPP(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_x))
    | ~ hBOOL(hAPP(V_A,V_x)) )).

cnf(cls_sup1CI_0,axiom,
    ( hBOOL(hAPP(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_x))
    | ~ hBOOL(hAPP(V_B,V_x)) )).

cnf(cls_sup1E_0,axiom,
    ( hBOOL(hAPP(V_B,V_x))
    | hBOOL(hAPP(V_A,V_x))
    | ~ hBOOL(hAPP(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B),V_x)) )).

cnf(cls_Body__triple__valid__0_0,axiom,
    ( c_Hoare__Mirabelle_Otriple__valid(c_HOL_Ozero__class_Ozero(tc_nat),hAPP(hAPP(hAPP(c_Hoare__Mirabelle_Otriple_Otriple(T_a),V_P),hAPP(c_Com_Ocom_OBODY,V_pn)),V_Q),T_a) )).

cnf(cls_sup__fun__eq_0,axiom,
    ( ~ class_Lattices_Olattice(T_b)
    | hAPP(hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(t_a,T_b)),V_f),V_g),v_x) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_b),hAPP(V_f,v_x)),hAPP(V_g,v_x)) )).

cnf(cls_Un__absorb_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_A) = V_A )).

cnf(cls_sup__idem_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_x) = V_x )).

cnf(cls_mem__def_1,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_S))
    | ~ hBOOL(hAPP(V_S,V_x)) )).

cnf(cls_mem__def_0,axiom,
    ( hBOOL(hAPP(V_S,V_x))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_S)) )).

cnf(cls_COMBB__def_0,axiom,
    ( hAPP(hAPP(c_COMBB(V_P,T_b,T_a,T_c),V_Q),V_R) = hAPP(V_P,hAPP(V_Q,V_R)) )).

cnf(cls_COMBS__def_0,axiom,
    ( hAPP(c_COMBS(V_P,V_Q,T_b,T_c,T_a),V_R) = hAPP(hAPP(V_P,V_R),hAPP(V_Q,V_R)) )).

cnf(cls_Un__left__commute_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_C)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_C)) )).

cnf(cls_Un__assoc_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)),V_C) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_C)) )).

cnf(cls_sup__assoc_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)),V_z) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)) )).

cnf(cls_sup__left__commute_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_z)) )).

cnf(cls_inf__sup__aci_I7_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_z)) )).

cnf(cls_inf__sup__aci_I6_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)),V_z) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_z)) )).

cnf(cls_Un__commute_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_B),V_A) )).

cnf(cls_sup__commute_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_x) )).

cnf(cls_inf__sup__aci_I5_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_y),V_x) )).

cnf(cls_com_Osimps_I6_J_0,axiom,
    ( hAPP(c_Com_Ocom_OBODY,V_pname) != hAPP(c_Com_Ocom_OBODY,V_pname_H)
    | V_pname = V_pname_H )).

cnf(cls_Un__left__absorb_0,axiom,
    ( hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(T_a,tc_bool)),V_A),V_B) )).

cnf(cls_sup__left__idem_0,axiom,
    ( ~ class_Lattices_Oupper__semilattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) )).

cnf(cls_inf__sup__aci_I8_J_0,axiom,
    ( ~ class_Lattices_Olattice(T_a)
    | hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y)) = hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(T_a),V_x),V_y) )).

cnf(cls_rev__image__eqI_0,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_aa),V_x),V_A))
    | hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_aa,T_a),V_A))) )).

cnf(cls_image__iff_2,axiom,
    ( ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A))
    | hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A))) )).

cnf(cls_image__eqI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_a),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_b,T_a),V_A)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_b),V_x),V_A)) )).

cnf(cls_imageI_0,axiom,
    ( hBOOL(hAPP(hAPP(c_in(T_b),hAPP(V_f,V_x)),hAPP(c_Set_Oimage(V_f,T_a,T_b),V_A)))
    | ~ hBOOL(hAPP(hAPP(c_in(T_a),V_x),V_A)) )).

cnf(cls_conjecture_0,negated_conjecture,
    ( hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),v_x),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_a),tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),tc_Com_Opname),v_P),c_Com_Ocom_OBODY,tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),v_Q,tc_Com_Opname,tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(t_a)),v_Procs))) )).

cnf(cls_conjecture_1,negated_conjecture,
    ( ~ c_Hoare__Mirabelle_Otriple__valid(c_HOL_Ozero__class_Ozero(tc_nat),v_x,t_a) )).

cnf(cls_conjecture_2,negated_conjecture,
    ( c_Hoare__Mirabelle_Otriple__valid(c_HOL_Ozero__class_Ozero(tc_nat),V_xa,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_xa),v_G)) )).

cnf(cls_conjecture_3,negated_conjecture,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_nb,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_nb),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_a),tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),tc_Com_Opname),v_P),hAPP(c_COMBB(c_Option_Othe(tc_Com_Ocom),tc_Option_Ooption(tc_Com_Ocom),tc_Com_Ocom,tc_Com_Opname),c_Com_Obody),tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),v_Q,tc_Com_Opname,tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(t_a)),v_Procs)))
    | hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),v_na(V_n)),hAPP(hAPP(c_Lattices_Oupper__semilattice__class_Osup(tc_fun(tc_Hoare__Mirabelle_Otriple(t_a),tc_bool)),v_G),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_a),tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),tc_Com_Opname),v_P),c_Com_Ocom_OBODY,tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),v_Q,tc_Com_Opname,tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(t_a)),v_Procs)))) )).

cnf(cls_conjecture_4,negated_conjecture,
    ( c_Hoare__Mirabelle_Otriple__valid(V_n,V_nc,t_a)
    | ~ hBOOL(hAPP(hAPP(c_in(tc_Hoare__Mirabelle_Otriple(t_a)),V_nc),hAPP(c_Set_Oimage(c_COMBS(c_COMBS(hAPP(c_COMBB(c_Hoare__Mirabelle_Otriple_Otriple(t_a),tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_fun(tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),tc_Com_Opname),v_P),hAPP(c_COMBB(c_Option_Othe(tc_Com_Ocom),tc_Option_Ooption(tc_Com_Ocom),tc_Com_Ocom,tc_Com_Opname),c_Com_Obody),tc_Com_Opname,tc_Com_Ocom,tc_fun(tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a))),v_Q,tc_Com_Opname,tc_fun(t_a,tc_fun(tc_Com_Ostate,tc_bool)),tc_Hoare__Mirabelle_Otriple(t_a)),tc_Com_Opname,tc_Hoare__Mirabelle_Otriple(t_a)),v_Procs)))
    | ~ c_Hoare__Mirabelle_Otriple__valid(V_n,v_na(V_n),t_a) )).

cnf(clsarity_fun__Complete__Lattice_Ocomplete__lattice,axiom,
    ( class_Complete__Lattice_Ocomplete__lattice(tc_fun(T_2,T_1))
    | ~ class_Complete__Lattice_Ocomplete__lattice(T_1) )).

cnf(clsarity_fun__Lattices_Oupper__semilattice,axiom,
    ( class_Lattices_Oupper__semilattice(tc_fun(T_2,T_1))
    | ~ class_Lattices_Olattice(T_1) )).

cnf(clsarity_fun__Lattices_Olower__semilattice,axiom,
    ( class_Lattices_Olower__semilattice(tc_fun(T_2,T_1))
    | ~ class_Lattices_Olattice(T_1) )).

cnf(clsarity_fun__Lattices_Odistrib__lattice,axiom,
    ( class_Lattices_Odistrib__lattice(tc_fun(T_2,T_1))
    | ~ class_Lattices_Odistrib__lattice(T_1) )).

cnf(clsarity_fun__Lattices_Obounded__lattice,axiom,
    ( class_Lattices_Obounded__lattice(tc_fun(T_2,T_1))
    | ~ class_Lattices_Obounded__lattice(T_1) )).

cnf(clsarity_fun__Lattices_Oboolean__algebra,axiom,
    ( class_Lattices_Oboolean__algebra(tc_fun(T_2,T_1))
    | ~ class_Lattices_Oboolean__algebra(T_1) )).

cnf(clsarity_fun__Finite__Set_Ofinite_Ofinite,axiom,
    ( class_Finite__Set_Ofinite_Ofinite(tc_fun(T_2,T_1))
    | ~ class_Finite__Set_Ofinite_Ofinite(T_1)
    | ~ class_Finite__Set_Ofinite_Ofinite(T_2) )).

cnf(clsarity_fun__Orderings_Opreorder,axiom,
    ( class_Orderings_Opreorder(tc_fun(T_2,T_1))
    | ~ class_Orderings_Opreorder(T_1) )).

cnf(clsarity_fun__Lattices_Olattice,axiom,
    ( class_Lattices_Olattice(tc_fun(T_2,T_1))
    | ~ class_Lattices_Olattice(T_1) )).

cnf(clsarity_fun__Orderings_Oorder,axiom,
    ( class_Orderings_Oorder(tc_fun(T_2,T_1))
    | ~ class_Orderings_Oorder(T_1) )).

cnf(clsarity_fun__Orderings_Otop,axiom,
    ( class_Orderings_Otop(tc_fun(T_2,T_1))
    | ~ class_Orderings_Otop(T_1) )).

cnf(clsarity_fun__Orderings_Obot,axiom,
    ( class_Orderings_Obot(tc_fun(T_2,T_1))
    | ~ class_Orderings_Obot(T_1) )).

cnf(clsarity_fun__HOL_Ouminus,axiom,
    ( class_HOL_Ouminus(tc_fun(T_2,T_1))
    | ~ class_HOL_Ouminus(T_1) )).

cnf(clsarity_fun__HOL_Ominus,axiom,
    ( class_HOL_Ominus(tc_fun(T_2,T_1))
    | ~ class_HOL_Ominus(T_1) )).

cnf(clsarity_fun__HOL_Oord,axiom,
    ( class_HOL_Oord(tc_fun(T_2,T_1))
    | ~ class_HOL_Oord(T_1) )).

cnf(clsarity_nat__Ring__and__Field_Ono__zero__divisors,axiom,
    ( class_Ring__and__Field_Ono__zero__divisors(tc_nat) )).

cnf(clsarity_nat__Ring__and__Field_Oordered__semidom,axiom,
    ( class_Ring__and__Field_Oordered__semidom(tc_nat) )).

cnf(clsarity_nat__Ring__and__Field_Ozero__neq__one,axiom,
    ( class_Ring__and__Field_Ozero__neq__one(tc_nat) )).

cnf(clsarity_nat__Lattices_Oupper__semilattice,axiom,
    ( class_Lattices_Oupper__semilattice(tc_nat) )).

cnf(clsarity_nat__Lattices_Olower__semilattice,axiom,
    ( class_Lattices_Olower__semilattice(tc_nat) )).

cnf(clsarity_nat__Ring__and__Field_Osemiring__1,axiom,
    ( class_Ring__and__Field_Osemiring__1(tc_nat) )).

cnf(clsarity_nat__Ring__and__Field_Osemiring__0,axiom,
    ( class_Ring__and__Field_Osemiring__0(tc_nat) )).

cnf(clsarity_nat__Ring__and__Field_Omult__zero,axiom,
    ( class_Ring__and__Field_Omult__zero(tc_nat) )).

cnf(clsarity_nat__Lattices_Odistrib__lattice,axiom,
    ( class_Lattices_Odistrib__lattice(tc_nat) )).

cnf(clsarity_nat__Nat_Osemiring__char__0,axiom,
    ( class_Nat_Osemiring__char__0(tc_nat) )).

cnf(clsarity_nat__Orderings_Opreorder,axiom,
    ( class_Orderings_Opreorder(tc_nat) )).

cnf(clsarity_nat__Orderings_Olinorder,axiom,
    ( class_Orderings_Olinorder(tc_nat) )).

cnf(clsarity_nat__Lattices_Olattice,axiom,
    ( class_Lattices_Olattice(tc_nat) )).

cnf(clsarity_nat__Orderings_Oorder,axiom,
    ( class_Orderings_Oorder(tc_nat) )).

cnf(clsarity_nat__Orderings_Obot,axiom,
    ( class_Orderings_Obot(tc_nat) )).

cnf(clsarity_nat__Power_Opower,axiom,
    ( class_Power_Opower(tc_nat) )).

cnf(clsarity_nat__HOL_Ominus,axiom,
    ( class_HOL_Ominus(tc_nat) )).

cnf(clsarity_nat__HOL_Oord,axiom,
    ( class_HOL_Oord(tc_nat) )).

cnf(clsarity_bool__Complete__Lattice_Ocomplete__lattice,axiom,
    ( class_Complete__Lattice_Ocomplete__lattice(tc_bool) )).

cnf(clsarity_bool__Lattices_Oupper__semilattice,axiom,
    ( class_Lattices_Oupper__semilattice(tc_bool) )).

cnf(clsarity_bool__Lattices_Olower__semilattice,axiom,
    ( class_Lattices_Olower__semilattice(tc_bool) )).

cnf(clsarity_bool__Lattices_Odistrib__lattice,axiom,
    ( class_Lattices_Odistrib__lattice(tc_bool) )).

cnf(clsarity_bool__Lattices_Obounded__lattice,axiom,
    ( class_Lattices_Obounded__lattice(tc_bool) )).

cnf(clsarity_bool__Lattices_Oboolean__algebra,axiom,
    ( class_Lattices_Oboolean__algebra(tc_bool) )).

cnf(clsarity_bool__Finite__Set_Ofinite_Ofinite,axiom,
    ( class_Finite__Set_Ofinite_Ofinite(tc_bool) )).

cnf(clsarity_bool__Orderings_Opreorder,axiom,
    ( class_Orderings_Opreorder(tc_bool) )).

cnf(clsarity_bool__Lattices_Olattice,axiom,
    ( class_Lattices_Olattice(tc_bool) )).

cnf(clsarity_bool__Orderings_Oorder,axiom,
    ( class_Orderings_Oorder(tc_bool) )).

cnf(clsarity_bool__Orderings_Otop,axiom,
    ( class_Orderings_Otop(tc_bool) )).

cnf(clsarity_bool__Orderings_Obot,axiom,
    ( class_Orderings_Obot(tc_bool) )).

cnf(clsarity_bool__HOL_Ouminus,axiom,
    ( class_HOL_Ouminus(tc_bool) )).

cnf(clsarity_bool__HOL_Ominus,axiom,
    ( class_HOL_Ominus(tc_bool) )).

cnf(clsarity_bool__HOL_Oord,axiom,
    ( class_HOL_Oord(tc_bool) )).

cnf(clsarity_Option__Ooption__Finite__Set_Ofinite_Ofinite,axiom,
    ( class_Finite__Set_Ofinite_Ofinite(tc_Option_Ooption(T_1))
    | ~ class_Finite__Set_Ofinite_Ofinite(T_1) )).

cnf(cls_ATP__Linkup_OCOMBS__def_0,axiom,
    ( hAPP(c_COMBS(V_P,V_Q,T_b,T_c,T_a),V_R) = hAPP(hAPP(V_P,V_R),hAPP(V_Q,V_R)) )).

cnf(cls_ATP__Linkup_OCOMBC__def_0,axiom,
    ( hAPP(hAPP(c_COMBC(V_P,T_b,T_c,T_a),V_Q),V_R) = hAPP(hAPP(V_P,V_R),V_Q) )).

cnf(cls_ATP__Linkup_OCOMBB__def_0,axiom,
    ( hAPP(hAPP(c_COMBB(V_P,T_b,T_a,T_c),V_Q),V_R) = hAPP(V_P,hAPP(V_Q,V_R)) )).

cnf(cls_ATP__Linkup_Oequal__imp__fequal_0,axiom,
    ( c_fequal(V_x,V_x,T_a) )).

cnf(cls_ATP__Linkup_Ofequal__imp__equal_0,axiom,
    ( V_X = V_Y
    | ~ c_fequal(V_X,V_Y,T_a) )).

%------------------------------------------------------------------------------
