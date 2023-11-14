%------------------------------------------------------------------------------
% File     : ALG001^5 : TPTP v8.2.0. Released v4.0.0.
% Domain   : General Algebra
% Problem  : TPS problem THM133
% Version  : Especial.
% English  : The composition of homomorphisms of binary operators is a
%            homomorphisms. Boyer et al JAR 2 page 284.

% Refs     : [BL+86] Boyer et al. (1986), Set Theory in First-Order Logic:
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0365 [Bro09]
%          : THM133 [TPS]
%          : Problem 221-223 [BL+86]

% Status   : Theorem
% Rating   : 0.00 v6.0.0, 0.14 v5.5.0, 0.17 v5.4.0, 0.20 v5.3.0, 0.40 v5.2.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    4 (   0 unt;   3 typ;   0 def)
%            Number of atoms       :    3 (   3 equ;   0 cnn)
%            Maximal formula atoms :    3 (   3 avg)
%            Number of connectives :   26 (   0   ~;   0   |;   1   &;  24   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%            Maximal formula depth :   10 (  10 avg)
%            Number of types       :    3 (   3 usr)
%            Number of type conns  :    8 (   8   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    1 (   0 usr;   0 con; 2-2 aty)
%            Number of variables   :   11 (   0   ^;  11   !;   0   ?;  11   :)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : Polymorphic definitions expanded.
%------------------------------------------------------------------------------
thf(g_type,type,
    g: $tType ).

thf(b_type,type,
    b: $tType ).

thf(a_type,type,
    a: $tType ).

thf(cTHM133_pme,conjecture,
    ! [Xh1: g > b,Xh2: b > a,Xf1: g > g > g,Xf2: b > b > b,Xf3: a > a > a] :
      ( ( ! [Xx: g,Xy: g] :
            ( ( Xh1 @ ( Xf1 @ Xx @ Xy ) )
            = ( Xf2 @ ( Xh1 @ Xx ) @ ( Xh1 @ Xy ) ) )
        & ! [Xx: b,Xy: b] :
            ( ( Xh2 @ ( Xf2 @ Xx @ Xy ) )
            = ( Xf3 @ ( Xh2 @ Xx ) @ ( Xh2 @ Xy ) ) ) )
     => ! [Xx: g,Xy: g] :
          ( ( Xh2 @ ( Xh1 @ ( Xf1 @ Xx @ Xy ) ) )
          = ( Xf3 @ ( Xh2 @ ( Xh1 @ Xx ) ) @ ( Xh2 @ ( Xh1 @ Xy ) ) ) ) ) ).

%------------------------------------------------------------------------------
