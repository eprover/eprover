%--------------------------------------------------------------------------
% File     : COL037-1 : TPTP v7.0.0. Released v1.0.0.
% Domain   : Combinatory Logic
% Problem  : Strong fixed point for B, S, and C
% Version  : [WM88] (equality) axioms.
% English  : The strong fixed point property holds for the set
%            P consisting of the combinators B, S, and C, where ((Sx)y)z
%            = (xz)(yz), ((Bx)y)z = x(yz), ((Cx)y)z = (xz)y.

% Refs     : [Smu85] Smullyan (1978), To Mock a Mocking Bird and Other Logi
%          : [MW87]  McCune & Wos (1987), A Case Study in Automated Theorem
%          : [WM88]  Wos & McCune (1988), Challenge Problems Focusing on Eq
%          : [MW88]  McCune & Wos (1988), Some Fixed Point Problems in Comb
% Source   : [MW88]
% Names    : - [MW88]

% Status   : Unsatisfiable
% Rating   : 0.28 v7.0.0, 0.26 v6.4.0, 0.37 v6.3.0, 0.35 v6.2.0, 0.36 v6.1.0, 0.38 v6.0.0, 0.57 v5.5.0, 0.47 v5.4.0, 0.40 v5.3.0, 0.42 v5.2.0, 0.43 v5.1.0, 0.40 v5.0.0, 0.36 v4.1.0, 0.27 v4.0.1, 0.36 v4.0.0, 0.46 v3.7.0, 0.22 v3.4.0, 0.25 v3.3.0, 0.29 v3.1.0, 0.44 v2.7.0, 0.36 v2.6.0, 0.33 v2.5.0, 0.25 v2.4.0, 0.33 v2.3.0, 0.00 v2.2.1, 0.22 v2.2.0, 0.14 v2.1.0, 0.88 v2.0.0
% Syntax   : Number of clauses     :    4 (   0 non-Horn;   4 unit;   1 RR)
%            Number of atoms       :    4 (   4 equality)
%            Maximal clause size   :    1 (   1 average)
%            Number of predicates  :    1 (   0 propositional; 2-2 arity)
%            Number of functors    :    5 (   3 constant; 0-2 arity)
%            Number of variables   :   10 (   0 singleton)
%            Maximal term depth    :    4 (   4 average)
% SPC      : CNF_UNS_RFO_PEQ_UEQ

% Comments :
%--------------------------------------------------------------------------
cnf(s_definition,axiom,
    ( apply(apply(apply(s,X),Y),Z) = apply(apply(X,Z),apply(Y,Z)) )).

cnf(b_definition,axiom,
    ( apply(apply(apply(b,X),Y),Z) = apply(X,apply(Y,Z)) )).

cnf(c_definition,axiom,
    ( apply(apply(apply(c,X),Y),Z) = apply(apply(X,Z),Y) )).

cnf(prove_fixed_point,negated_conjecture,
    (  apply(Y,f(Y)) != apply(f(Y),apply(Y,f(Y))) )).

%--------------------------------------------------------------------------
