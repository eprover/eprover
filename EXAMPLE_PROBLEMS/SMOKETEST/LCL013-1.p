%--------------------------------------------------------------------------
% File     : LCL013-1 : TPTP v7.0.0. Released v1.0.0.
% Domain   : Logic Calculi (Equivalential)
% Problem  : UM depends on XGF
% Version  : [McC92] axioms.
% English  : Show that the single Meredith axiom UM can be derived from
%            the single Meredith axiom XGF.

% Refs     : [MW92]  McCune & Wos (1992), Experiments in Automated Deductio
%          : [McC92] McCune (1992), Email to G. Sutcliffe
%          : [Wos95] Wos (1995), Searching for Circles of Pure Proofs
% Source   : [McC92]
% Names    : EC-76 [MW92]

% Status   : Unsatisfiable
% Rating   : 0.00 v5.4.0, 0.06 v5.3.0, 0.10 v5.2.0, 0.00 v2.1.0, 0.00 v2.0.0
% Syntax   : Number of clauses     :    3 (   0 non-Horn;   2 unit;   2 RR)
%            Number of atoms       :    5 (   0 equality)
%            Maximal clause size   :    3 (   2 average)
%            Number of predicates  :    1 (   0 propositional; 1-1 arity)
%            Number of functors    :    4 (   3 constant; 0-2 arity)
%            Number of variables   :    5 (   0 singleton)
%            Maximal term depth    :    5 (   3 average)
% SPC      : CNF_UNS_RFO_NEQ_HRN

% Comments :
%--------------------------------------------------------------------------
cnf(condensed_detachment,axiom,
    ( ~ is_a_theorem(equivalent(X,Y))
    | ~ is_a_theorem(X)
    | is_a_theorem(Y) )).

%----Axiom by Meredith
cnf(xgf,axiom,
    ( is_a_theorem(equivalent(X,equivalent(equivalent(Y,equivalent(X,Z)),equivalent(Z,Y)))) )).

%----Axiom by Meredith
cnf(prove_um,negated_conjecture,
    ( ~ is_a_theorem(equivalent(equivalent(equivalent(a,b),c),equivalent(b,equivalent(c,a)))) )).

%--------------------------------------------------------------------------
