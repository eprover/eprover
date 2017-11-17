%------------------------------------------------------------------------------
% File     : ALG296^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from SEQUENTIAL-PU-ALG-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1191 [Bro09]

% Status   : Unknown
% Rating   : 1.00 v4.0.0
% Syntax   : Number of formulae    :    6 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :  119 (   9 equality;  57 variable)
%            Maximal formula depth :   25 (   6 average)
%            Number of connectives :  101 (   1   ~;   0   |;  13   &;  77   @)
%                                         (   1 <=>;   9  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    7 (   7   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   5   :;   0   =)
%            Number of variables   :   21 (   0 sgn;  16   !;   5   ?;   0   ^)
%                                         (  21   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_UNK_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(cP,type,(
    cP: a > a > a )).

thf(cR,type,(
    cR: a > a )).

thf(cZ,type,(
    cZ: a )).

thf(cL,type,(
    cL: a > a )).

thf(cPU_LEM6_pme,conjecture,
    ( ( ( ( cL @ cZ )
        = cZ )
      & ( ( cR @ cZ )
        = cZ )
      & ! [Xx: a,Xy: a] :
          ( ( cL @ ( cP @ Xx @ Xy ) )
          = Xx )
      & ! [Xx: a,Xy: a] :
          ( ( cR @ ( cP @ Xx @ Xy ) )
          = Xy )
      & ! [Xt: a] :
          ( ( Xt != cZ )
        <=> ( Xt
            = ( cP @ ( cL @ Xt ) @ ( cR @ Xt ) ) ) ) )
   => ! [Xt: a,Xb: a] :
        ( ! [X: a > $o] :
            ( ( ( X @ cZ )
              & ! [Xx: a] :
                  ( ( X @ Xx )
                 => ( ( X @ ( cP @ Xx @ cZ ) )
                    & ( X @ ( cP @ Xx @ ( cP @ cZ @ cZ ) ) ) ) ) )
           => ( X @ Xb ) )
       => ? [Xu: a] :
            ( ? [Xb_0: a,Xu_0: a] :
                ( ( ( cP @ Xb @ Xu )
                  = ( cP @ Xb_0 @ Xu_0 ) )
                & ! [X: a > $o] :
                    ( ( ( X @ ( cP @ cZ @ Xt ) )
                      & ! [Xc: a,Xv: a] :
                          ( ( X @ ( cP @ Xc @ Xv ) )
                         => ( ( X @ ( cP @ ( cP @ Xc @ cZ ) @ ( cL @ Xv ) ) )
                            & ( X @ ( cP @ ( cP @ Xc @ ( cP @ cZ @ cZ ) ) @ ( cR @ Xv ) ) ) ) ) )
                   => ( X @ ( cP @ Xb_0 @ Xu_0 ) ) ) )
            & ! [Xv: a] :
                ( ? [Xb_1: a,Xu0: a] :
                    ( ( ( cP @ Xb @ Xv )
                      = ( cP @ Xb_1 @ Xu0 ) )
                    & ! [X: a > $o] :
                        ( ( ( X @ ( cP @ cZ @ Xt ) )
                          & ! [Xc: a,Xv0: a] :
                              ( ( X @ ( cP @ Xc @ Xv0 ) )
                             => ( ( X @ ( cP @ ( cP @ Xc @ cZ ) @ ( cL @ Xv0 ) ) )
                                & ( X @ ( cP @ ( cP @ Xc @ ( cP @ cZ @ cZ ) ) @ ( cR @ Xv0 ) ) ) ) ) )
                       => ( X @ ( cP @ Xb_1 @ Xu0 ) ) ) )
               => ( Xu = Xv ) ) ) ) )).

%------------------------------------------------------------------------------
