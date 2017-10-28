%--------------------------------------------------------------------------
% File     : BOO028-1 : TPTP v7.0.0. Released v2.2.0.
% Domain   : Boolean Algebra
% Problem  : Self-dual 2-basis from majority reduction, part 1.
% Version  : [MP96] (equality) axioms : Especial.
% English  : This is part of a proof that there exists an independent
%            self-dual-2-basis for Boolean algebra by majority reduction.

% Refs     : [McC98] McCune (1998), Email to G. Sutcliffe
%          : [MP96]  McCune & Padmanabhan (1996), Automated Deduction in Eq
% Source   : [McC98]
% Names    : DUAL-BA-5-a [MP96]

% Status   : Unsatisfiable
% Rating   : 0.06 v7.0.0, 0.11 v6.4.0, 0.16 v6.3.0, 0.18 v6.2.0, 0.21 v6.1.0, 0.38 v6.0.0, 0.48 v5.5.0, 0.47 v5.4.0, 0.33 v5.3.0, 0.25 v5.2.0, 0.29 v5.1.0, 0.27 v5.0.0, 0.21 v4.1.0, 0.18 v4.0.1, 0.21 v4.0.0, 0.15 v3.7.0, 0.11 v3.4.0, 0.12 v3.3.0, 0.14 v3.2.0, 0.07 v3.1.0, 0.00 v2.7.0, 0.09 v2.6.0, 0.00 v2.2.1
% Syntax   : Number of clauses     :   11 (   0 non-Horn;  11 unit;   1 RR)
%            Number of atoms       :   11 (  11 equality)
%            Maximal clause size   :    1 (   1 average)
%            Number of predicates  :    1 (   0 propositional; 2-2 arity)
%            Number of functors    :    6 (   3 constant; 0-2 arity)
%            Number of variables   :   26 (   8 singleton)
%            Maximal term depth    :    4 (   3 average)
% SPC      : CNF_UNS_RFO_PEQ_UEQ

% Comments :
%--------------------------------------------------------------------------
%----Properties L1, L3, and B1 of Boolean Algebra:
cnf(l1,axiom,
    ( add(X,multiply(Y,multiply(X,Z))) = X )).

cnf(l3,axiom,
    ( add(add(multiply(X,Y),multiply(Y,Z)),Y) = Y )).

cnf(b1,axiom,
    ( multiply(add(X,Y),add(X,inverse(Y))) = X )).

%----The corresponding dual properties L2, L4, and B2.
cnf(l2,axiom,
    ( multiply(X,add(Y,add(X,Z))) = X )).

cnf(l4,axiom,
    ( multiply(multiply(add(X,Y),add(Y,Z)),Y) = Y )).

cnf(b2,axiom,
    ( add(multiply(X,Y),multiply(X,inverse(Y))) = X )).

%----Associativity and Commutativity of both operations:
cnf(commutativity_of_add,axiom,
    ( add(X,Y) = add(Y,X) )).

cnf(commutativity_of_multiply,axiom,
    ( multiply(X,Y) = multiply(Y,X) )).

cnf(associativity_of_add,axiom,
    ( add(add(X,Y),Z) = add(X,add(Y,Z)) )).

cnf(associativity_of_multiply,axiom,
    ( multiply(multiply(X,Y),Z) = multiply(X,multiply(Y,Z)) )).

%----Denial of conclusion:
cnf(prove_multiply_add_property,negated_conjecture,
    (  multiply(a,add(b,c)) != add(multiply(b,a),multiply(c,a)) )).

%--------------------------------------------------------------------------
