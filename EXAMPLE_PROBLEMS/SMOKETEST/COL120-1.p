%------------------------------------------------------------------------------
% File     : COL120-1 : TPTP v7.0.0. Released v3.2.0.
% Domain   : Combinatory Logic
% Problem  : Problem about combinators
% Version  : [Pau06] axioms : Especial.
% English  :

% Refs     : [Pau06] Paulson (2006), Email to G. Sutcliffe
% Source   : [Pau06]
% Names    : Comb__diamond_rtrancl_1 [Pau06]

% Status   : Unsatisfiable
% Rating   : 0.00 v7.0.0, 0.13 v6.4.0, 0.07 v6.3.0, 0.00 v6.2.0, 0.10 v6.1.0, 0.00 v5.5.0, 0.25 v5.3.0, 0.22 v5.2.0, 0.19 v5.1.0, 0.29 v4.1.0, 0.23 v4.0.1, 0.36 v4.0.0, 0.18 v3.7.0, 0.20 v3.5.0, 0.18 v3.4.0, 0.25 v3.3.0, 0.36 v3.2.0
% Syntax   : Number of clauses     : 1371 (  28 non-Horn; 224 unit;1285 RR)
%            Number of atoms       : 2585 ( 203 equality)
%            Maximal clause size   :    4 (   2 average)
%            Number of predicates  :   83 (   0 propositional; 1-3 arity)
%            Number of functors    :  128 (  23 constant; 0-6 arity)
%            Number of variables   : 1940 ( 222 singleton)
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
cnf(cls_Comb_Ocomb_Odistinct__1__iff1_0,axiom,
    ( c_Comb_Ocomb_OK != c_Comb_Ocomb_OS )).

cnf(cls_Comb_Ocomb_Odistinct__2__iff1_0,axiom,
    ( c_Comb_Ocomb_OS != c_Comb_Ocomb_OK )).

cnf(cls_Comb_Ocomb_Odistinct__3__iff1_0,axiom,
    ( c_Comb_Ocomb_OK != c_Comb_Ocomb_Oop_A_D_D(V_comb1_H,V_comb2_H) )).

cnf(cls_Comb_Ocomb_Odistinct__4__iff1_0,axiom,
    ( c_Comb_Ocomb_Oop_A_D_D(V_comb1_H,V_comb2_H) != c_Comb_Ocomb_OK )).

cnf(cls_Comb_Ocomb_Odistinct__5__iff1_0,axiom,
    ( c_Comb_Ocomb_OS != c_Comb_Ocomb_Oop_A_D_D(V_comb1_H,V_comb2_H) )).

cnf(cls_Comb_Ocomb_Odistinct__6__iff1_0,axiom,
    ( c_Comb_Ocomb_Oop_A_D_D(V_comb1_H,V_comb2_H) != c_Comb_Ocomb_OS )).

cnf(cls_Comb_Ocomb_Oinject__iff1_0,axiom,
    ( c_Comb_Ocomb_Oop_A_D_D(V_comb1,V_comb2) != c_Comb_Ocomb_Oop_A_D_D(V_comb1_H,V_comb2_H)
    | V_comb1 = V_comb1_H )).

cnf(cls_Comb_Ocomb_Oinject__iff1_1,axiom,
    ( c_Comb_Ocomb_Oop_A_D_D(V_comb1,V_comb2) != c_Comb_Ocomb_Oop_A_D_D(V_comb1_H,V_comb2_H)
    | V_comb2 = V_comb2_H )).

cnf(cls_Comb_Odiamond__strip__lemmaE_0,axiom,
    ( ~ c_Comb_Odiamond(V_r,T_a)
    | ~ c_in(c_Pair(V_x,V_y_H,T_a,T_a),V_r,tc_prod(T_a,T_a))
    | ~ c_in(c_Pair(V_x,V_y,T_a,T_a),c_Transitive__Closure_Ortrancl(V_r,T_a),tc_prod(T_a,T_a))
    | c_in(c_Pair(V_y_H,c_Comb_Odiamond__strip__lemmaE__1(V_r,V_y,V_y_H,T_a),T_a,T_a),c_Transitive__Closure_Ortrancl(V_r,T_a),tc_prod(T_a,T_a)) )).

cnf(cls_Comb_Odiamond__strip__lemmaE_1,axiom,
    ( ~ c_Comb_Odiamond(V_r,T_a)
    | ~ c_in(c_Pair(V_x,V_y_H,T_a,T_a),V_r,tc_prod(T_a,T_a))
    | ~ c_in(c_Pair(V_x,V_y,T_a,T_a),c_Transitive__Closure_Ortrancl(V_r,T_a),tc_prod(T_a,T_a))
    | c_in(c_Pair(V_y,c_Comb_Odiamond__strip__lemmaE__1(V_r,V_y,V_y_H,T_a),T_a,T_a),V_r,tc_prod(T_a,T_a)) )).

cnf(cls_Transitive__Closure_Ortrancl__trans_0,axiom,
    ( ~ c_in(c_Pair(V_b,V_c,T_a,T_a),c_Transitive__Closure_Ortrancl(V_r,T_a),tc_prod(T_a,T_a))
    | ~ c_in(c_Pair(V_a,V_b,T_a,T_a),c_Transitive__Closure_Ortrancl(V_r,T_a),tc_prod(T_a,T_a))
    | c_in(c_Pair(V_a,V_c,T_a,T_a),c_Transitive__Closure_Ortrancl(V_r,T_a),tc_prod(T_a,T_a)) )).

cnf(cls_conjecture_0,negated_conjecture,
    ( c_Comb_Odiamond(v_r,t_a) )).

cnf(cls_conjecture_1,negated_conjecture,
    ( c_in(c_Pair(v_y,v_x,t_a,t_a),c_Transitive__Closure_Ortrancl(v_r,t_a),tc_prod(t_a,t_a)) )).

cnf(cls_conjecture_2,negated_conjecture,
    ( ~ c_in(c_Pair(v_x,V_U,t_a,t_a),c_Transitive__Closure_Ortrancl(v_r,t_a),tc_prod(t_a,t_a))
    | ~ c_in(c_Pair(v_y,V_U,t_a,t_a),c_Transitive__Closure_Ortrancl(v_r,t_a),tc_prod(t_a,t_a)) )).

%------------------------------------------------------------------------------
