%------------------------------------------------------------------------------
% File     : SEV432^1 : TPTP v7.0.0. Released v5.2.0.
% Domain   : Set Theory
% Problem  : Surjective functions f:A->B have right inverses
% Version  : Especial.
% English  :

% Refs     : [Bro11] Brown (2011), Email to Geoff Sutcliffe
% Source   : [Bro11]
% Names    : INVEXISTS5 [Bro11]

% Status   : Theorem
% Rating   : 0.00 v6.2.0, 0.43 v6.1.0, 0.14 v6.0.0, 0.29 v5.5.0, 0.33 v5.4.0, 0.40 v5.2.0
% Syntax   : Number of formulae    :    5 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :    9 (   2 equality;   5 variable)
%            Maximal formula depth :    6 (   4 average)
%            Number of connectives :    3 (   0   ~;   0   |;   0   &;   3   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    2 (   2   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    4 (   3   :;   0   =)
%            Number of variables   :    4 (   0 sgn;   2   !;   2   ?;   0   ^)
%                                         (   4   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(a,type,(
    a: $tType )).

thf(b,type,(
    b: $tType )).

thf(f,type,(
    f: a > b )).

thf(fsurj,axiom,(
    ! [Y: b] :
    ? [X: a] :
      ( ( f @ X )
      = Y ) )).

thf(invexists,conjecture,(
    ? [G: b > a] :
    ! [X: b] :
      ( ( f @ ( G @ X ) )
      = X ) )).

%------------------------------------------------------------------------------
