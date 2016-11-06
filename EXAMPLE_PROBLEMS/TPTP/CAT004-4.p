%--------------------------------------------------------------------------
% File     : CAT004-4 : TPTP v6.4.0. Released v1.0.0.
% Domain   : Category Theory
% Problem  : X and Y epimorphisms, XY well-defined => XY epimorphism
% Version  : [Sco79] axioms : Reduced > Complete.
% English  : If x and y are epimorphisms and xy is well-defined, then
%            xy is an epimorphism.

% Refs     : [Sco79] Scott (1979), Identity and Existence in Intuitionist L
% Source   : [TPTP]
% Names    :

% Status   : Unsatisfiable
% Rating   : 0.00 v6.1.0, 0.20 v6.0.0, 0.22 v5.5.0, 0.31 v5.4.0, 0.27 v5.3.0, 0.33 v5.2.0, 0.25 v5.1.0, 0.14 v5.0.0, 0.29 v4.1.0, 0.11 v4.0.1, 0.17 v3.3.0, 0.14 v3.1.0, 0.22 v2.7.0, 0.17 v2.6.0, 0.14 v2.5.0, 0.20 v2.4.0, 0.17 v2.2.1, 0.33 v2.2.0, 0.14 v2.1.0, 0.40 v2.0.0
% Syntax   : Number of clauses     :   17 (   0 non-Horn;   7 unit;  14 RR)
%            Number of atoms       :   31 (  15 equality)
%            Maximal clause size   :    3 (   2 average)
%            Number of predicates  :    3 (   0 propositional; 1-2 arity)
%            Number of functors    :    7 (   4 constant; 0-2 arity)
%            Number of variables   :   25 (   2 singleton)
%            Maximal term depth    :    3 (   2 average)
% SPC      : CNF_UNS_RFO_SEQ_HRN

% Comments : The dependent axioms have been removed.
%--------------------------------------------------------------------------
%----Include Scott's axioms for category theory
include('Axioms/CAT004-0.ax').
%--------------------------------------------------------------------------
cnf(assume_ab_exists,hypothesis,
    ( there_exists(compose(a,b)) )).

cnf(cancellation_for_product1,hypothesis,
    ( compose(X,a) != Y
    | compose(Z,a) != Y
    | X = Z )).

cnf(cancellation_for_product2,hypothesis,
    ( compose(X,b) != Y
    | compose(Z,b) != Y
    | X = Z )).

cnf(assume_h_exists,hypothesis,
    ( there_exists(h) )).

cnf(h_ab_equals_g_ab,hypothesis,
    ( compose(h,compose(a,b)) = compose(g,compose(a,b)) )).

cnf(prove_h_equals_g,negated_conjecture,
    (  h != g )).

%--------------------------------------------------------------------------
