%------------------------------------------------------------------------------
% File     : SET844-1 : TPTP v6.4.0. Released v3.2.0.
% Domain   : Set Theory
% Problem  : Problem about Zorn's lemma
% Version  : [Pau06] axioms : Especial.
% English  :

% Refs     : [Pau06] Paulson (2006), Email to G. Sutcliffe
% Source   : [Pau06]
% Names    : Zorn__equal_succ_Union_simpler_1 [Pau06]

% Status   : Unsatisfiable
% Rating   : 0.27 v6.4.0, 0.20 v6.3.0, 0.09 v6.2.0, 0.20 v6.1.0, 0.29 v6.0.0, 0.20 v5.5.0, 0.70 v5.4.0, 0.65 v5.3.0, 0.67 v5.2.0, 0.56 v5.1.0, 0.65 v5.0.0, 0.64 v4.1.0, 0.62 v4.0.1, 0.45 v3.7.0, 0.20 v3.5.0, 0.27 v3.4.0, 0.33 v3.3.0, 0.50 v3.2.0
% Syntax   : Number of clauses     : 1367 (  29 non-Horn; 220 unit;1279 RR)
%            Number of atoms       : 2578 ( 195 equality)
%            Maximal clause size   :    4 (   2 average)
%            Number of predicates  :   82 (   0 propositional; 1-3 arity)
%            Number of functors    :  126 (  20 constant; 0-6 arity)
%            Number of variables   : 1930 ( 211 singleton)
%            Maximal term depth    :    4 (   1 average)
% SPC      : CNF_UNS_RFO_SEQ_NHN

% Comments : The problems in the [Pau06] collection each have very many axioms,
%            of which only a small selection are required for the refutation.
%            The mission is to find those few axioms, after which a refutation
%            can be quite easily found.
%------------------------------------------------------------------------------
include('Axioms/MSC001-2.ax').
include('Axioms/MSC001-0.ax').
%------------------------------------------------------------------------------
cnf(cls_Set_OUnion__least_0,axiom,
    ( c_in(c_Main_OUnion__least__1(V_A,V_C,T_a),V_A,tc_set(T_a))
    | c_lessequals(c_Union(V_A,T_a),V_C,tc_set(T_a)) )).

cnf(cls_Set_OUnion__least_1,axiom,
    ( ~ c_lessequals(c_Main_OUnion__least__1(V_A,V_C,T_a),V_C,tc_set(T_a))
    | c_lessequals(c_Union(V_A,T_a),V_C,tc_set(T_a)) )).

cnf(cls_Set_OUnion__upper_0,axiom,
    ( ~ c_in(V_B,V_A,tc_set(T_a))
    | c_lessequals(V_B,c_Union(V_A,T_a),tc_set(T_a)) )).

cnf(cls_Zorn_OAbrial__axiom1_0,axiom,
    ( c_lessequals(V_x,c_Zorn_Osucc(V_S,V_x,T_a),tc_set(tc_set(T_a))) )).

cnf(cls_Zorn_OTFin_OsuccI_0,axiom,
    ( ~ c_in(V_x,c_Zorn_OTFin(V_S,T_a),tc_set(tc_set(T_a)))
    | c_in(c_Zorn_Osucc(V_S,V_x,T_a),c_Zorn_OTFin(V_S,T_a),tc_set(tc_set(T_a))) )).

cnf(cls_Zorn_OTFin__UnionI_0,axiom,
    ( ~ c_lessequals(V_Y,c_Zorn_OTFin(V_S,T_a),tc_set(tc_set(tc_set(T_a))))
    | c_in(c_Union(V_Y,tc_set(T_a)),c_Zorn_OTFin(V_S,T_a),tc_set(tc_set(T_a))) )).

cnf(cls_Zorn_Oeq__succ__upper_0,axiom,
    ( ~ c_in(V_m,c_Zorn_OTFin(V_S,T_a),tc_set(tc_set(T_a)))
    | ~ c_in(V_n,c_Zorn_OTFin(V_S,T_a),tc_set(tc_set(T_a)))
    | V_m != c_Zorn_Osucc(V_S,V_m,T_a)
    | c_lessequals(V_n,V_m,tc_set(tc_set(T_a))) )).

cnf(cls_conjecture_0,negated_conjecture,
    ( c_in(v_m,c_Zorn_OTFin(v_S,t_a),tc_set(tc_set(t_a))) )).

cnf(cls_conjecture_1,negated_conjecture,
    ( v_m = c_Zorn_Osucc(v_S,v_m,t_a) )).

cnf(cls_conjecture_2,negated_conjecture,
    ( ~ c_lessequals(c_Union(c_Zorn_OTFin(v_S,t_a),tc_set(t_a)),v_m,tc_set(tc_set(t_a))) )).

%------------------------------------------------------------------------------
