%--------------------------------------------------------------------------
% File     : BOO010-2 : TPTP v6.4.0. Released v1.0.0.
% Domain   : Boolean Algebra
% Problem  : Addition absorbtion (X + (X * Y) = X)
% Version  : [ANL] (equality) axioms.
% English  :

% Refs     :
% Source   : [ANL]
% Names    : prob4_part2.ver2.in [ANL]

% Status   : Unsatisfiable
% Rating   : 0.05 v6.3.0, 0.12 v6.2.0, 0.14 v6.1.0, 0.06 v6.0.0, 0.19 v5.5.0, 0.16 v5.4.0, 0.00 v5.1.0, 0.07 v5.0.0, 0.00 v2.2.1, 0.11 v2.2.0, 0.14 v2.1.0, 0.38 v2.0.0
% Syntax   : Number of clauses     :   15 (   0 non-Horn;  15 unit;   1 RR)
%            Number of atoms       :   15 (  15 equality)
%            Maximal clause size   :    1 (   1 average)
%            Number of predicates  :    1 (   0 propositional; 2-2 arity)
%            Number of functors    :    7 (   4 constant; 0-2 arity)
%            Number of variables   :   24 (   0 singleton)
%            Maximal term depth    :    3 (   2 average)
% SPC      : CNF_UNS_RFO_PEQ_UEQ

% Comments :
%--------------------------------------------------------------------------
%----Include boolean algebra axioms for equality formulation
include('Axioms/BOO003-0.ax').
%--------------------------------------------------------------------------
cnf(prove_a_plus_ab_is_a,negated_conjecture,
    (  add(a,multiply(a,b)) != a )).

%--------------------------------------------------------------------------
