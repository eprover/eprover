%------------------------------------------------------------------------------
% File     : NUN023^1 : TPTP v7.0.0. Released v6.4.0.
% Domain   : Number Theory
% Problem  : Function h s.t. h(0) = 1, h(1) = 0, no witness
% Version  : Especial.
% English  : Using an axiomatiztion of if-then-else, find the if-then-else
%            term that expresses the function H.

% Refs     : [Rie16] Riener (2016), Email to Geoff Sutcliffe
% Source   : [TPTP]
% Names    : ntape6-0-without-witness.tptp [Rie16]

% Status   : Theorem
% Rating   : 0.50 v7.0.0, 0.57 v6.4.0
% Syntax   : Number of formulae    :    4 (   0 unit;   3 type;   0 defn)
%            Number of atoms       :   24 (   4 equality;  12 variable)
%            Maximal formula depth :   11 (   5 average)
%            Number of connectives :   16 (   1   ~;   0   |;   2   &;  10   @)
%                                         (   0 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    5 (   5   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    5 (   3   :;   0   =)
%            Number of variables   :    7 (   0 sgn;   6   !;   1   ?;   0   ^)
%                                         (   7   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments :
%------------------------------------------------------------------------------
thf(n5,type,(
    zero: $i )).

thf(n6,type,(
    s: $i > $i )).

thf(n7,type,(
    ite: $o > $i > $i > $i )).

thf(n8,conjecture,
    ( ( ! [X: $o,U: $i,V: $i] :
          ( (X)
         => ( ( ite @ X @ U @ V )
            = U ) )
      & ! [X: $o,U: $i,V: $i] :
          ( ~ ( X )
         => ( ( ite @ X @ U @ V )
            = V ) ) )
   => ? [H: $i > $i] :
        ( (  H @ zero 
          =  s @ zero  )
        & ( H @ ( s @ zero ) 
          = zero ) ) )).

%------------------------------------------------------------------------------
