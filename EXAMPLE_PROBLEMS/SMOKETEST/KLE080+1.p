%------------------------------------------------------------------------------
% File     : KLE080+1 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Kleene Algebra (Domain Semirings)
% Problem  : Another complementation property of domain and antidomain elements
% Version  : [Hoe08] axioms.
% English  :

% Refs     : [DS08]  Desharnais & Struth (2008), Modal Semirings Revisited
%          : [Hoe08] Hoefner (2008), Email to G. Sutcliffe
% Source   : [Hoe08]
% Names    :

% Status   : Theorem
% Rating   : 0.35 v7.0.0, 0.43 v6.4.0, 0.42 v6.2.0, 0.52 v6.1.0, 0.57 v5.5.0, 0.63 v5.4.0, 0.64 v5.3.0, 0.67 v5.2.0, 0.55 v5.1.0, 0.57 v5.0.0, 0.54 v4.1.0, 0.48 v4.0.1, 0.52 v4.0.0
% Syntax   : Number of formulae    :   18 (  16 unit)
%            Number of atoms       :   21 (  20 equality)
%            Maximal formula depth :    5 (   3 average)
%            Number of connectives :    3 (   0   ~;   0   |;   1   &)
%                                         (   1 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of predicates  :    2 (   0 propositional; 2-2 arity)
%            Number of functors    :    6 (   2 constant; 0-2 arity)
%            Number of variables   :   30 (   0 sgn;  30   !;   0   ?)
%            Maximal term depth    :    4 (   2 average)
% SPC      : FOF_THM_RFO_SEQ

% Comments : Equational encoding
%------------------------------------------------------------------------------
%---Include axioms for domain semiring
include('Axioms/KLE001+0.ax').
%---Include axioms for domain
include('Axioms/KLE001+5.ax').
%------------------------------------------------------------------------------
fof(goals,conjecture,(
    ! [X0] :
      ( ! [X1] :
          ( addition(domain(X1),antidomain(X1)) = one
          & multiplication(domain(X1),antidomain(X1)) = zero )
     => antidomain(antidomain(X0)) = domain(X0) ) )).

%------------------------------------------------------------------------------
