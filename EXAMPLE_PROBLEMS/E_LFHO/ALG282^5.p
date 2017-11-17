%------------------------------------------------------------------------------
% File     : ALG282^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra
% Problem  : TPS problem from GRP-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0920 [Bro09]

% Status   : Theorem
% Rating   : 0.25 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.43 v5.5.0, 0.50 v5.4.0, 0.60 v5.1.0, 0.80 v5.0.0, 0.60 v4.1.0, 0.33 v4.0.0
% Syntax   : Number of formulae    :    5 (   0 unit;   4 type;   0 defn)
%            Number of atoms       :   27 (   4 equality;  13 variable)
%            Maximal formula depth :   11 (   4 average)
%            Number of connectives :   18 (   0   ~;   0   |;   2   &;  15   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    3 (   3   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    5 (   4   :;   0   =)
%            Number of variables   :    8 (   0 sgn;   7   !;   1   ?;   0   ^)
%                                         (   8   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(cP,type,(
    cP: a > a > a )).

thf(cE,type,(
    cE: a )).

thf(cJ,type,(
    cJ: a > a )).

thf(cTHM21_pme,conjecture,
    ( ( ! [Xx: a,Xy: a,Xz: a] :
          ( ( cP @ ( cP @ Xx @ Xy ) @ Xz )
          = ( cP @ Xx @ ( cP @ Xy @ Xz ) ) )
      & ! [Xx: a] :
          ( ( cP @ cE @ Xx )
          = Xx )
      & ! [Xy: a] :
          ( ( cP @ ( cJ @ Xy ) @ Xy )
          = cE ) )
   => ! [X: a,Y: a] :
      ? [W: a] :
        ( ( cP @ W @ X )
        = Y ) )).

%------------------------------------------------------------------------------
