%------------------------------------------------------------------------------
% File     : PUZ091^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Puzzles
% Problem  : TPS problem from BASIC-FO-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1153 [Bro09]
%          : tps_1152 [Bro09]

% Status   : Theorem
% Rating   : 0.43 v7.0.0, 0.38 v6.4.0, 0.43 v6.3.0, 0.50 v6.2.0, 0.33 v6.1.0, 0.50 v6.0.0, 0.33 v5.5.0, 0.20 v5.4.0, 0.25 v5.2.0, 0.00 v4.1.0, 0.33 v4.0.0
% Syntax   : Number of formulae    :   12 (   0 unit;  11 type;   0 defn)
%            Number of atoms       :   58 (   0 equality;   0 variable)
%            Maximal formula depth :   20 (   4 average)
%            Number of connectives :   80 (  23   ~;   0   |;  40   &;   0   @)
%                                         (   0 <=>;  17  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    0 (   0   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   12 (  11   :;   0   =)
%            Number of variables   :    0 (   0 sgn;   0   !;   0   ?;   0   ^)
%                                         (   0   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(cIN_BED_BEFORE_FOUR,type,(
    cIN_BED_BEFORE_FOUR: $o )).

thf(cRISES_AT_FIVE,type,(
    cRISES_AT_FIVE: $o )).

thf(cLOGICIAN,type,(
    cLOGICIAN: $o )).

thf(cEARNEST,type,(
    cEARNEST: $o )).

thf(cGAMBLER,type,(
    cGAMBLER: $o )).

thf(cLIKELY_LOSE_MONEY,type,(
    cLIKELY_LOSE_MONEY: $o )).

thf(cLIVELY,type,(
    cLIVELY: $o )).

thf(cRAVENOUS,type,(
    cRAVENOUS: $o )).

thf(cBETTER_TAKE_TO_CAB_DRIVING,type,(
    cBETTER_TAKE_TO_CAB_DRIVING: $o )).

thf(cEATS_PORKCHOPS,type,(
    cEATS_PORKCHOPS: $o )).

thf(cHAS_LOST_MONEY,type,(
    cHAS_LOST_MONEY: $o )).

thf(cPORKCHOP1,conjecture,
    ( ( ( ( cLOGICIAN
          & cEATS_PORKCHOPS )
       => cLIKELY_LOSE_MONEY )
      & ( ( cGAMBLER
          & ~ ( cRAVENOUS ) )
       => cLIKELY_LOSE_MONEY )
      & ( ( ~ ( cLIVELY )
          & cHAS_LOST_MONEY
          & cLIKELY_LOSE_MONEY )
       => cRISES_AT_FIVE )
      & ( ( ~ ( cGAMBLER )
          & ~ ( cEATS_PORKCHOPS ) )
       => cRAVENOUS )
      & ( ( cLIVELY
          & cIN_BED_BEFORE_FOUR )
       => cBETTER_TAKE_TO_CAB_DRIVING )
      & ( ( cRAVENOUS
          & ~ ( cHAS_LOST_MONEY )
          & ~ ( cRISES_AT_FIVE ) )
       => cEATS_PORKCHOPS )
      & ( ( cLOGICIAN
          & cLIKELY_LOSE_MONEY )
       => cBETTER_TAKE_TO_CAB_DRIVING )
      & ( ( cEARNEST
          & ~ ( cLIVELY )
          & cGAMBLER
          & ~ ( cHAS_LOST_MONEY ) )
       => ~ ( cLIKELY_LOSE_MONEY ) )
      & ( ( ~ ( cGAMBLER )
          & ~ ( cRAVENOUS ) )
       => cLIVELY )
      & ( ( cLIVELY
          & cLOGICIAN
          & cEARNEST )
       => ~ ( cLIKELY_LOSE_MONEY ) )
      & ( ( cRAVENOUS
          & cEARNEST )
       => ~ ( cBETTER_TAKE_TO_CAB_DRIVING ) )
      & ( ( cGAMBLER
          & ~ ( cLIVELY )
          & ~ ( cLIKELY_LOSE_MONEY ) )
       => ~ ( cIN_BED_BEFORE_FOUR ) )
      & ( ( cHAS_LOST_MONEY
          & ~ ( cEATS_PORKCHOPS )
          & ~ ( cRISES_AT_FIVE ) )
       => cBETTER_TAKE_TO_CAB_DRIVING )
      & ( ( cGAMBLER
          & cIN_BED_BEFORE_FOUR
          & ~ ( cRAVENOUS ) )
       => ~ ( cBETTER_TAKE_TO_CAB_DRIVING ) )
      & ( ( cRAVENOUS
          & ~ ( cLIVELY )
          & ~ ( cLIKELY_LOSE_MONEY ) )
       => cGAMBLER ) )
   => ( ( cEARNEST
        & cLOGICIAN )
     => ( cRISES_AT_FIVE
        & ~ ( cIN_BED_BEFORE_FOUR ) ) ) )).

%------------------------------------------------------------------------------
