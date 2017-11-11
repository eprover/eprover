%------------------------------------------------------------------------------
% File     : COM024^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Computing Theory
% Problem  : TPS problem THM9
% Version  : Especial.
% English  : A very naive version of the recursion theorem. TM X Y is the
%            output of Turing machine X on input Y, TH F is the number of a
%            Turing machine that computes function F.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0189 [Bro09]
%          : THM9 [TPS]

% Status   : Theorem
% Rating   : 0.88 v7.0.0, 0.86 v6.4.0, 0.83 v6.3.0, 0.80 v6.2.0, 0.71 v5.5.0, 0.83 v5.4.0, 0.80 v5.3.0, 1.00 v5.2.0, 0.80 v4.1.0, 0.67 v4.0.0
% Syntax   : Number of formulae    :    3 (   0 unit;   2 type;   0 defn)
%            Number of atoms       :   11 (   2 equality;   5 variable)
%            Maximal formula depth :    7 (   5 average)
%            Number of connectives :    6 (   0   ~;   0   |;   0   &;   5   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    6 (   6   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    3 (   2   :;   0   =)
%            Number of variables   :    3 (   0 sgn;   2   !;   1   ?;   0   ^)
%                                         (   3   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
%------------------------------------------------------------------------------
thf(cTM,type,(
    cTM: $i > $i > $i )).

thf(cTH,type,(
    cTH: ( $i > $i ) > $i )).

thf(cTHM9,conjecture,
    ( ! [G: $i > $i] :
        (  cTM @ ( cTH @ G ) 
        = G )
   => ! [F: $i > $i] :
      ? [N: $i] :
        (  cTM @ ( F @ N ) 
        = ( cTM @ N ) ) )).

%------------------------------------------------------------------------------
