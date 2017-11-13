%------------------------------------------------------------------------------
% File     : NUM737^1 : TPTP v7.0.0. Released v3.7.0.
% Domain   : Number Theory
% Problem  : Landau theorem 44
% Version  : Especial.
% English  : more (ts (num z) (den u)) (ts (num u) (den z))

% Refs     : [Lan30] Landau (1930), Grundlagen der Analysis
%          : [vBJ79] van Benthem Jutting (1979), Checking Landau's "Grundla
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : satz44 [Lan30]

% Status   : Theorem
%          : Without extensionality : Theorem
% Rating   : 0.62 v7.0.0, 0.57 v6.4.0, 0.67 v6.3.0, 0.60 v6.2.0, 0.43 v6.1.0, 0.57 v6.0.0, 0.86 v5.5.0, 0.83 v5.4.0, 0.80 v4.1.0, 1.00 v3.7.0
% Syntax   : Number of formulae    :   19 (   0 unit;  10 type;   0 defn)
%            Number of atoms       :  104 (   6 equality;  30 variable)
%            Maximal formula depth :    9 (   5 average)
%            Number of connectives :   83 (   0   ~;   0   |;   0   &;  80   @)
%                                         (   0 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    6 (   6   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   12 (  10   :;   0   =)
%            Number of variables   :   13 (   0 sgn;  13   !;   0   ?;   0   ^)
%                                         (  13   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(frac_type,type,(
    frac: $tType )).

thf(x,type,(
    x: frac )).

thf(y,type,(
    y: frac )).

thf(z,type,(
    z: frac )).

thf(u,type,(
    u: frac )).

thf(nat_type,type,(
    nat: $tType )).

thf(more,type,(
    more: nat > nat > $o )).

thf(ts,type,(
    ts: nat > nat > nat )).

thf(num,type,(
    num: frac > nat )).

thf(den,type,(
    den: frac > nat )).

thf(m,axiom,
    ( more @ ( ts @ ( num @ x ) @ ( den @ y ) ) @ ( ts @ ( num @ y ) @ ( den @ x ) ) )).

thf(e,axiom,
    (  ts @ ( num @ x ) @ ( den @ z ) 
    =  ts @ ( num @ z ) @ ( den @ x ) ) ).

thf(f,axiom,
    (  ts @ ( num @ y ) @ ( den @ u ) 
    =  ts @ ( num @ u ) @ ( den @ y ) ) ).

thf(satz33a,axiom,(
    ! [Xx: nat,Xy: nat,Xz: nat] :
      ( ( more @ ( ts @ Xx @ Xz ) @ ( ts @ Xy @ Xz ) )
     => ( more @ Xx @ Xy ) ) )).

thf(satz32d,axiom,(
    ! [Xx: nat,Xy: nat,Xz: nat] :
      ( ( more @ Xx @ Xy )
     => ( more @ ( ts @ Xz @ Xx ) @ ( ts @ Xz @ Xy ) ) ) )).

thf(satz29,axiom,(
    ! [Xx: nat,Xy: nat] :
      (  ts @ Xx @ Xy 
      =  ts @ Xy @ Xx ) ) ).

thf(satz31,axiom,(
    ! [Xx: nat,Xy: nat,Xz: nat] :
      (  ts @ ( ts @ Xx @ Xy ) @ Xz 
      =  ts @ Xx @ ( ts @ Xy @ Xz ) ) ) ).

thf(satz38,axiom,(
    ! [Xx: frac,Xy: frac] :
      ( (  ts @ ( num @ Xx ) @ ( den @ Xy ) 
        =  ts @ ( num @ Xy ) @ ( den @ Xx ) ) 
     => (  ts @ ( num @ Xy ) @ ( den @ Xx ) 
        =  ts @ ( num @ Xx ) @ ( den @ Xy ) ) ) ) ).

thf(satz44,conjecture,
    ( more @ ( ts @ ( num @ z ) @ ( den @ u ) ) @ ( ts @ ( num @ u ) @ ( den @ z ) ) )).

%------------------------------------------------------------------------------
