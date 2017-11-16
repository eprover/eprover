%------------------------------------------------------------------------------
% File     : SEV051^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Set Theory (Relations)
% Problem  : TPS problem THM557
% Version  : Especial.
% English  : Equality is an LC-relation.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0210 [Bro09]
%          : THM557 [TPS]

% Status   : Theorem
% Rating   : 0.00 v6.2.0, 0.14 v6.1.0, 0.00 v6.0.0, 0.14 v5.5.0, 0.17 v5.4.0, 0.20 v4.1.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    2 (   0 unit;   1 type;   0 defn)
%            Number of atoms       :   12 (   4 equality;   8 variable)
%            Maximal formula depth :    8 (   5 average)
%            Number of connectives :    3 (   0   ~;   0   |;   2   &;   0   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    0 (   0   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    2 (   1   :;   0   =)
%            Number of variables   :    4 (   0 sgn;   4   !;   0   ?;   0   ^)
%                                         (   4   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : Polymorphic definitions expanded.
%          : 
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(cTHM557_pme,conjecture,
    ( ! [Xx: a] : ( Xx = Xx )
    & ! [Xu: a,Xv: a,Xw: a] :
        ( ( ( Xu = Xw )
          & ( Xv = Xw ) )
       => ( Xu = Xv ) ) )).

%------------------------------------------------------------------------------
