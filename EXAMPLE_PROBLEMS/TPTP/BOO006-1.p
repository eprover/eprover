%--------------------------------------------------------------------------
% File     : BOO006-1 : TPTP v6.4.0. Released v1.0.0.
% Domain   : Boolean Algebra
% Problem  : Multiplication is bounded (X * 0 = 0)
% Version  : [MOW76] axioms.
% English  :

% Refs     : [Whi61] Whitesitt (1961), Boolean Algebra and Its Applications
%          : [MOW76] McCharen et al. (1976), Problems and Experiments for a
%          : [OMW76] Overbeek et al. (1976), Complexity and Related Enhance
% Source   : [MOW76]
% Names    : B3 part 2 [MOW76]
%          : B6 [MOW76]
%          : Lemma proved [OMW76]
%          : prob3_part2.ver1 [ANL]

% Status   : Unsatisfiable
% Rating   : 0.00 v6.0.0, 0.11 v5.5.0, 0.19 v5.4.0, 0.13 v5.3.0, 0.25 v5.2.0, 0.12 v5.1.0, 0.14 v4.1.0, 0.11 v4.0.1, 0.17 v3.7.0, 0.00 v2.4.0, 0.17 v2.3.0, 0.00 v2.1.0, 0.00 v2.0.0
% Syntax   : Number of clauses     :   23 (   0 non-Horn;  11 unit;  13 RR)
%            Number of atoms       :   61 (   2 equality)
%            Maximal clause size   :    5 (   3 average)
%            Number of predicates  :    3 (   0 propositional; 2-3 arity)
%            Number of functors    :    6 (   3 constant; 0-2 arity)
%            Number of variables   :   82 (   0 singleton)
%            Maximal term depth    :    2 (   1 average)
% SPC      : CNF_UNS_RFO_SEQ_HRN

% Comments :
%--------------------------------------------------------------------------
%----Include boolean algebra axioms
include('Axioms/BOO002-0.ax').
%--------------------------------------------------------------------------
cnf(prove_equations,negated_conjecture,
    ( ~ product(x,additive_identity,additive_identity) )).

%--------------------------------------------------------------------------
