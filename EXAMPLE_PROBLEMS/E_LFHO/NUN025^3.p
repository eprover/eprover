%------------------------------------------------------------------------------
% File     : NUN025^3 : TPTP v7.0.0. Released v6.4.0.
% Domain   : Number Theory
% Problem  : Function h s.t. h(0) = 1, h(1) = 0, h(2) = 1, with witness
% Version  : Especial.
% English  : Using an axiomatiztion of if-then-else, find the if-then-else
%            term that expresses the function H.

% Refs     : [Rie16] Riener (2016), Email to Geoff Sutcliffe
% Source   : [TPTP]
% Names    : ntape6-2-with-witness2.tptp [Rie16]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.43 v6.4.0
% Syntax   : Number of formulae    :    5 (   0 unit;   4 type;   0 defn)
%            Number of atoms       :   56 (  10 equality;  19 variable)
%            Maximal formula depth :   14 (   5 average)
%            Number of connectives :   38 (   3   ~;   0   |;   6   &;  26   @)
%                                         (   0 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    6 (   6   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    6 (   4   :;   0   =)
%            Number of variables   :   10 (   0 sgn;   9   !;   1   ?;   0   ^)
%                                         (  10   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments :
%------------------------------------------------------------------------------
thf(n6,type,(
    zero: $i )).

thf(n7,type,(
    s: $i > $i )).

thf(n8,type,(
    ite: $o > $i > $i > $i )).

thf(n9,type,(
    h: $i > $i )).

thf(n10,conjecture,
    ( ( ! [X100: $o,U: $i,V: $i] :
          ( X100
         => ( ( ite @ X100 @ U @ V )
            = U ) )
      & ! [X100: $o,U: $i,V: $i] :
          ( ~ ( X100 )
         => ( ( ite @ X100 @ U @ V )
            = V ) )
      & ! [X: $i] :
          ( ( s @ X )
         != zero )
      & ! [X: $i] :
          ( ( s @ X )
         != X )
      & ! [X: $i] :
          ( ( h @ X )
          = ( ite @ ( X = zero ) @ ( s @ zero )
            @ ( ite
              @ ( X
                = ( s @ zero ) )
              @ zero
              @ ( s @ zero ) ) ) ) )
   => ? [H: $i > $i] :
        ( ( ( H @ zero )
          = ( s @ zero ) )
        & ( ( H @ ( s @ zero ) )
          = zero )
        & ( ( H @ ( s @ ( s @ zero ) ) )
          = ( s @ zero ) ) ) )).

%------------------------------------------------------------------------------
