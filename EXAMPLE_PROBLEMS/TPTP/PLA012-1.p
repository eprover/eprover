%--------------------------------------------------------------------------
% File     : PLA012-1 : TPTP v6.4.0. Released v1.1.0.
% Domain   : Planning (Blocks world)
% Problem  : Block D on B on C
% Version  : [SE94] axioms.
% English  :

% Refs     : [Sus73] Sussman (1973), A Computational Model of Skill Acquisi
%          : [SE94]  Segre & Elkan (1994), A High-Performance Explanation-B
% Source   : [SE94]
% Names    : - [SE94]

% Status   : Unsatisfiable
% Rating   : 0.25 v6.2.0, 0.33 v6.1.0, 0.64 v6.0.0, 0.56 v5.5.0, 0.62 v5.4.0, 0.67 v5.3.0, 0.70 v5.2.0, 0.77 v5.1.0, 0.69 v5.0.0, 0.60 v4.1.0, 0.67 v4.0.1, 0.71 v3.7.0, 0.57 v3.4.0, 0.40 v3.3.0, 0.33 v2.7.0, 0.50 v2.6.0, 0.14 v2.5.0, 0.43 v2.4.0, 0.43 v2.3.0, 0.29 v2.2.1, 0.56 v2.2.0, 0.67 v2.1.0, 0.71 v2.0.0
% Syntax   : Number of clauses     :   31 (   0 non-Horn;  20 unit;  28 RR)
%            Number of atoms       :   53 (   0 equality)
%            Maximal clause size   :    4 (   2 average)
%            Number of predicates  :    2 (   0 propositional; 2-2 arity)
%            Number of functors    :   14 (   7 constant; 0-2 arity)
%            Number of variables   :   37 (   5 singleton)
%            Maximal term depth    :    3 (   1 average)
% SPC      : CNF_UNS_RFO_NEQ_HRN

% Comments : The axioms are a reconstruction of the situation calculus
%            blocks world as in [Sus73].
%--------------------------------------------------------------------------
%----Include Blocks world axioms
include('Axioms/PLA001-0.ax').
%----Include Blocks world difference axioms for 4 blocks
include('Axioms/PLA001-1.ax').
%--------------------------------------------------------------------------
cnf(prove_DBC,negated_conjecture,
    ( ~ holds(and(on(d,b),on(b,c)),State) )).

%--------------------------------------------------------------------------
