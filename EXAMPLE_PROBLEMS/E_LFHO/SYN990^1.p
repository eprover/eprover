%------------------------------------------------------------------------------
% File     : SYN990^1 : TPTP v7.0.0. Released v3.7.0.
% Domain   : Syntactic
% Problem  : Simple test for satisfiability
% Version  : Especial.
% English  : There are two individuals, a and b. Everything is either a or b.
%            There are three functions from individuals to individuals: f,g,h.
%            They are all different.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    :

% Status   : Satisfiable
% Rating   : 0.33 v6.1.0, 0.00 v5.4.0, 0.67 v5.2.0, 1.00 v5.0.0, 0.33 v4.1.0, 0.00 v3.7.0
% Syntax   : Number of formulae    :    9 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :   15 (   5 equality;   2 variable)
%            Maximal formula depth :    4 (   3 average)
%            Number of connectives :    4 (   3   ~;   1   |;   0   &;   0   @)
%                                         (   0 <=>;   0  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    3 (   3   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    6 (   5   :;   0   =)
%            Number of variables   :    1 (   0 sgn;   1   !;   0   ?;   0   ^)
%                                         (   1   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_SAT_EQU_NAR

% Comments : 
%------------------------------------------------------------------------------
thf(a,type,(
    a: $i )).

thf(b,type,(
    b: $i )).

thf(f,type,(
    f: $i > $i )).

thf(g,type,(
    g: $i > $i )).

thf(h,type,(
    h: $i > $i )).

thf(ab,axiom,(
    ! [X: $i] :
      ( ( X = a )
      | ( X = b ) ) )).

thf(fg,axiom,(
    f != g )).

thf(gh,axiom,(
    g != h )).

thf(fh,axiom,(
    f != h )).

%------------------------------------------------------------------------------
