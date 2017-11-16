%------------------------------------------------------------------------------
% File     : SEV344^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Set Theory (GvNB)
% Problem  : TPS problem from GVB-MB-AXIOMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0738 [Bro09]

% Status   : CounterSatisfiable
% Rating   : 0.00 v4.0.0
% Syntax   : Number of formulae    :    6 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :   11 (   1 equality;   3 variable)
%            Maximal formula depth :    7 (   4 average)
%            Number of connectives :    9 (   1   ~;   0   |;   2   &;   5   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    5 (   5   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   5   :;   0   =)
%            Number of variables   :    1 (   0 sgn;   0   !;   1   ?;   0   ^)
%                                         (   1   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_CSA_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
%------------------------------------------------------------------------------
thf(x,type,(
    x: $i )).

thf(cGVB_DISJOINT,type,(
    cGVB_DISJOINT: $i > $i > $o )).

thf(cGVB_IN,type,(
    cGVB_IN: $i > $i > $o )).

thf(cGVB_M,type,(
    cGVB_M: $i > $o )).

thf(cGVB_ZERO,type,(
    cGVB_ZERO: $i )).

thf(cGVB_D,conjecture,
    ( ( x != cGVB_ZERO )
   => ? [Xu: $i] :
        ( ( cGVB_M @ Xu )
        & ( cGVB_IN @ Xu @ x )
        & ( cGVB_DISJOINT @ Xu @ x ) ) )).

%------------------------------------------------------------------------------
