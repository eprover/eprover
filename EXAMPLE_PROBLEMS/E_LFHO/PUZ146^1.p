%------------------------------------------------------------------------------
% File     : PUZ146^1 : TPTP v7.0.0. Released v6.4.0.
% Domain   : Puzzles
% Problem  : Peter and Mary have different hobbies
% Version  : Especial.
% English  : 

% Refs     : [Ben16] Benzmueller (2011), Email to Geoff Sutcliffe
% Source   : [Ben16]
% Names    :

% Status   : Theorem
% Rating   : 0.00 v6.4.0
% Syntax   : Number of formulae    :   19 (   0 unit;  10 type;   2 defn)
%            Number of atoms       :   31 (   7 equality;   0 variable)
%            Maximal formula depth :    5 (   3 average)
%            Number of connectives :   14 (   6   ~;   0   |;   2   &;   6   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   12 (  10   :;   0   =)
%            Number of variables   :    0 (   0 sgn;   0   !;   0   ?;   0   ^)
%                                         (   0   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(hobby_type,type,(
    hobby: $tType )).

thf(earthling_type,type,(
    earthling: $tType )).

thf(peter,type,(
    peter: earthling )).

thf(mary,type,(
    mary: earthling )).

thf(beer_drinking,type,(
    beer_drinking: hobby )).

thf(belly_dancing,type,(
    belly_dancing: hobby )).

thf(weight_lifting,type,(
    weight_lifting: hobby )).

thf(has_hobby,type,(
    has_hobby: earthling > hobby > $o )).

thf(peters_hobbies,type,(
    peters_hobbies: hobby > $o )).

thf(marys_hobbies,type,(
    marys_hobbies: hobby > $o )).

thf(not_the_same_1,axiom,(
    peter != mary )).

thf(not_the_same_2,axiom,
    ( ( beer_drinking != belly_dancing )
    & ( belly_dancing != weight_lifting )
    & ( beer_drinking != weight_lifting ) )).

thf(peters_hobbies_001,definition,
    ( peters_hobbies
    = ( has_hobby @ peter ) )).

thf(marys_hobbies_002,definition,
    ( marys_hobbies
    = ( has_hobby @ mary ) )).

thf(mary_does_belly_dancing,axiom,
    ( marys_hobbies @ belly_dancing )).

thf(mary_does_not_do_beer_drinking,axiom,(
    ~ ( marys_hobbies @ beer_drinking ) )).

thf(peter_does_beer_drinking,axiom,
    ( peters_hobbies @ beer_drinking )).

thf(peter_does_weight_lifting,axiom,
    ( peters_hobbies @ weight_lifting )).

thf(peter_and_mary_have_different_hobbies,conjecture,(
    peters_hobbies != marys_hobbies )).

%------------------------------------------------------------------------------
