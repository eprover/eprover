%------------------------------------------------------------------------------
% File     : NUM736^1 : TPTP v7.0.0. Released v3.7.0.
% Domain   : Number Theory
% Problem  : Landau theorem 42
% Version  : Especial.
% English  : less (ts (num y) (den x)) (ts (num x) (den y))

% Refs     : [Lan30] Landau (1930), Grundlagen der Analysis
%          : [vBJ79] van Benthem Jutting (1979), Checking Landau's "Grundla
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : satz42 [Lan30]

% Status   : Theorem
%          : Without extensionality : Theorem
% Rating   : 0.00 v5.3.0, 0.25 v5.2.0, 0.00 v3.7.0
% Syntax   : Number of formulae    :   12 (   0 unit;   9 type;   0 defn)
%            Number of atoms       :   28 (   0 equality;   4 variable)
%            Maximal formula depth :    6 (   4 average)
%            Number of connectives :   25 (   0   ~;   0   |;   0   &;  24   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    8 (   8   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   11 (   9   :;   0   =)
%            Number of variables   :    2 (   0 sgn;   2   !;   0   ?;   0   ^)
%                                         (   2   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(frac_type,type,(
    frac: $tType )).

thf(x,type,(
    x: frac )).

thf(y,type,(
    y: frac )).

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

thf(less,type,(
    less: nat > nat > $o )).

thf(satz11,axiom,(
    ! [Xx: nat,Xy: nat] :
      ( ( more @ Xx @ Xy )
     => ( less @ Xy @ Xx ) ) )).

thf(satz42,conjecture,
    ( less @ ( ts @ ( num @ y ) @ ( den @ x ) ) @ ( ts @ ( num @ x ) @ ( den @ y ) ) )).

%------------------------------------------------------------------------------
