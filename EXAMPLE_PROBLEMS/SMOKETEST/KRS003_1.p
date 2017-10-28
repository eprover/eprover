%--------------------------------------------------------------------------
% File     : KRS003_1 : TPTP v7.0.0. Released v5.0.0.
% Domain   : Knowledge Representation
% Problem  : Paramasivam problem T-Box 1c
% Version  : Especial.
% English  : e and f exist.

% Refs     : [PP95]  Paramasivam & Plaisted (1995), Automated Deduction Tec
% Source   : [TPTP]
% Names    : 

% Status   : Theorem
% Rating   : 0.00 v5.0.0
% Syntax   : Number of formulae    :   30 (   0 unit;  15 type)
%            Number of atoms       :   34 (   0 equality)
%            Maximal formula depth :    7 (   3 average)
%            Number of connectives :   22 (   3   ~;   4   |;   4   &)
%                                         (   0 <=>;  11  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :   14 (  12   >;   2   *;   0   +;   0  <<)
%            Number of predicates  :   25 (  17 propositional; 0-2 arity)
%            Number of functors    :    5 (   1 constant; 0-1 arity)
%            Number of variables   :   18 (   0 sgn;  18   !;   0   ?)
%                                         (  18   :;   0  !>;   0  ?*)
%            Maximal term depth    :    2 (   1 average)
% SPC      : TF0_THM_NEQ_NAR

% Comments :
%--------------------------------------------------------------------------
tff(unreal_type,type,(
    unreal: $tType )).

tff(real_type,type,(
    real: $tType )).

tff(u1r1_type,type,(
    u1r1: unreal > real )).

tff(u1r2_type,type,(
    u1r2: unreal > real )).

tff(u3r1_type,type,(
    u3r1: unreal > real )).

tff(u3r2_type,type,(
    u3r2: unreal > real )).

tff(exist_type,type,(
    exist: unreal )).

tff(f_type,type,(
    f: unreal > $o )).

tff(d_type,type,(
    d: unreal > $o )).

tff(e_type,type,(
    e: unreal > $o )).

tff(s1most_type,type,(
    s1most: unreal > $o )).

tff(s_type,type,(
    s: ( unreal * real ) > $o )).

tff(c_type,type,(
    c: unreal > $o )).

tff(equalish_type,type,(
    equalish: ( real * real ) > $o )).

tff(s2least_type,type,(
    s2least: unreal > $o )).

tff(clause_3,axiom,(
    ! [X1: unreal] :
      ( c(X1)
     => s2least(X1) ) )).

tff(clause_4,axiom,(
    ! [X1: unreal] :
      ( s2least(X1)
     => c(X1) ) )).

tff(clause_5,axiom,(
    ! [X1: unreal] :
      ~ ( s2least(X1)
        & equalish(u1r2(X1),u1r1(X1)) ) )).

tff(clause_6,axiom,(
    ! [X1: unreal] :
      ( s2least(X1)
     => s(X1,u1r1(X1)) ) )).

tff(clause_7,axiom,(
    ! [X1: unreal] :
      ( s2least(X1)
     => s(X1,u1r2(X1)) ) )).

tff(clause_8,axiom,(
    ! [X2: real,X3: real,X1: unreal] :
      ( ( s(X1,X3)
        & s(X1,X2) )
     => ( s2least(X1)
        | equalish(X3,X2) ) ) )).

tff(clause_9,axiom,(
    ! [X1: unreal] :
      ( d(X1)
     => s1most(X1) ) )).

tff(clause_10,axiom,(
    ! [X1: unreal] :
      ( s1most(X1)
     => d(X1) ) )).

tff(clause_11,axiom,(
    ! [X2: real,X3: real,X1: unreal] :
      ( ( s1most(X1)
        & s(X1,X3)
        & s(X1,X2) )
     => equalish(X3,X2) ) )).

tff(clause_12,axiom,(
    ! [X1: unreal] :
      ( equalish(u3r2(X1),u3r1(X1))
     => s1most(X1) ) )).

tff(clause_13,axiom,(
    ! [X1: unreal] :
      ( s1most(X1)
      | s(X1,u3r1(X1)) ) )).

tff(clause_14,axiom,(
    ! [X1: unreal] :
      ( s1most(X1)
      | s(X1,u3r2(X1)) ) )).

tff(clause_15,axiom,(
    ! [X1: unreal] :
      ( e(X1)
     => c(X1) ) )).

tff(clause_16,axiom,(
    ! [X1: unreal] :
      ( f(X1)
     => d(X1) ) )).

tff(clause_1__clause_2,conjecture,
    ( ~ e(exist)
    | ~ f(exist) )).
%--------------------------------------------------------------------------
