%------------------------------------------------------------------------------
% File     : NUM830^5 : TPTP v7.0.0. Bugfixed v5.3.0.
% Domain   : Number Theory (Induction on naturals)
% Problem  : TPS problem from PA-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0798 [Bro09]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.29 v6.4.0, 0.33 v6.3.0, 0.40 v6.2.0, 0.14 v6.1.0, 0.29 v5.5.0, 0.17 v5.4.0, 0.40 v5.3.0
% Syntax   : Number of formulae    :   14 (   0 unit;   9 type;   4 defn)
%            Number of atoms       :   56 (   9 equality;  12 variable)
%            Maximal formula depth :    9 (   4 average)
%            Number of connectives :   33 (   0   ~;   0   |;   3   &;  29   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    5 (   5   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   11 (   9   :;   0   =)
%            Number of variables   :    6 (   0 sgn;   6   !;   0   ?;   0   ^)
%                                         (   6   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
% Bugfixes : v5.2.0 - Added missing type declarations.
%          : v5.3.0 - Fixed tType to $tType from last bugfixes.
%------------------------------------------------------------------------------
thf(n_type,type,(
    n: $tType )).

thf(c0_type,type,(
    c0: n )).

thf(cS_type,type,(
    cS: n > n )).

thf(c_plus_type,type,(
    c_plus: n > n > n )).

thf(c_star_type,type,(
    c_star: n > n > n )).

thf(cPA_THM1,conjecture,
    ( ( ( ! [Xx: n] :
          (  c_plus @ Xx @ c0 
          = Xx ) )
      & ( ! [Xx: n,Xy: n] :
          (  c_plus @ Xx @ ( cS @ Xy ) 
          =  cS @ ( c_plus @ Xx @ Xy ) ) )
      & ( ! [Xx: n] :
          (  c_star @ Xx @ c0 
          = c0 ) )
      & ( ! [Xx: n,Xy: n] :
          (  c_star @ Xx @ ( cS @ Xy ) 
          =  c_plus @ ( c_star @ Xx @ Xy ) @ Xx ) ) )
   => (  c_star @ ( cS @ ( cS @ c0 ) ) @ ( cS @ ( cS @ c0 ) ) 
      =  c_plus @ ( cS @ ( cS @ c0 ) ) @ ( cS @ ( cS @ c0 ) ) ) ) ).

%------------------------------------------------------------------------------
