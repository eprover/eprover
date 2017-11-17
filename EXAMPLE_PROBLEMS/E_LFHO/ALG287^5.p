%------------------------------------------------------------------------------
% File     : ALG287^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from PAIRING-UNPAIRING-ALG-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1000 [Bro09]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.40 v6.2.0, 0.14 v5.5.0, 0.33 v5.4.0, 0.40 v4.1.0, 0.00 v4.0.1, 0.33 v4.0.0
% Syntax   : Number of formulae    :   10 (   0 unit;   9 type;   0 defn)
%            Number of atoms       :   43 (   9 equality;  10 variable)
%            Maximal formula depth :   12 (   3 average)
%            Number of connectives :   25 (   1   ~;   0   |;   5   &;  16   @)
%                                         (   1 <=>;   2  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   10 (   9   :;   0   =)
%            Number of variables   :    5 (   0 sgn;   5   !;   0   ?;   0   ^)
%                                         (   5   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(v,type,(
    v: a )).

thf(u,type,(
    u: a )).

thf(y,type,(
    y: a )).

thf(x,type,(
    x: a )).

thf(cP,type,(
    cP: a > a > a )).

thf(cR,type,(
    cR: a > a )).

thf(cL,type,(
    cL: a > a )).

thf(cZ,type,(
    cZ: a )).

thf(cPU_P_INJ_pme,conjecture,
    ( ( ( ( cL @ cZ )
        = cZ )
      & ( ( cR @ cZ )
        = cZ )
      & ! [Xx0: a,Xy0: a] :
          ( ( cL @ ( cP @ Xx0 @ Xy0 ) )
          = Xx0 )
      & ! [Xx0: a,Xy0: a] :
          ( ( cR @ ( cP @ Xx0 @ Xy0 ) )
          = Xy0 )
      & ! [Xt: a] :
          ( ( Xt != cZ )
        <=> ( Xt
            = ( cP @ ( cL @ Xt ) @ ( cR @ Xt ) ) ) ) )
   => ( ( ( cP @ x @ u )
        = ( cP @ y @ v ) )
     => ( ( x = y )
        & ( u = v ) ) ) )).

%------------------------------------------------------------------------------
