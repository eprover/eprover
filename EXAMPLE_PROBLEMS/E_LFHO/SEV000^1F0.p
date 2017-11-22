%------------------------------------------------------------------------------
%------------------------------------------------------------------------------
% File     : SEV000^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Set Theory (Relations)
% Problem  : TPS problem MODULAR-EQUIV-THM
% Version  : Especial.
% English  : The equivalence of two definitions of modularity.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0562 [Bro09]
%          : MODULAR-EQUIV-THM [TPS]

% Status   : Theorem
% Rating   : 0.25 v7.0.0, 0.29 v6.4.0, 0.33 v6.3.0, 0.40 v6.2.0, 0.43 v6.1.0, 0.29 v5.5.0, 0.50 v5.4.0, 0.60 v5.1.0, 0.80 v5.0.0, 1.00 v4.1.0, 0.67 v4.0.1, 1.00 v4.0.0
% Syntax   : Number of formulae    :    2 (   0 unit;   1 type;   0 defn)
%            Number of atoms       :   91 (  11 equality;  80 variable)
%            Maximal formula depth :   18 (  10 average)
%            Number of connectives :   68 (   0   ~;   0   |;   7   &;  58   @)
%                                         (   1 <=>;   2  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    2 (   1   :;   0   =)
%            Number of variables   :   24 (   0 sgn;  24   !;   0   ?;   0   ^)
%                                         (  24   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : Polymorphic definitions expanded.
%          : 
%------------------------------------------------------------------------------
tff(a_type,type,(
    a: $tType )).
tff(joinType, type, join : (a*a) > a).
tff(meetType, type, meet : (a*a) > a).

tff(cMODULAR_EQUIV_THM_pme,negated_conjecture,(
      ~ ( ( ! [Xx: a] :
            (  join(Xx,Xx)
            = Xx )
        & ! [Xx: a] :
            (  meet(Xx,Xx)
            = Xx )
        & ! [Xx: a,Xy: a,Xz: a] :
            (  join(join( Xx , Xy ) ,Xz )
            =  join(Xx, join(Xy, Xz ) ) ) 
        & ! [Xx: a,Xy: a,Xz: a] :
            (  meet ( meet(Xx, Xy), Xz )
            =  meet (Xx, meet( Xy , Xz ) ) )
        & ! [Xx: a,Xy: a] :
            (  join (Xx, Xy)
            =  join (Xy, Xx ) )
        & ! [Xx: a,Xy: a] :
            (  meet (Xx,Xy)
            =  meet (Xy,Xx) )
        & ! [Xx: a,Xy: a] :
            (  join(meet(Xx , Xy ),  Xy )
            = Xy )
        & ! [Xx: a,Xy: a] :
            (  meet( join (Xx, Xy ), Xy )
            = Xy ) )
     => ( ! [Xx: a,Xy: a,Xz: a] :
            ( (  join (Xx , Xz )
              = Xz )
           => (  join (Xx, meet (Xy , Xz) )
              =  meet (join(Xx, Xy ) , Xz ) ) )
      <=> ! [Xx: a,Xy: a,Xz: a] :
            (  join (Xx, meet(Xy, join (Xx, Xz) ) )
            =  meet( join (Xx, Xy ), join (Xx , Xz ) ) ) ) ) )).

%------------------------------------------------------------------------------
