%--------------------------------------------------------------------------
% File     : BOO072-1 : TPTP v7.0.0. Released v2.6.0.
% Domain   : Boolean Algebra
% Problem  : DN-1 is a single axiom for Boolean algebra, part 1
% Version  : [EF+02] axioms.
% English  :

% Refs     : [EF+02] Ernst et al. (2002), More First-order Test Problems in
%          : [MV+02] McCune et al. (2002), Short Single Axioms for Boolean
% Source   : [TPTP]
% Names    :

% Status   : Unsatisfiable
% Rating   : 0.17 v7.0.0, 0.21 v6.4.0, 0.26 v6.3.0, 0.24 v6.2.0, 0.21 v6.1.0, 0.25 v6.0.0, 0.33 v5.5.0, 0.37 v5.4.0, 0.20 v5.3.0, 0.08 v5.2.0, 0.14 v5.1.0, 0.20 v5.0.0, 0.21 v4.1.0, 0.27 v4.0.1, 0.21 v4.0.0, 0.23 v3.7.0, 0.22 v3.4.0, 0.25 v3.3.0, 0.07 v3.1.0, 0.00 v2.7.0, 0.09 v2.6.0
% Syntax   : Number of clauses     :    2 (   0 non-Horn;   2 unit;   1 RR)
%            Number of atoms       :    2 (   2 equality)
%            Maximal clause size   :    1 (   1 average)
%            Number of predicates  :    1 (   0 propositional; 2-2 arity)
%            Number of functors    :    4 (   2 constant; 0-2 arity)
%            Number of variables   :    4 (   2 singleton)
%            Maximal term depth    :    9 (   4 average)
% SPC      : CNF_UNS_RFO_PEQ_UEQ

% Comments : A UEQ part of BOO038-1
%--------------------------------------------------------------------------
cnf(dn1,axiom,
    ( inverse(add(inverse(add(inverse(add(A,B)),C)),inverse(add(A,inverse(add(inverse(C),inverse(add(C,D)))))))) = C )).

cnf(huntinton_1,negated_conjecture,
    (  add(b,a) != add(a,b) )).

%--------------------------------------------------------------------------
