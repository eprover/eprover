%------------------------------------------------------------------------------
% File     : NUM728^1 : TPTP v7.0.0. Released v3.7.0.
% Domain   : Number Theory
% Problem  : Landau theorem 40a
% Version  : Especial.
% English  : eq (fr (ts (1x x) n) (ts (2x x) n)) x

% Refs     : [Lan30] Landau (1930), Grundlagen der Analysis
%          : [vBJ79] van Benthem Jutting (1979), Checking Landau's "Grundla
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : satz40a [Lan30]

% Status   : Theorem
%          : Without extensionality : Theorem
% Rating   : 0.00 v5.3.0, 0.25 v5.2.0, 0.00 v3.7.0
% Syntax   : Number of formulae    :   12 (   0 unit;   9 type;   0 defn)
%            Number of atoms       :   28 (   0 equality;   9 variable)
%            Maximal formula depth :    9 (   4 average)
%            Number of connectives :   25 (   0   ~;   0   |;   0   &;  24   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    8 (   8   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   11 (   9   :;   0   =)
%            Number of variables   :    4 (   0 sgn;   4   !;   0   ?;   0   ^)
%                                         (   4   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(frac_type,type,(
    frac: $tType )).

thf(x,type,(
    x: frac )).

thf(nat_type,type,(
    nat: $tType )).

thf(n,type,(
    n: nat )).

thf(eq,type,(
    eq: frac > frac > $o )).

thf(fr,type,(
    fr: nat > nat > frac )).

thf(ts,type,(
    ts: nat > nat > nat )).

thf(c1x,type,(
    c1x: frac > nat )).

thf(c2x,type,(
    c2x: frac > nat )).

thf(satz38,axiom,(
    ! [Xx: frac,Xy: frac] :
      ( ( eq @ Xx @ Xy )
     => ( eq @ Xy @ Xx ) ) )).

thf(satz40,axiom,(
    ! [Xx: frac,Xn: nat] :
      ( eq @ Xx @ ( fr @ ( ts @ ( c1x @ Xx ) @ Xn ) @ ( ts @ ( c2x @ Xx ) @ Xn ) ) ) )).

thf(satz40a,conjecture,
    ( eq @ ( fr @ ( ts @ ( c1x @ x ) @ n ) @ ( ts @ ( c2x @ x ) @ n ) ) @ x )).

%------------------------------------------------------------------------------
