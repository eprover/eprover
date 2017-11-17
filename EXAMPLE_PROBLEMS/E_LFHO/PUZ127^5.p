%------------------------------------------------------------------------------
% File     : PUZ127^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Puzzles
% Problem  : TPS problem from CHECKERBOARD-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1182 [Bro09]

% Status   : Theorem
% Rating   : 0.75 v7.0.0, 0.71 v6.4.0, 0.83 v6.3.0, 0.80 v6.2.0, 0.71 v5.5.0, 0.67 v5.4.0, 1.00 v4.0.0
% Syntax   : Number of formulae    :    9 (   0 unit;   8 type;   0 defn)
%            Number of atoms       :  122 (  25 equality;  30 variable)
%            Maximal formula depth :   22 (   5 average)
%            Number of connectives :   77 (   6   ~;   5   |;  16   &;  47   @)
%                                         (   3 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    3 (   3   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    9 (   8   :;   0   =)
%            Number of variables   :   11 (   0 sgn;  11   !;   0   ?;   0   ^)
%                                         (  11   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(c5,type,(
    c5: $i )).

thf(g,type,(
    g: $i > $i > $i )).

thf(c4,type,(
    c4: $i )).

thf(c3,type,(
    c3: $i )).

thf(c2,type,(
    c2: $i )).

thf(c1,type,(
    c1: $i )).

thf(s,type,(
    s: $i > $i )).

thf(c8,type,(
    c8: $i )).

thf(cTOUGHNUT2,conjecture,(
    ~ ( ( ( s @ ( s @ ( s @ ( s @ ( s @ ( s @ ( s @ ( s @ c8 ) ) ) ) ) ) ) )
        = c8 )
      & ! [Xx: $i] :
          ( ( s @ ( s @ ( s @ ( s @ Xx ) ) ) )
         != Xx )
      & ! [Xx: $i,Xy: $i] :
          ( ( ( g @ Xx @ Xy )
            = c5 )
        <=> ( ( ( Xx = c8 )
              & ( Xy = c8 ) )
            | ( ( Xx = c1 )
              & ( Xy = c1 ) ) ) )
      & ! [Xx: $i,Xy: $i] :
          ( ( ( g @ Xx @ Xy )
            = c1 )
        <=> ( ( g @ ( s @ Xx ) @ Xy )
            = c3 ) )
      & ! [Xx: $i,Xy: $i] :
          ( ( ( g @ Xx @ Xy )
            = c2 )
        <=> ( ( g @ Xx @ ( s @ Xy ) )
            = c4 ) )
      & ! [Xx: $i,Xy: $i] :
          ( ( ( g @ c1 @ Xy )
           != c3 )
          & ( ( g @ c8 @ Xy )
           != c1 )
          & ( ( g @ Xx @ c1 )
           != c4 )
          & ( ( g @ Xx @ c8 )
           != c2 ) )
      & ( c1
        = ( s @ c8 ) )
      & ( c2
        = ( s @ c1 ) )
      & ( c3
        = ( s @ c2 ) )
      & ( c4
        = ( s @ c3 ) )
      & ( c5
        = ( s @ c4 ) )
      & ! [Xx: $i,Xy: $i] :
          ( ( ( g @ Xx @ Xy )
            = c1 )
          | ( ( g @ Xx @ Xy )
            = c2 )
          | ( ( g @ Xx @ Xy )
            = c3 )
          | ( ( g @ Xx @ Xy )
            = c4 )
          | ( ( g @ Xx @ Xy )
            = c5 ) ) ) )).

%------------------------------------------------------------------------------
