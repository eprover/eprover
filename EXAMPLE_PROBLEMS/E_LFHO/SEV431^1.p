%------------------------------------------------------------------------------
% File     : SEV431^1 : TPTP v7.0.0. Released v5.2.0.
% Domain   : Set Theory
% Problem  : Injective functions f:A->B have left inverses
% Version  : Especial.
% English  :

% Refs     : [Bro11] Brown (2011), Email to Geoff Sutcliffe
% Source   : [Bro11]
% Names    : INVEXISTS4 [Bro11]

% Status   : Theorem
% Rating   : 0.62 v7.0.0, 0.57 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.71 v5.5.0, 0.83 v5.4.0, 0.80 v5.2.0
% Syntax   : Number of formulae    :    5 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :   13 (   3 equality;   7 variable)
%            Maximal formula depth :    6 (   4 average)
%            Number of connectives :    5 (   0   ~;   0   |;   0   &;   4   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    2 (   2   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    4 (   3   :;   0   =)
%            Number of variables   :    4 (   0 sgn;   3   !;   1   ?;   0   ^)
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

thf(finj,axiom,(
    ! [X: a,Y: a] :
      ( ( ( f @ X )
        = ( f @ Y ) )
     => ( X = Y ) ) )).

thf(invexists,conjecture,(
    ? [G: b > a] :
    ! [X: a] :
      ( ( G @ ( f @ X ) )
      = X ) )).

%------------------------------------------------------------------------------
