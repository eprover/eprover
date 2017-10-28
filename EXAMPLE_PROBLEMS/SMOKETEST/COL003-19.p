%--------------------------------------------------------------------------
% File     : COL003-19 : TPTP v7.0.0. Released v2.1.0.
% Domain   : Combinatory Logic
% Problem  : Strong fixed point for B and W
% Version  : [WM88] (equality) axioms.
%            Theorem formulation : The fixed point is provided and checked.
% English  : The strong fixed point property holds for the set
%            P consisting of the combinators B and W alone, where ((Bx)y)z
%            = x(yz) and (Wx)y = (xy)y.

% Refs     : [WM88]  Wos & McCune (1988), Challenge Problems Focusing on Eq
%          : [Wos93] Wos (1993), The Kernel Strategy and Its Use for the St
% Source   : [Wos93]
% Names    :

% Status   : Unsatisfiable
% Rating   : 0.17 v7.0.0, 0.21 v6.4.0, 0.26 v6.3.0, 0.29 v6.1.0, 0.38 v6.0.0, 0.48 v5.5.0, 0.47 v5.3.0, 0.42 v5.2.0, 0.43 v5.1.0, 0.33 v5.0.0, 0.36 v4.0.1, 0.29 v4.0.0, 0.31 v3.7.0, 0.22 v3.4.0, 0.25 v3.3.0, 0.14 v3.1.0, 0.11 v2.7.0, 0.27 v2.6.0, 0.17 v2.5.0, 0.00 v2.2.1, 0.50 v2.2.0, 0.60 v2.1.0
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

cnf(w_definition,axiom,
    ( apply(apply(w,X),Y) = apply(apply(X,Y),Y) )).

cnf(strong_fixed_point,axiom,
    ( strong_fixed_point = apply(apply(b,apply(apply(b,apply(w,w)),apply(apply(b,apply(b,w)),b))),b) )).

cnf(prove_strong_fixed_point,negated_conjecture,
    (  apply(strong_fixed_point,fixed_pt) != apply(fixed_pt,apply(strong_fixed_point,fixed_pt)) )).

%--------------------------------------------------------------------------
