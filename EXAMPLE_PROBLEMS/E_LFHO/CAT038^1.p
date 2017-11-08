%------------------------------------------------------------------------------
% File     : CAT038^1 : TPTP v7.0.0. Released v4.1.0.
% Domain   : Syntactic
% Problem  : Swapping function
% Version  : Especial.
% English  : The proposition can be interpreted in concrete categories and
%            asserts the existence of a certain arrow.

% Refs     : [Bro09] Brown E. (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : 

% Status   : Theorem
%          : Without choice : CounterSatisfiable
%          : Without choice and with if-then-else : Theorem
%          : Without choice and with description : Theorem
% Rating   : 1.00 v4.1.0
% Syntax   : Number of formulae    :    3 (   0 unit;   2 type;   0 defn)
%            Number of atoms       :    8 (   2 equality;   2 variable)
%            Maximal formula depth :    5 (   3 average)
%            Number of connectives :    3 (   0   ~;   0   |;   1   &;   2   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    1 (   1   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    3 (   2   :;   0   =)
%            Number of variables   :    1 (   0 sgn;   0   !;   1   ?;   0   ^)
%                                         (   1   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : 
%          : The fragment of simple type theory that restricts equations to
%            base types and disallows lambda abstraction and quantification is
%            decidable. This is an example.
%------------------------------------------------------------------------------
thf(a,type,(
    a: $i )).

thf(b,type,(
    b: $i )).

thf(swap,conjecture,(
    ? [F: $i > $i] :
      ( (  F @ a 
        = b )
      & (  F @ b 
        = a ) ) )).

%------------------------------------------------------------------------------
