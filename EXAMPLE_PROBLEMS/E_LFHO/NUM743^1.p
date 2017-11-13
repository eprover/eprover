%------------------------------------------------------------------------------
% File     : NUM743^1 : TPTP v7.0.0. Released v3.7.0.
% Domain   : Number Theory
% Problem  : Landau theorem 51b
% Version  : Especial.
% English  : lessf x z

% Refs     : [Lan30] Landau (1930), Grundlagen der Analysis
%          : [vBJ79] van Benthem Jutting (1979), Checking Landau's "Grundla
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : satz51b [Lan30]

% Status   : Theorem
%          : Without extensionality : Theorem
% Rating   : 0.14 v7.0.0, 0.12 v6.4.0, 0.14 v6.3.0, 0.17 v6.2.0, 0.00 v6.1.0, 0.17 v6.0.0, 0.00 v5.3.0, 0.25 v5.2.0, 0.00 v5.1.0, 0.25 v5.0.0, 0.00 v4.0.1, 0.33 v4.0.0, 0.00 v3.7.0
% Syntax   : Number of formulae    :   13 (   0 unit;   6 type;   0 defn)
%            Number of atoms       :   38 (   0 equality;  18 variable)
%            Maximal formula depth :   10 (   4 average)
%            Number of connectives :   34 (   3   ~;   0   |;   0   &;  24   @)
%                                         (   0 <=>;   7  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    8 (   6   :;   0   =)
%            Number of variables   :    9 (   0 sgn;   9   !;   0   ?;   0   ^)
%                                         (   9   :;   0  !>;   0  ?*)
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

thf(z,type,(
    z: frac )).

thf(lessf,type,(
    lessf: frac > frac > $o )).

thf(l,axiom,
    ( lessf @ x @ y )).

thf(eq,type,(
    eq: frac > frac > $o )).

thf(k,axiom,
    ( ~ ( lessf @ y @ z )
   => ( eq @ y @ z ) )).

thf(et,axiom,(
    ! [Xa: $o] :
      ( ~ ( ~ ( Xa ) )
     => Xa ) )).

thf(satz45,axiom,(
    ! [Xx: frac,Xy: frac,Xz: frac,Xu: frac] :
      ( ( lessf @ Xx @ Xy )
     => ( ( eq @ Xx @ Xz )
       => ( ( eq @ Xy @ Xu )
         => ( lessf @ Xz @ Xu ) ) ) ) )).

thf(satz37,axiom,(
    ! [Xx: frac] :
      ( eq @ Xx @ Xx ) )).

thf(satz50,axiom,(
    ! [Xx: frac,Xy: frac,Xz: frac] :
      ( ( lessf @ Xx @ Xy )
     => ( ( lessf @ Xy @ Xz )
       => ( lessf @ Xx @ Xz ) ) ) )).

thf(satz51b,conjecture,
    ( lessf @ x @ z )).

%------------------------------------------------------------------------------
