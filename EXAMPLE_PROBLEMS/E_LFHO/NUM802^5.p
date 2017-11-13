%------------------------------------------------------------------------------
% File     : NUM802^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Number Theory
% Problem  : TPS problem BLEDSOE-FENG-8
% Version  : Especial.
% English  : There is a set containing no nonnegative numbers and containing 
%            -2.

% Refs     : [BF93]  Bledsoe & Feng (1993), SET-VAR
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0143 [Bro09]
%          : BLEDSOE-FENG-8 [TPS]
%          : Example 8 [BF93]

% Status   : Theorem
% Rating   : 0.25 v7.0.0, 0.14 v6.4.0, 0.17 v6.3.0, 0.20 v6.2.0, 0.14 v6.1.0, 0.00 v6.0.0, 0.14 v5.5.0, 0.17 v5.4.0, 0.40 v5.2.0, 0.20 v5.1.0, 0.40 v5.0.0, 0.20 v4.1.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    5 (   0 unit;   4 type;   0 defn)
%            Number of atoms       :   15 (   1 equality;   6 variable)
%            Maximal formula depth :    9 (   4 average)
%            Number of connectives :   14 (   2   ~;   0   |;   1   &;   8   @)
%                                         (   0 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    6 (   4   :;   0   =)
%            Number of variables   :    4 (   0 sgn;   3   !;   1   ?;   0   ^)
%                                         (   4   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
%------------------------------------------------------------------------------
thf(c_2,type,(
    c_2: $i )).

thf(absval,type,(
    absval: $i > $i )).

thf(c0,type,(
    c0: $i )).

thf(c_less_,type,(
    c_less_: $i > $i > $o )).

thf(cBLEDSOE_FENG_8,conjecture,
    ( ( c_less_ @ c_2 @ c0 )
   => ( ! [Xu: $i,Xv: $i] :
          ( ( c_less_ @ Xu @ c0 )
         => ( Xu
           != ( absval @ Xv ) ) )
     => ? [A: $i > $o] :
          ( ! [Xy: $i] :
              ~ ( A @ ( absval @ Xy ) )
          & ( A @ c_2 ) ) ) )).

%------------------------------------------------------------------------------
