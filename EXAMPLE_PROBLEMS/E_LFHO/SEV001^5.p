%------------------------------------------------------------------------------
% File     : SEV001^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : Set Theory (Relations)
% Problem  : TPS problem DISTRIB-THM
% Version  : Especial.
% English  : In a lattice, join distributes over meet iff meet distributes 
%            over join.

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_0563 [Bro09]
%          : DISTRIB-THM [TPS]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.71 v6.1.0, 0.43 v5.5.0, 0.67 v5.4.0, 0.80 v5.0.0, 1.00 v4.0.0
% Syntax   : Number of formulae    :    2 (   0 unit;   1 type;   0 defn)
%            Number of atoms       :   86 (  10 equality;  76 variable)
%            Maximal formula depth :   18 (  10 average)
%            Number of connectives :   65 (   0   ~;   0   |;   7   &;  56   @)
%                                         (   1 <=>;   1  =>;   0  <=;   0 <~>)
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
thf(a_type,type,(
    a: $tType )).

thf(cDISTRIB_THM_pme,conjecture,(
    ! [JOIN: a > a > a,MEET: a > a > a] :
      ( ( ! [Xx: a] :
            (  JOIN @ Xx @ Xx 
            = Xx )
        & ! [Xx: a] :
            (  MEET @ Xx @ Xx 
            = Xx )
        & ! [Xx: a,Xy: a,Xz: a] :
            (  JOIN @ ( JOIN @ Xx @ Xy ) @ Xz 
            =  JOIN @ Xx @ ( JOIN @ Xy @ Xz ) ) 
        & ! [Xx: a,Xy: a,Xz: a] :
            (  MEET @ ( MEET @ Xx @ Xy ) @ Xz 
            =  MEET @ Xx @ ( MEET @ Xy @ Xz ) ) 
        & ! [Xx: a,Xy: a] :
            (  JOIN @ Xx @ Xy 
            =  JOIN @ Xy @ Xx ) 
        & ! [Xx: a,Xy: a] :
            (  MEET @ Xx @ Xy 
            =  MEET @ Xy @ Xx ) 
        & ! [Xx: a,Xy: a] :
            (  JOIN @ ( MEET @ Xx @ Xy ) @ Xy 
            = Xy )
        & ! [Xx: a,Xy: a] :
            ( MEET @ ( JOIN @ Xx @ Xy ) @ Xy 
            = Xy ) )
     => ( ! [Xx: a,Xy: a,Xz: a] :
            (  JOIN @ Xx @ ( MEET @ Xy @ Xz ) 
            =  MEET @ ( JOIN @ Xx @ Xy ) @ ( JOIN @ Xx @ Xz ) ) 
      <=> ! [Xx: a,Xy: a,Xz: a] :
            (  MEET @ Xx @ ( JOIN @ Xy @ Xz ) 
            =  JOIN @ ( MEET @ Xx @ Xy ) @ ( MEET @ Xx @ Xz ) ) ) ) ) ).

%------------------------------------------------------------------------------
