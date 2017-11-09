%------------------------------------------------------------------------------
% File     : GRP001^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Group Theory
% Problem  : TPS problem GRP-COMM2
% Version  : Especial.
% English  : Group is Abelian iff every element has order 2.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0516 [Bro09]
%          : GRP-COMM2 [TPS]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.43 v5.5.0, 0.33 v5.4.0, 0.40 v5.3.0, 0.60 v5.2.0, 0.40 v5.1.0, 0.60 v5.0.0, 0.40 v4.1.0, 0.00 v4.0.0
% Syntax   : Number of formulae    :    3 (   0 unit;   2 type;   0 defn)
%            Number of atoms       :   33 (   5 equality;  16 variable)
%            Maximal formula depth :   13 (   6 average)
%            Number of connectives :   22 (   0   ~;   0   |;   3   &;  18   @)
%                                         (   0 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    2 (   2   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    3 (   2   :;   0   =)
%            Number of variables   :    8 (   0 sgn;   8   !;   0   ?;   0   ^)
%                                         (   8   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%          : 
%------------------------------------------------------------------------------
thf(cP,type,(
    cP: $i > $i > $i )).

thf(e,type,(
    e: $i )).

thf(cGRP_COMM2,conjecture,
    ( ( ! [Xx: $i] :
          (  cP @ e @ Xx 
          = Xx )
      & ! [Xy: $i] :
          (  cP @ Xy @ e 
          = Xy )
      & ! [Xz: $i] :
          (  cP @ Xz @ Xz 
          = e )
      & ! [Xx: $i,Xy: $i,Xz: $i] :
          (  cP @ ( cP @ Xx @ Xy ) @ Xz 
          =  cP @ Xx @ ( cP @ Xy @ Xz ) ) ) 
   => ! [Xa: $i,Xb: $i] :
        (  cP @ Xa @ Xb 
        =  cP @ Xb @ Xa ) ) ).

%------------------------------------------------------------------------------
