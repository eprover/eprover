%--------------------------------------------------------------------------
% File     : MGT063+1 : TPTP v6.4.0. Released v2.4.0.
% Domain   : Management (Organisation Theory)
% Problem  : Conditions for increasing then decreasing hazard of mortality
% Version  : [Han98] axioms.
% English  : If environmental drift destroys alignment before advantage can
%            be gained from occupancy of a robust position, then the hazard
%            of mortality for an unendowed organization with a robust
%            position initially increases with age, then decreases with
%            further aging and falls below the initial level.

% Refs     : [Kam00] Kamps (2000), Email to G. Sutcliffe
%            [CH00]  Carroll & Hannan (2000), The Demography of Corporation
%            [Han98] Hannan (1998), Rethinking Age Dependence in Organizati
% Source   : [Kam00]
% Names    : THEOREM 9 [Han98]

% Status   : Theorem
% Rating   : 0.33 v6.4.0, 0.31 v6.3.0, 0.33 v6.2.0, 0.32 v6.1.0, 0.47 v6.0.0, 0.52 v5.5.0, 0.59 v5.4.0, 0.61 v5.3.0, 0.63 v5.2.0, 0.45 v5.1.0, 0.48 v5.0.0, 0.46 v4.1.0, 0.48 v4.0.0, 0.46 v3.7.0, 0.45 v3.5.0, 0.42 v3.4.0, 0.37 v3.3.0, 0.43 v3.2.0, 0.55 v3.1.0, 0.67 v2.7.0, 0.50 v2.6.0, 0.83 v2.5.0, 1.00 v2.4.0
% Syntax   : Number of formulae    :   20 (   6 unit)
%            Number of atoms       :   77 (  12 equality)
%            Maximal formula depth :   17 (   5 average)
%            Number of connectives :   69 (  12 ~  ;   4  |;  29  &)
%                                         (   8 <=>;  16 =>;   0 <=)
%                                         (   0 <~>;   0 ~|;   0 ~&)
%            Number of predicates  :   12 (   0 propositional; 1-3 arity)
%            Number of functors    :   11 (   9 constant; 0-2 arity)
%            Number of variables   :   34 (   0 singleton;  34 !;   0 ?)
%            Maximal term depth    :    2 (   1 average)
% SPC      : FOF_THM_RFO_SEQ

% Comments : See MGT042+1.p for the mnemonic names.
%--------------------------------------------------------------------------
include('Axioms/MGT001+0.ax').
%--------------------------------------------------------------------------
%----Problem Axioms
%----An endowment provides an immunity that lasts until an
%----organization's age exceeds `eta'.
fof(definition_1,axiom,
    ( ! [X] :
        ( has_endowment(X)
      <=> ! [T] :
            ( organization(X)
            & ( smaller_or_equal(age(X,T),eta)
             => has_immunity(X,T) )
            & ( greater(age(X,T),eta)
             => ~ has_immunity(X,T) ) ) ) )).

%----An unendowed organization never possesses immunity.
fof(assumption_1,axiom,
    ( ! [X,T] :
        ( ( organization(X)
          & ~ has_endowment(X) )
       => ~ has_immunity(X,T) ) )).

%----Two states of the environment are dissimilar for an organization
%----if and only if the organization cannot be aligned to both.
%----
%----Added quantification over X.
fof(definition_2,axiom,
    ( ! [X,T0,T] :
        ( dissimilar(X,T0,T)
      <=> ( organization(X)
          & ~ ( is_aligned(X,T0)
            <=> is_aligned(X,T) ) ) ) )).

%----An organization is aligned with the state of the environment at
%----its time of founding.
fof(assumption_13,axiom,
    ( ! [X,T] :
        ( ( organization(X)
          & age(X,T) = zero )
       => is_aligned(X,T) ) )).

%----Environmental drift: the environments at times separated by more
%----than `sigma' are dissimilar.
fof(assumption_15,axiom,
    ( ! [X,T0,T] :
        ( ( organization(X)
          & age(X,T0) = zero )
       => ( greater(age(X,T),sigma)
        <=> dissimilar(X,T0,T) ) ) )).

%----An organization's position is robust if and only if it provides
%----positional advantage only after age `tau'.
%----
%----Text says fragile_position(X) instead of robust_position(X).
%----Interchanged ~ positional_advantage(X,T) and positional_advantage(X,T).
fof(definition_4,axiom,
    ( ! [X] :
        ( robust_position(X)
      <=> ! [T] :
            ( ( smaller_or_equal(age(X,T),tau)
             => ~ positional_advantage(X,T) )
            & ( greater(age(X,T),tau)
             => positional_advantage(X,T) ) ) ) )).

%----An organization's immunity. alignment of capability with the
%----current state of the environment and positional advantage jointly
%----affect the hazard of mortality with the following ordinal scaling:
fof(assumption_17,axiom,
    ( ! [X,T] :
        ( organization(X)
       => ( ( has_immunity(X,T)
           => hazard_of_mortality(X,T) = very_low )
          & ( ~ has_immunity(X,T)
           => ( ( ( is_aligned(X,T)
                  & positional_advantage(X,T) )
               => hazard_of_mortality(X,T) = low )
              & ( ( ~ is_aligned(X,T)
                  & positional_advantage(X,T) )
               => hazard_of_mortality(X,T) = mod1 )
              & ( ( is_aligned(X,T)
                  & ~ positional_advantage(X,T) )
               => hazard_of_mortality(X,T) = mod2 )
              & ( ( ~ is_aligned(X,T)
                  & ~ positional_advantage(X,T) )
               => hazard_of_mortality(X,T) = high ) ) ) ) ) )).

%----The levels of hazard of mortality are ordered:
%----
%----Split over 5 separate formulas because TPTP gives an error on top
%----level occurrences of `&'.
fof(assumption_18a,axiom,
    ( greater(high,mod1) )).

fof(assumption_18b,axiom,
    ( greater(mod1,low) )).

fof(assumption_18c,axiom,
    ( greater(low,very_low) )).

fof(assumption_18d,axiom,
    ( greater(high,mod2) )).

fof(assumption_18e,axiom,
    ( greater(mod2,low) )).

%----Position dominates alignment:
fof(assumption_19,axiom,
    ( greater(mod2,mod1) )).

%----Problem theorems
%----Robust position without endowment when (`sigma' < `tau'): If
%----environmental drift destroys alignment before advantage can
%----be gained from occupancy of a robust position (`sigma' < `tau'), then
%----the hazard of mortality for an unendowed organization with a
%----robust position initially increases with age, then decreases with
%----further aging and falls below the initial level.
%----From D2, D4, A1, A13, A15, A17, A18 (text says D1,2,4 and A1,2,13-15,
%----17-19; also needs D<, D<=, MP>str, MP>com, MP>tra).
%----
%----Added (`sigma' < `tau') in antecedent
%----and (hazard_of_mortality(X,T1) = hazard_of_mortality(X,T0)).
fof(theorem_9,conjecture,
    ( ! [X,T0,T1,T2,T3] :
        ( ( organization(X)
          & robust_position(X)
          & ~ has_endowment(X)
          & age(X,T0) = zero
          & greater(sigma,zero)
          & greater(tau,zero)
          & smaller(sigma,tau)
          & smaller_or_equal(age(X,T1),sigma)
          & greater(age(X,T2),sigma)
          & smaller_or_equal(age(X,T2),tau)
          & greater(age(X,T3),tau) )
       => ( smaller(hazard_of_mortality(X,T3),hazard_of_mortality(X,T1))
          & smaller(hazard_of_mortality(X,T1),hazard_of_mortality(X,T2))
          & hazard_of_mortality(X,T1) = hazard_of_mortality(X,T0) ) ) )).

%--------------------------------------------------------------------------
