%------------------------------------------------------------------------------
% File     : PUZ088^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Puzzles
% Problem  : TPS problem THM68
% Version  : Especial.
% English  : If everyone likes Bruce and Lyle likes everyone who likes someone
%            then someone likes everyone.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0122 [Bro09]
%          : tps_0220 [Bro09]
%          : tps_0223 [Bro09]
%          : THM68 [TPS]
%          : THM68A [TPS]
%          : THM68B [TPS]
%          : THM102 [TPS]

% Status   : Theorem
% Rating   : 0.00 v6.1.0, 0.17 v6.0.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    4 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :   12 (   0 equality;   6 variable)
%            Maximal formula depth :    8 (   4 average)
%            Number of connectives :   11 (   0   ~;   0   |;   1   &;   8   @)
%                                         (   0 <=>;   2  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    2 (   2   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    5 (   3   :;   0   =)
%            Number of variables   :    5 (   0 sgn;   3   !;   2   ?;   0   ^)
%                                         (   5   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(cLIKES,type,(
    cLIKES: $i > $i > $o )).

thf(cLYLE,type,(
    cLYLE: $i )).

thf(cBRUCE,type,(
    cBRUCE: $i )).

thf(cTHM68A,conjecture,
    ( ( ! [X: $i] :
          ( cLIKES @ X @ cBRUCE )
      & ! [Y: $i] :
          ( ? [Z: $i] :
              ( cLIKES @ Y @ Z )
         => ( cLIKES @ cLYLE @ Y ) ) )
   => ? [U: $i] :
      ! [V: $i] :
        ( cLIKES @ U @ V ) )).

%------------------------------------------------------------------------------
