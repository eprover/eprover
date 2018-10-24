%------------------------------------------------------------------------------
% File     : SEV286^5 : TPTP v7.2.0. Released v4.0.0.
% Domain   : Set Theory
% Problem  : TPS problem from TTTP-NATS-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0088 [Bro09]

% Status   : Theorem
% Rating   : 0.00 v6.2.0, 0.14 v6.1.0, 0.00 v6.0.0, 0.14 v5.5.0, 0.17 v5.4.0, 0.20 v4.1.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    3 (   0 unit;   2 type;   0 defn)
%            Number of atoms       :    8 (   2 equality;   6 variable)
%            Maximal formula depth :    7 (   4 average)
%            Number of connectives :    3 (   0   ~;   0   |;   0   &;   2   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    1 (   1   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    3 (   2   :;   0   =)
%            Number of variables   :    3 (   0 sgn;   3   !;   0   ?;   0   ^)
%                                         (   3   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(b_type,type,(
    b: $tType )).

thf(cEC_eq_,conjecture,(
    ! [F: a > b,A: a,B: a] :
      ( ( A = B )
     => ( ( F @ A )
        = ( F @ B ) ) ) )).

%------------------------------------------------------------------------------