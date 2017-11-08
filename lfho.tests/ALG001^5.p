%------------------------------------------------------------------------------
% File     : ALG001^5 : TPTP v7.0.0. Released v4.0.0.
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
% Rating   : 0.00 v6.0.0, 0.14 v5.5.0, 0.17 v5.4.0, 0.20 v5.3.0, 0.40 v5.2.0, 0.20 v4.1.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    4 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :   33 (   3 equality;  30 variable)
%            Maximal formula depth :   14 (   5 average)
%            Number of connectives :   26 (   0   ~;   0   |;   1   &;  24   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    8 (   8   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    4 (   3   :;   0   =)
%            Number of variables   :   11 (   0 sgn;  11   !;   0   ?;   0   ^)
%                                         (  11   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : Polymorphic definitions expanded.
%------------------------------------------------------------------------------
thf(g_type,type,(
    g: $tType )).

thf(b_type,type,(
    b: $tType )).

thf(a_type,type,(
    a: $tType )).

thf(cTHM133_pme,conjecture,(
    ! [X: g > b,Y: b > a,Z: g > g > g,W: b > b > b,U: a > a > a] :
      ( ( ! [Xx: g,Xy: g] :
            (  X @ ( Z @ Xx @ Xy ) 
            =  W @ ( X @ Xx ) @ ( X @ Xy ) ) 
        & ! [Xx: b,Xy: b] :
            (  Y @ ( W @ Xx @ Xy ) 
            =  U @ ( Y @ Xx ) @ ( Y @ Xy ) ) ) 
     => ! [Xx: g,Xy: g] :
          (  Y @ ( X @ ( Z @ Xx @ Xy ) ) 
          =  U @ ( Y @ ( X @ Xx ) ) @ ( Y @ ( X @ Xy ) ) ) ) ) ).
%------------------------------------------------------------------------------
