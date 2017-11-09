%------------------------------------------------------------------------------
% File     : PUZ047^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Puzzles
% Problem  : TPS problem THM100A
% Version  : Especial.
% English  : A naive formalization of the problem of moving man wolf goat
%            cabbage from south to north side of river.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0543 [Bro09]
%          : tps_0544 [Bro09]
%          : tps_0545 [Bro09]
%          : tps_0427 [Bro09]
%          : THM100 [TPS]
%          : THM100A [TPS]
%          : THM100B [TPS]
%          : THM100-TPS2 [TPS]

% Status   : Theorem
% Rating   : 0.29 v7.0.0, 0.25 v6.4.0, 0.29 v6.3.0, 0.33 v6.2.0, 0.17 v6.1.0, 0.33 v6.0.0, 0.17 v5.5.0, 0.00 v5.3.0, 0.25 v5.2.0, 0.00 v4.0.1, 0.33 v4.0.0
% Syntax   : Number of formulae    :   11 (   0 unit;  10 type;   0 defn)
%            Number of atoms       :  194 (   0 equality;  37 variable)
%            Maximal formula depth :   23 (   5 average)
%            Number of connectives :  193 (   0   ~;   0   |;  14   &; 164   @)
%                                         (   0 <=>;  15  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    9 (   9   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   12 (  10   :;   0   =)
%            Number of variables   :   19 (   0 sgn;  18   !;   1   ?;   0   ^)
%                                         (  19   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(b_type,type,(
    b: $tType )).

thf(cN,type,(
    cN: a )).

thf(cP,type,(
    cP: a > a > a > a > b > $o )).

thf(cD,type,(
    cD: b > b )).

thf(cS,type,(
    cS: a )).

thf(cG,type,(
    cG: b > b )).

thf(cW,type,(
    cW: b > b )).

thf(cL,type,(
    cL: b > b )).

thf(cO,type,(
    cO: b )).

thf(cTHM100A,conjecture,
    ( ( ( cP @ cS @ cS @ cS @ cS @ cO )
      & ! [T: b] :
          ( ( cP @ cS @ cN @ cS @ cN @ T )
         => ( cP @ cN @ cN @ cS @ cN @ ( cL @ T ) ) )
      & ! [T1: b] :
          ( ( cP @ cN @ cN @ cS @ cN @ T1 )
         => ( cP @ cS @ cN @ cS @ cN @ ( cL @ T1 ) ) )
      & ! [T2: b] :
          ( ( cP @ cS @ cS @ cN @ cS @ T2 )
         => ( cP @ cN @ cS @ cN @ cS @ ( cL @ T2 ) ) )
      & ! [T3: b] :
          ( ( cP @ cN @ cS @ cN @ cS @ T3 )
         => ( cP @ cS @ cS @ cN @ cS @ ( cL @ T3 ) ) )
      & ! [T4: b] :
          ( ( cP @ cS @ cS @ cS @ cN @ T4 )
         => ( cP @ cN @ cN @ cS @ cN @ ( cW @ T4 ) ) )
      & ! [T5: b] :
          ( ( cP @ cN @ cN @ cS @ cN @ T5 )
         => ( cP @ cS @ cS @ cS @ cN @ ( cW @ T5 ) ) )
      & ! [T6: b] :
          ( ( cP @ cS @ cS @ cN @ cS @ T6 )
         => ( cP @ cN @ cN @ cN @ cS @ ( cW @ T6 ) ) )
      & ! [T7: b] :
          ( ( cP @ cN @ cN @ cN @ cS @ T7 )
         => ( cP @ cS @ cS @ cN @ cS @ ( cW @ T7 ) ) )
      & ! [X: a,Y: a,U: b] :
          ( ( cP @ cS @ X @ cS @ Y @ U )
         => ( cP @ cN @ X @ cN @ Y @ ( cG @ U ) ) )
      & ! [X1: a,Y1: a,V: b] :
          ( ( cP @ cN @ X1 @ cN @ Y1 @ V )
         => ( cP @ cS @ X1 @ cS @ Y1 @ ( cG @ V ) ) )
      & ! [T8: b] :
          ( ( cP @ cS @ cN @ cS @ cS @ T8 )
         => ( cP @ cN @ cN @ cS @ cN @ ( cD @ T8 ) ) )
      & ! [T9: b] :
          ( ( cP @ cN @ cN @ cS @ cN @ T9 )
         => ( cP @ cS @ cN @ cS @ cS @ ( cD @ T9 ) ) )
      & ! [U1: b] :
          ( ( cP @ cS @ cS @ cN @ cS @ U1 )
         => ( cP @ cN @ cS @ cN @ cN @ ( cD @ U1 ) ) )
      & ! [V1: b] :
          ( ( cP @ cN @ cS @ cN @ cN @ V1 )
         => ( cP @ cS @ cS @ cN @ cS @ ( cD @ V1 ) ) ) )
   => ? [Z: b] :
        ( cP @ cN @ cN @ cN @ cN @ Z ) )).

%------------------------------------------------------------------------------
