%------------------------------------------------------------------------------
% File     : ALG284^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra
% Problem  : TPS problem from GRP-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0930 [Bro09]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.43 v5.5.0, 0.50 v5.4.0, 0.60 v5.1.0, 0.80 v5.0.0, 0.60 v4.1.0, 0.33 v4.0.0
% Syntax   : Number of formulae    :    4 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :   32 (   4 equality;  14 variable)
%            Maximal formula depth :   11 (   5 average)
%            Number of connectives :   23 (   0   ~;   0   |;   2   &;  20   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    3 (   3   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    4 (   3   :;   0   =)
%            Number of variables   :    7 (   0 sgn;   7   !;   0   ?;   0   ^)
%                                         (   7   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(cJ,type,(
    cJ: $i > $i )).

thf(cP,type,(
    cP: $i > $i > $i )).

thf(cE,type,(
    cE: $i )).

thf(cTHM18_pme,conjecture,
    ( ( ! [Xx: $i,Xy: $i,Xz: $i] :
          ( ( cP @ ( cP @ Xx @ Xy ) @ Xz )
          = ( cP @ Xx @ ( cP @ Xy @ Xz ) ) )
      & ! [Xx: $i] :
          ( ( cP @ cE @ Xx )
          = Xx )
      & ! [Xy: $i] :
          ( ( cP @ ( cJ @ Xy ) @ Xy )
          = cE ) )
   => ! [X: $i,Y: $i] :
        ( ( cJ @ ( cP @ X @ Y ) )
        = ( cP @ ( cJ @ Y ) @ ( cJ @ X ) ) ) )).

%------------------------------------------------------------------------------
