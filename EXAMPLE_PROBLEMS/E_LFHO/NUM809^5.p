%------------------------------------------------------------------------------
% File     : NUM809^5 : TPTP v7.0.0. Bugfixed v5.2.0.
% Domain   : Number Theory (Induction on naturals)
% Problem  : TPS problem THM130
% Version  : Especial.
% English  : Induction theorem in which the conclusion is weaker than the 
%            statement which must be proved by induction.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0235 [Bro09]
%          : THM130 [TPS]

% Status   : Theorem
% Rating   : 0.62 v7.0.0, 0.57 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.57 v5.5.0, 0.67 v5.4.0, 0.40 v5.2.0
% Syntax   : Number of formulae    :    6 (   0 unit;   4 type;   1 defn)
%            Number of atoms       :   26 (   1 equality;  13 variable)
%            Maximal formula depth :   10 (   5 average)
%            Number of connectives :   22 (   0   ~;   0   |;   3   &;  15   @)
%                                         (   0 <=>;   4  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    6 (   4   :;   0   =)
%            Number of variables   :    7 (   0 sgn;   6   !;   1   ?;   0   ^)
%                                         (   7   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
% Bugfixes : v5.2.0 - Added missing type declarations.
%------------------------------------------------------------------------------
thf(c0_type,type,(
    c0: $i )).

thf(cS_type,type,(
    cS: $i > $i )).

thf(r_type,type,(
    r: $i > $i > $o )).

thf(cIND_type,type,(
    cIND: $o )).

thf(cTHM130,conjecture,
    ( ( ( ! [Xp: $i > $o] :
          ( ( ( Xp @ c0 )
            & ! [Xx: $i] :
                ( ( Xp @ Xx )
               => ( Xp @ ( cS @ Xx ) ) ) )
         => ! [Xx: $i] :
              ( Xp @ Xx ) ) )
      & ( r @ c0 @ c0 )
      & ! [Xx: $i,Xy: $i] :
          ( ( r @ Xx @ Xy )
         => ( r @ ( cS @ Xx ) @ ( cS @ Xy ) ) ) )
   => ! [Xx: $i] :
      ? [Xy: $i] :
        ( r @ Xx @ Xy ) )).

%------------------------------------------------------------------------------
