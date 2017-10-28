%------------------------------------------------------------------------------
% File     : KLE143+1 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Kleene Algebra (Demonic Refinement Algebra)
% Problem  : Strong iteration is idempotent w.r.t. multiplication
% Version  : [Hoe08] axioms.
% English  :

% Refs     : [vW02]  von Wright (2002), From Kleene Algebra to Refinement A
%          : [Hoe08] Hoefner (2008), Email to G. Sutcliffe
% Source   : [Hoe08]
% Names    :

% Status   : Theorem
% Rating   : 0.39 v7.0.0, 0.50 v6.4.0, 0.46 v6.2.0, 0.56 v6.1.0, 0.73 v6.0.0, 0.65 v5.5.0, 0.67 v5.4.0, 0.71 v5.3.0, 0.74 v5.2.0, 0.60 v5.1.0, 0.62 v4.1.0, 0.61 v4.0.1, 0.70 v4.0.0
% Syntax   : Number of formulae    :   19 (  15 unit)
%            Number of atoms       :   23 (  16 equality)
%            Maximal formula depth :    5 (   3 average)
%            Number of connectives :    4 (   0   ~;   0   |;   0   &)
%                                         (   1 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of predicates  :    2 (   0 propositional; 2-2 arity)
%            Number of functors    :    6 (   2 constant; 0-2 arity)
%            Number of variables   :   35 (   0 sgn;  35   !;   0   ?)
%            Maximal term depth    :    4 (   2 average)
% SPC      : FOF_THM_RFO_SEQ

% Comments :
%------------------------------------------------------------------------------
%---Include axioms for demonic refinement algebra
include('Axioms/KLE004+0.ax').
%------------------------------------------------------------------------------
fof(goals,conjecture,(
    ! [X0] : multiplication(strong_iteration(X0),strong_iteration(X0)) = strong_iteration(X0) )).

%------------------------------------------------------------------------------
