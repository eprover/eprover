%--------------------------------------------------------------------------
% File     : SYN190-1 : TPTP v6.4.0. Released v1.1.0.
% Domain   : Syntactic (Random Prolog Theory)
% Problem  : RPT63 synthetic problem 88 (quasi-uniform distribution)
% Version  : [SE94] axioms : Especial.
% English  :

% Refs     : [SE94]  Segre & Elkan (1994), A High-Performance Explanation-B
% Source   : [SE94]
% Names    : - [SE94]

% Status   : Unsatisfiable
% Rating   : 0.00 v6.2.0, 0.12 v6.1.0, 0.00 v2.2.0, 0.22 v2.1.0, 0.00 v2.0.0
% Syntax   : Number of clauses     :  369 (   0 non-Horn;  39 unit; 362 RR)
%            Number of atoms       : 1060 (   0 equality)
%            Maximal clause size   :    5 (   3 average)
%            Number of predicates  :   48 (   0 propositional; 1-3 arity)
%            Number of functors    :    5 (   5 constant; 0-0 arity)
%            Number of variables   :  627 ( 161 singleton)
%            Maximal term depth    :    1 (   1 average)
% SPC      : CNF_UNS_EPR

% Comments : This theory has a finite deductive closure.
%--------------------------------------------------------------------------
%----Include Synthetic domain theory for EBL
include('Axioms/SYN001-0.ax').
%--------------------------------------------------------------------------
cnf(prove_this,negated_conjecture,
    ( ~ r3(a,X,d) )).

%--------------------------------------------------------------------------
