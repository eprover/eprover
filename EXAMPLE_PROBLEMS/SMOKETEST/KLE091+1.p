%------------------------------------------------------------------------------
% File     : KLE091+1 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Kleene Algebra (Domain Semirings)
% Problem  : Codomain closure
% Version  : [Hoe08] axioms.
% English  : In a Boolean domain semiring, codomain elements are domain
%            elements.

% Refs     : [Hoe08] Hoefner (2008), Email to G. Sutcliffe
% Source   : [Hoe08]
% Names    :

% Status   : Theorem
% Rating   : 0.52 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.46 v6.2.0, 0.52 v6.1.0, 0.63 v6.0.0, 0.61 v5.5.0, 0.70 v5.4.0, 0.71 v5.3.0, 0.74 v5.2.0, 0.60 v5.1.0, 0.62 v4.1.0, 0.61 v4.0.0
% Syntax   : Number of formulae    :   21 (  20 unit)
%            Number of atoms       :   22 (  21 equality)
%            Maximal formula depth :    4 (   3 average)
%            Number of connectives :    1 (   0   ~;   0   |;   0   &)
%                                         (   1 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of predicates  :    2 (   0 propositional; 2-2 arity)
%            Number of functors    :    8 (   2 constant; 0-2 arity)
%            Number of variables   :   33 (   0 sgn;  33   !;   0   ?)
%            Maximal term depth    :    6 (   2 average)
% SPC      : FOF_THM_RFO_SEQ

% Comments : Equational encoding
%------------------------------------------------------------------------------
%---Include axioms for domain semiring (Boolean test algebra)
include('Axioms/KLE001+0.ax').
%---Include axioms for Boolean domain/codomain
include('Axioms/KLE001+4.ax').
%------------------------------------------------------------------------------
fof(goals,conjecture,(
    ! [X0] : domain(codomain(X0)) = codomain(X0) )).

%------------------------------------------------------------------------------
