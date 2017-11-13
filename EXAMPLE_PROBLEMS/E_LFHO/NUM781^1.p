%------------------------------------------------------------------------------
% File     : NUM781^1 : TPTP v7.0.0. Released v3.7.0.
% Domain   : Number Theory
% Problem  : Landau theorem 79
% Version  : Especial.
% English  : y0 = x0

% Refs     : [Lan30] Landau (1930), Grundlagen der Analysis
%          : [vBJ79] van Benthem Jutting (1979), Checking Landau's "Grundla
%          : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : satz79 [Lan30]

% Status   : Theorem
%          : Without extensionality : Theorem
% Rating   : 0.00 v6.0.0, 0.14 v5.5.0, 0.17 v5.4.0, 0.20 v5.3.0, 0.40 v5.2.0, 0.20 v4.1.0, 0.00 v3.7.0
% Syntax   : Number of formulae    :    5 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :    6 (   2 equality;   0 variable)
%            Maximal formula depth :    2 (   2 average)
%            Number of connectives :    0 (   0   ~;   0   |;   0   &;   0   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    0 (   0   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    4 (   3   :;   0   =)
%            Number of variables   :    0 (   0 sgn;   0   !;   0   ?;   0   ^)
%                                         (   0   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(rat_type,type,(
    rat: $tType )).

thf(x0,type,(
    x0: rat )).

thf(y0,type,(
    y0: rat )).

thf(i,axiom,(
    x0 = y0 )).

thf(satz79,conjecture,(
    y0 = x0 )).

%------------------------------------------------------------------------------
