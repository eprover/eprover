%------------------------------------------------------------------------------
% File     : PUZ140^2 : TPTP v7.0.0. Released v6.4.0.
% Domain   : Puzzles
% Problem  : A mixture of coffee and syrup that is hot
% Version  : Especial.
% English  : 

% Refs     : 
% Source   : [TPTP]
% Names    :

% Status   : Theorem
% Rating   : 0.12 v7.0.0, 0.00 v6.4.0
% Syntax   : Number of formulae    :    9 (   0 unit;   6 type;   1 defn)
%            Number of atoms       :   20 (   4 equality;   7 variable)
%            Maximal formula depth :    7 (   3 average)
%            Number of connectives :    9 (   0   ~;   0   |;   3   &;   6   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    5 (   5   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    8 (   6   :;   0   =)
%            Number of variables   :    4 (   0 sgn;   2   !;   2   ?;   0   ^)
%                                         (   4   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This version highlights partial application
%------------------------------------------------------------------------------
thf(syrup_type,type,(
    syrup: $tType )).

thf(beverage_type,type,(
    beverage: $tType )).

thf(coffee_type,type,(
    coffee: beverage )).

thf(mix_type,type,(
    mix: beverage > syrup > beverage )).

thf(coffee_mixture_type,type,(
    coffee_mixture: syrup > beverage )).

thf(hot_type,type,(
    hot: beverage > $o )).

%----The mixture of coffee and something
thf(coffee_mixture_definition,definition,
    ( coffee_mixture
    =  mix @ coffee  )).

%----Any coffee mixture is hot coffee
thf(coffee_and_syrup_is_hot_coffee,axiom,(
    ! [S: syrup] :
      ( (  coffee_mixture @ S 
        = coffee )
      & ( hot @ ( coffee_mixture @ S ) ) ) )).

%----There is some mixture of coffee and any syrup which is hot coffee
thf(there_is_hot_coffee,conjecture,(
    ? [SyrupMixer: syrup > beverage] :
    ! [S: syrup] :
    ? [B: beverage] :
      ( ( B
        =  SyrupMixer @ S  )
      & ( B = coffee )
      & ( hot @ B ) ) )).

%------------------------------------------------------------------------------
