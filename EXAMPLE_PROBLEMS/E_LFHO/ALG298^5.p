%------------------------------------------------------------------------------
% File     : ALG298^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra
% Problem  : TPS problem THM270
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0400 [Bro09]
%          : THM270 [TPS]

% Status   : Theorem
% Rating   : 0.50 v7.0.0, 0.57 v6.4.0, 0.67 v6.3.0, 0.60 v6.2.0, 0.29 v6.1.0, 0.43 v5.5.0, 0.33 v5.4.0, 0.40 v5.3.0, 0.60 v5.2.0, 0.40 v5.1.0, 0.60 v5.0.0, 0.40 v4.1.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    7 (   0 unit;   6 type;   0 defn)
%            Number of atoms       :   40 (   5 equality;  29 variable)
%            Maximal formula depth :   14 (   5 average)
%            Number of connectives :   29 (   0   ~;   0   |;   3   &;  25   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    9 (   9   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   6   :;   0   =)
%            Number of variables   :   12 (   0 sgn;  11   !;   1   ?;   0   ^)
%                                         (  12   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : Polymorphic definitions expanded.
%------------------------------------------------------------------------------
thf(c_type,type,(
    c: $tType )).

thf(b_type,type,(
    b: $tType )).

thf(a_type,type,(
    a: $tType )).

thf(c_starc,type,(
    c_starc: c > c > c )).

thf(c_starb,type,(
    c_starb: b > b > b )).

thf(c_stara,type,(
    c_stara: a > a > a )).

thf(cTHM270_pme,conjecture,(
    ! [Xf: a > b,Xg: a > c,Xh: b > c] :
      ( ( ! [Xx: a] :
            ( ( Xh @ ( Xf @ Xx ) )
            = ( Xg @ Xx ) )
        & ! [Xy: b] :
          ? [Xx: a] :
            ( ( Xf @ Xx )
            = Xy )
        & ! [Xx: a,Xy: a] :
            ( ( Xf @ ( c_stara @ Xx @ Xy ) )
            = ( c_starb @ ( Xf @ Xx ) @ ( Xf @ Xy ) ) )
        & ! [Xx: a,Xy: a] :
            ( ( Xg @ ( c_stara @ Xx @ Xy ) )
            = ( c_starc @ ( Xg @ Xx ) @ ( Xg @ Xy ) ) ) )
     => ! [Xx: b,Xy: b] :
          ( ( Xh @ ( c_starb @ Xx @ Xy ) )
          = ( c_starc @ ( Xh @ Xx ) @ ( Xh @ Xy ) ) ) ) )).

%------------------------------------------------------------------------------
