%------------------------------------------------------------------------------
% File     : PUZ031^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Puzzles
% Problem  : TPS problem from BASIC-FO-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1229 [Bro09]

% Status   : Theorem
% Rating   : 0.43 v7.0.0, 0.38 v6.4.0, 0.43 v6.3.0, 0.50 v6.2.0, 0.33 v6.1.0, 0.50 v6.0.0, 0.33 v5.5.0, 0.40 v5.4.0, 0.25 v5.2.0, 0.50 v5.1.0, 0.75 v5.0.0, 0.50 v4.1.0, 0.33 v4.0.1, 0.67 v4.0.0
% Syntax   : Number of formulae    :   19 (   0 unit;  18 type;   0 defn)
%            Number of atoms       :  146 (   0 equality;  73 variable)
%            Maximal formula depth :   37 (   5 average)
%            Number of connectives :  186 (  41   ~;  37   |;  25   &;  83   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :   14 (  14   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :   20 (  18   :;   0   =)
%            Number of variables   :   33 (   0 sgn;  33   !;   0   ?;   0   ^)
%                                         (  33   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(eats,type,(
    eats: $i > $i > $o )).

thf(grain,type,(
    grain: $i > $o )).

thf(animal,type,(
    animal: $i > $o )).

thf(snail,type,(
    snail: $i > $o )).

thf(sf,type,(
    sf: $i > $i )).

thf(plant,type,(
    plant: $i > $o )).

thf(caterpillar,type,(
    caterpillar: $i > $o )).

thf(cf,type,(
    cf: $i > $i )).

thf(bird,type,(
    bird: $i > $o )).

thf(wolf,type,(
    wolf: $i > $o )).

thf(fox,type,(
    fox: $i > $o )).

thf(msmaller,type,(
    msmaller: $i > $i > $o )).

thf(a_grain,type,(
    a_grain: $i )).

thf(a_snail,type,(
    a_snail: $i )).

thf(a_caterpillar,type,(
    a_caterpillar: $i )).

thf(a_bird,type,(
    a_bird: $i )).

thf(a_fox,type,(
    a_fox: $i )).

thf(a_wolf,type,(
    a_wolf: $i )).

thf(cPUZ031_1,conjecture,(
    ~ ( ! [X: $i] :
          (  ( animal @ X )
          | ~ ( wolf @ X ) )
      & ! [X: $i] :
          ( ( animal @ X )
          | ~ ( fox @ X ) )
      & ! [X: $i] :
          ( ( animal @ X )
          | ~ ( bird @ X ) )
      & ! [X: $i] :
          ( ( animal @ X )
          | ~ ( caterpillar @ X ) )
      & ! [X: $i] :
          ( ( animal @ X )
          | ~ ( snail @ X ) )
      & ( wolf @ a_wolf )
      & ( fox @ a_fox )
      & ( bird @ a_bird )
      & ( caterpillar @ a_caterpillar )
      & ( snail @ a_snail )
      & ( grain @ a_grain )
      & ! [X: $i] :
          ( ( plant @ X )
          | ~ ( grain @ X ) )
      & ! [A: $i,P: $i,S: $i,O: $i] :
          ( ( eats @ A @ P )
          | ( eats @ A @ S )
          | ~ ( animal @ A )
          | ~ ( plant @ P )
          | ~ ( animal @ S )
          | ~ ( plant @ O )
          | ~ ( msmaller @ S @ A )
          | ~ ( eats @ S @ O ) )
      & ! [C: $i,B: $i] :
          ( ( msmaller @ C @ B )
          | ~ ( caterpillar @ C )
          | ~ ( bird @ B ) )
      & ! [S: $i,B: $i] :
          ( ( msmaller @ S @ B )
          | ~ ( snail @ S )
          | ~ ( bird @ B ) )
      & ! [B: $i,F: $i] :
          ( ( msmaller @ B @ F )
          | ~ ( bird @ B )
          | ~ ( fox @ F ) )
      & ! [F: $i,W: $i] :
          ( ( msmaller @ F @ W )
          | ~ ( fox @ F )
          | ~ ( wolf @ W ) )
      & ! [F: $i,W: $i] :
          ( ~ ( wolf @ W )
          | ~ ( fox @ F )
          | ~ ( eats @ W @ F ) )
      & ! [W: $i,G: $i] :
          ( ~ ( wolf @ W )
          | ~ ( grain @ G )
          | ~ ( eats @ W @ G ) )
      & ! [B: $i,C: $i] :
          ( ( eats @ B @ C )
          | ~ ( bird @ B )
          | ~ ( caterpillar @ C ) )
      & ! [B: $i,S: $i] :
          ( ~ ( bird @ B )
          | ~ ( snail @ S )
          | ~ ( eats @ B @ S ) )
      & ! [C: $i] :
          ( ( plant @ ( cf @ C ) )
          | ~ ( caterpillar @ C ) )
      & ! [C: $i] :
          ( ( eats @ C @ ( cf @ C ) )
          | ~ ( caterpillar @ C ) )
      & ! [S: $i] :
          ( ( plant @ ( sf @ S ) )
          | ~ ( snail @ S ) )
      & ! [S: $i] :
          ( ( eats @ S @ ( sf @ S ) )
          | ~ ( snail @ S ) )
      & ! [A: $i,E: $i,G: $i] :
          ( ~ ( animal @ A )
          | ~ ( animal @ E )
          | ~ ( grain @ G )
          | ~ ( eats @ A @ E )
          | ~ ( eats @ E @ G ) ) ) )).

%------------------------------------------------------------------------------
