%--------------------------------------------------------------------------
% File     : PUZ028-6 : TPTP v6.4.0. Released v2.0.0.
% Domain   : Puzzles
% Problem  : People at a party
% Version  : [SETHEO] axioms : Especial.
% English  : We can always choose 3 persons who are either familiar with
%            each other or not familiar with each other, from 6 persons
%            who meet at a party.

% Refs     :
% Source   : [TPTP]
% Names    :

% Status   : Unsatisfiable
% Rating   : 0.00 v3.1.0, 0.11 v2.7.0, 0.17 v2.6.0, 0.11 v2.5.0, 0.00 v2.4.0, 0.50 v2.3.0, 0.00 v2.2.0, 0.67 v2.1.0
% Syntax   : Number of clauses     :   41 (   1 non-Horn;  36 unit;  41 RR)
%            Number of atoms       :   51 (   0 equality)
%            Maximal clause size   :    5 (   1 average)
%            Number of predicates  :    4 (   0 propositional; 1-2 arity)
%            Number of functors    :    6 (   6 constant; 0-0 arity)
%            Number of variables   :   12 (   0 singleton)
%            Maximal term depth    :    1 (   1 average)
% SPC      : CNF_UNS_EPR

% Comments : This version is unsatisfiable because familiarity is symmetric.
%--------------------------------------------------------------------------
cnf(person_1,axiom,
    ( person(n1) )).

cnf(person_2,axiom,
    ( person(n2) )).

cnf(person_3,axiom,
    ( person(n3) )).

cnf(person_4,axiom,
    ( person(n4) )).

cnf(person_5,axiom,
    ( person(n5) )).

cnf(person_6,axiom,
    ( person(n6) )).

cnf(not_equal_1_2,axiom,
    ( not_equal(n1,n2) )).

cnf(not_equal_1_3,axiom,
    ( not_equal(n1,n3) )).

cnf(not_equal_1_4,axiom,
    ( not_equal(n1,n4) )).

cnf(not_equal_1_5,axiom,
    ( not_equal(n1,n5) )).

cnf(not_equal_1_6,axiom,
    ( not_equal(n1,n6) )).

cnf(not_equal_2_1,axiom,
    ( not_equal(n2,n1) )).

cnf(not_equal_2_3,axiom,
    ( not_equal(n2,n3) )).

cnf(not_equal_2_4,axiom,
    ( not_equal(n2,n4) )).

cnf(not_equal_2_5,axiom,
    ( not_equal(n2,n5) )).

cnf(not_equal_2_6,axiom,
    ( not_equal(n2,n6) )).

cnf(not_equal_3_1,axiom,
    ( not_equal(n3,n1) )).

cnf(not_equal_3_2,axiom,
    ( not_equal(n3,n2) )).

cnf(not_equal_3_4,axiom,
    ( not_equal(n3,n4) )).

cnf(not_equal_3_5,axiom,
    ( not_equal(n3,n5) )).

cnf(not_equal_3_6,axiom,
    ( not_equal(n3,n6) )).

cnf(not_equal_4_1,axiom,
    ( not_equal(n4,n1) )).

cnf(not_equal_4_2,axiom,
    ( not_equal(n4,n2) )).

cnf(not_equal_4_3,axiom,
    ( not_equal(n4,n3) )).

cnf(not_equal_4_5,axiom,
    ( not_equal(n4,n5) )).

cnf(not_equal_4_6,axiom,
    ( not_equal(n4,n6) )).

cnf(not_equal_5_1,axiom,
    ( not_equal(n5,n1) )).

cnf(not_equal_5_2,axiom,
    ( not_equal(n5,n2) )).

cnf(not_equal_5_3,axiom,
    ( not_equal(n5,n3) )).

cnf(not_equal_5_4,axiom,
    ( not_equal(n5,n4) )).

cnf(not_equal_5_6,axiom,
    ( not_equal(n5,n6) )).

cnf(not_equal_6_1,axiom,
    ( not_equal(n6,n1) )).

cnf(not_equal_6_2,axiom,
    ( not_equal(n6,n2) )).

cnf(not_equal_6_3,axiom,
    ( not_equal(n6,n3) )).

cnf(not_equal_6_4,axiom,
    ( not_equal(n6,n4) )).

cnf(not_equal_6_5,axiom,
    ( not_equal(n6,n5) )).

cnf(familiar_or_not,axiom,
    ( familiar(X,Y)
    | not_familiar(X,Y)
    | ~ person(X)
    | ~ person(Y)
    | ~ not_equal(X,Y) )).

cnf(symmetry_of_familiar,axiom,
    ( ~ familiar(X1,X2)
    | familiar(X2,X1) )).

cnf(symmetry_of_not_familiar,axiom,
    ( ~ not_familiar(X1,X2)
    | not_familiar(X2,X1) )).

cnf(three_familiar,negated_conjecture,
    ( ~ familiar(X1,X2)
    | ~ familiar(X2,X3)
    | ~ familiar(X3,X1) )).

cnf(three_not_familiar,negated_conjecture,
    ( ~ not_familiar(X1,X2)
    | ~ not_familiar(X2,X3)
    | ~ not_familiar(X3,X1) )).

%--------------------------------------------------------------------------
