%------------------------------------------------------------------------------
% File     : PUZ136^1 : TPTP v7.0.0. Released v5.2.0.
% Domain   : Puzzles
% Problem  : Under two assumptions there are at least two individuals.
% Version  : Especial.
% English  :

% Refs     : [Bro11] Brown (2011), Email to Geoff Sutcliffe
% Source   : [Bro11]
% Names    : 

% Status   : Theorem
% Rating   : 0.14 v7.0.0, 0.12 v6.4.0, 0.14 v6.3.0, 0.17 v6.0.0, 0.00 v5.2.0
% Syntax   : Number of formulae    :    8 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :   10 (   0 equality;   4 variable)
%            Maximal formula depth :    7 (   3 average)
%            Number of connectives :    9 (   2   ~;   0   |;   1   &;   6   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    3 (   3   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   5   :;   0   =)
%            Number of variables   :    3 (   0 sgn;   0   !;   3   ?;   0   ^)
%                                         (   3   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : The conclusion is given using a higher order quantifier.
%            Satallax tends to find solutions by instantiating for P twice.
%            A solution instantiating it once is to take P x := (x = horus), 
%            X := horus. Instantiating Y three times gives you that everyone 
%            is the same as horus, so the two axioms conflict. This problem 
%            is a simplification of CSR138^1.
%------------------------------------------------------------------------------
thf(parent,type,(
    parent: $i > $i > $o )).

thf(kronus,type,(
    kronus: $i )).

thf(zeus,type,(
    zeus: $i )).

thf(ax1,axiom,
    ( parent @ kronus @ zeus )).

thf(sutekh,type,(
    sutekh: $i )).

thf(horus,type,(
    horus: $i )).

thf(ax2,axiom,(
    ~ ( parent @ sutekh @ horus ) )).

thf(hotwo,question,(
    ? [P: $i > $o,X: $i,Y: $i] :
      ( ( P @ X )
      & ~ ( P @ Y ) ) )).

%------------------------------------------------------------------------------
