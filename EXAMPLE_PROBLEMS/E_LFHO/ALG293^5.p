%------------------------------------------------------------------------------
% File     : ALG293^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from PU-LAMBDA-MODEL-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1208 [Bro09]

% Status   : Unknown
% Rating   : 1.00 v4.0.0
% Syntax   : Number of formulae    :    6 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :  122 (  11 equality;  79 variable)
%            Maximal formula depth :   19 (   6 average)
%            Number of connectives :  100 (   1   ~;   0   |;  19   &;  62   @)
%                                         (   2 <=>;  16  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :   11 (  11   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   5   :;   0   =)
%            Number of variables   :   35 (   0 sgn;  22   !;  13   ?;   0   ^)
%                                         (  35   :;   0  !>;   0  ?*)
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

thf(cL,type,(
    cL: a > a )).

thf(cZ,type,(
    cZ: a )).

thf(cPU_X239_pme,conjecture,
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
            = ( cP @ ( cL @ Xt ) @ ( cR @ Xt ) ) ) )
      & ! [X: a > $o] :
          ( ? [Xt: a] :
              ( ( X @ Xt )
              & ! [Xu: a] :
                  ( ( X @ Xu )
                 => ( X @ ( cL @ Xu ) ) ) )
         => ( X @ cZ ) ) )
   => ! [X: a > $o,Xz: a] :
        ( ? [Xx: a] :
            ( ! [Xx_23: a] :
                ( ! [X0: a > $o] :
                    ( ( ( X0 @ Xx )
                      & ! [Xz0: a] :
                          ( ( X0 @ Xz0 )
                         => ( X0 @ ( cL @ Xz0 ) ) ) )
                   => ? [Xv: a] :
                        ( ( X0 @ Xv )
                        & ( ( cR @ Xv )
                          = Xx_23 ) ) )
               => ? [Xy: a] :
                    ( X @ ( cP @ Xy @ Xx_23 ) ) )
            & ? [Xz_2: a] :
                ( X @ ( cP @ ( cP @ Xx @ Xz ) @ Xz_2 ) ) )
      <=> ? [Xx: a] :
            ( ! [Xx_24: a] :
                ( ! [X0: a > $o] :
                    ( ( ( X0 @ Xx )
                      & ! [Xz0: a] :
                          ( ( X0 @ Xz0 )
                         => ( X0 @ ( cL @ Xz0 ) ) ) )
                   => ? [Xv: a] :
                        ( ( X0 @ Xv )
                        & ( ( cR @ Xv )
                          = Xx_24 ) ) )
               => ( X @ Xx_24 ) )
            & ? [Xx_26: a] :
                ( ! [Xx_25: a] :
                    ( ! [X0: a > $o] :
                        ( ( ( X0 @ Xx_26 )
                          & ! [Xz0: a] :
                              ( ( X0 @ Xz0 )
                             => ( X0 @ ( cL @ Xz0 ) ) ) )
                       => ? [Xv: a] :
                            ( ( X0 @ Xv )
                            & ( ( cR @ Xv )
                              = Xx_25 ) ) )
                   => ? [Xy: a] :
                      ! [X0: a > $o] :
                        ( ( ( X0 @ Xx )
                          & ! [Xz0: a] :
                              ( ( X0 @ Xz0 )
                             => ( X0 @ ( cL @ Xz0 ) ) ) )
                       => ? [Xv: a] :
                            ( ( X0 @ Xv )
                            & ( ( cR @ Xv )
                              = ( cP @ Xy @ Xx_25 ) ) ) ) )
                & ? [Xz_3: a] :
                  ! [X0: a > $o] :
                    ( ( ( X0 @ Xx )
                      & ! [Xz0: a] :
                          ( ( X0 @ Xz0 )
                         => ( X0 @ ( cL @ Xz0 ) ) ) )
                   => ? [Xv: a] :
                        ( ( X0 @ Xv )
                        & ( ( cR @ Xv )
                          = ( cP @ ( cP @ Xx_26 @ Xz ) @ Xz_3 ) ) ) ) ) ) ) )).

%------------------------------------------------------------------------------
