%--------------------------------------------------------------------------
% File     : HEN011-2 : TPTP v6.4.0. Released v1.0.0.
% Domain   : Henkin Models
% Problem  : This operation is commutative
% Version  : [MOW76] axioms : Augmented.
% English  : Define & on the set of Z', where Z' = identity/Z,
%            by X' & Y' = X'/(identity/Y'). The operation is commutative.

% Refs     : [MOW76] McCharen et al. (1976), Problems and Experiments for a
% Source   : [MOW76]
% Names    : H11 [MOW76]
%          : hp11.ver1.in [ANL]

% Status   : Unsatisfiable
% Rating   : 0.29 v6.3.0, 0.17 v6.2.0, 0.00 v6.1.0, 0.20 v6.0.0, 0.44 v5.5.0, 0.69 v5.4.0, 0.73 v5.3.0, 0.58 v5.2.0, 0.50 v5.1.0, 0.43 v5.0.0, 0.14 v4.1.0, 0.22 v4.0.1, 0.17 v3.5.0, 0.00 v3.3.0, 0.14 v3.1.0, 0.11 v2.7.0, 0.00 v2.6.0, 0.14 v2.5.0, 0.20 v2.4.0, 0.33 v2.2.1, 0.78 v2.2.0, 0.71 v2.1.0, 0.80 v2.0.0
% Syntax   : Number of clauses     :   26 (   0 non-Horn;  14 unit;  19 RR)
%            Number of atoms       :   55 (   5 equality)
%            Maximal clause size   :    6 (   2 average)
%            Number of predicates  :    3 (   0 propositional; 2-3 arity)
%            Number of functors    :   11 (  10 constant; 0-2 arity)
%            Number of variables   :   55 (   7 singleton)
%            Maximal term depth    :    2 (   1 average)
% SPC      : CNF_UNS_RFO_SEQ_HRN

% Comments :
%--------------------------------------------------------------------------
%----Include Henkin model axioms
include('Axioms/HEN001-0.ax').
%--------------------------------------------------------------------------
%----McCharen uses these earlier results too. I don't
cnf(everything_divide_identity_is_zero,axiom,
    ( quotient(X,identity,zero) )).

cnf(zero_divide_anything_is_zero,axiom,
    ( quotient(zero,X,zero) )).

cnf(x_divide_x_is_zero,axiom,
    ( quotient(X,X,zero) )).

cnf(x_divde_zero_is_x,axiom,
    ( quotient(X,zero,X) )).

cnf(transitivity_of_less_equal,axiom,
    ( ~ less_equal(X,Y)
    | ~ less_equal(Y,Z)
    | less_equal(X,Z) )).

cnf(xQyLEz_implies_xQzLEy,axiom,
    ( ~ quotient(X,Y,W1)
    | ~ less_equal(W1,Z)
    | ~ quotient(X,Z,W2)
    | less_equal(W2,Y) )).

cnf(xLEy_implies_zQyLEzQx,axiom,
    ( ~ less_equal(X,Y)
    | ~ quotient(Z,Y,W1)
    | ~ quotient(Z,X,W2)
    | less_equal(W1,W2) )).

cnf(xLEy_implies_xQzLEyQz,axiom,
    ( ~ less_equal(X,Y)
    | ~ quotient(X,Z,W1)
    | ~ quotient(Y,Z,W2)
    | less_equal(W1,W2) )).

cnf(one_inversion_equals_three,axiom,
    ( ~ quotient(identity,X,Y1)
    | ~ quotient(identity,Y1,Y2)
    | ~ quotient(identity,Y2,Y3)
    | Y1 = Y3 )).

cnf(inversion_lemma,axiom,
    ( ~ quotient(identity,X,Y1)
    | ~ quotient(identity,Y1,Y2)
    | ~ quotient(Y1,Y2,Y3)
    | Y1 = Y3 )).

cnf(identity_divide_a,hypothesis,
    ( quotient(identity,a,idQa) )).

cnf(identity_divide_b,hypothesis,
    ( quotient(identity,b,idQb) )).

cnf(identity_divide_idQb,hypothesis,
    ( quotient(identity,idQb,idQ_idQb) )).

cnf(idQa_divide_idQ_idQb,hypothesis,
    ( quotient(idQa,idQ_idQb,idQa_Q__idQ_idQb) )).

cnf(identity_divide_idQa,hypothesis,
    ( quotient(identity,idQa,idQ_idQa) )).

cnf(idQb_divide_idQ_idQa,hypothesis,
    ( quotient(idQb,idQ_idQa,idQb_Q__idQ_idQa) )).

cnf(prove_idQa_Q__idQ_idQb_equals_idQb_Q__idQ_idQa,negated_conjecture,
    (  idQa_Q__idQ_idQb != idQb_Q__idQ_idQa )).

%--------------------------------------------------------------------------
