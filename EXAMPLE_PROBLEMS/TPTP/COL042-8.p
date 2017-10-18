%--------------------------------------------------------------------------
% File     : COL042-8 : TPTP v6.4.0. Released v2.1.0.
% Domain   : Combinatory Logic
% Problem  : Strong fixed point for B and W1
% Version  : [WM88] (equality) axioms.
%            Theorem formulation : The fixed point is provided and checked.
% English  : The strong fixed point property holds for the set
%            P consisting of the combinators B and W1, where ((Bx)y)z
%            = x(yz), (W1x)y = (yx)x.

% Refs     : [WM88]  Wos & McCune (1988), Challenge Problems Focusing on Eq
%          : [Wos93] Wos (1993), The Kernel Strategy and Its Use for the St
% Source   : [TPTP]
% Names    :

% Status   : Unsatisfiable
% Rating   : 0.26 v6.4.0, 0.32 v6.3.0, 0.29 v6.2.0, 0.36 v6.1.0, 0.31 v6.0.0, 0.48 v5.5.0, 0.47 v5.4.0, 0.40 v5.3.0, 0.33 v5.2.0, 0.36 v5.1.0, 0.33 v5.0.0, 0.43 v4.1.0, 0.36 v4.0.0, 0.31 v3.7.0, 0.11 v3.4.0, 0.12 v3.3.0, 0.14 v3.1.0, 0.11 v2.7.0, 0.18 v2.6.0, 0.00 v2.5.0, 0.25 v2.4.0, 0.00 v2.2.1, 0.50 v2.2.0, 0.80 v2.1.0
% Syntax   : Number of clauses     :    4 (   0 non-Horn;   4 unit;   2 RR)
%            Number of atoms       :    4 (   4 equality)
%            Maximal clause size   :    1 (   1 average)
%            Number of predicates  :    1 (   0 propositional; 2-2 arity)
%            Number of functors    :    5 (   4 constant; 0-2 arity)
%            Number of variables   :    5 (   0 singleton)
%            Maximal term depth    :    7 (   3 average)
% SPC      : CNF_UNS_RFO_PEQ_UEQ

% Comments :
%--------------------------------------------------------------------------
cnf(b_definition,axiom,
    ( apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z)) )).

cnf(w1_definition,axiom,
    ( apply(apply(w1,X),Y) = apply(apply(Y,X),X) )).

cnf(strong_fixed_point,axiom,
    ( strong_fixed_point = apply(apply(b,apply(apply(b,apply(w1,w1)),apply(apply(b,apply(b,w1)),b))),b) )).

cnf(prove_strong_fixed_point,negated_conjecture,
    (  apply(strong_fixed_point,fixed_pt) != apply(fixed_pt,apply(strong_fixed_point,fixed_pt)) )).

%--------------------------------------------------------------------------
