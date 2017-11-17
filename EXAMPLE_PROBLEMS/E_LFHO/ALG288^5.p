%------------------------------------------------------------------------------
% File     : ALG288^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from PU-LAMBDA-MODEL-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1148 [Bro09]

% Status   : Unknown
% Rating   : 1.00 v4.0.0
% Syntax   : Number of formulae    :    6 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :   73 (   8 equality;  43 variable)
%            Maximal formula depth :   16 (   5 average)
%            Number of connectives :   57 (   1   ~;   0   |;  11   &;  35   @)
%                                         (   2 <=>;   8  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    8 (   8   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   5   :;   0   =)
%            Number of variables   :   20 (   0 sgn;  14   !;   6   ?;   0   ^)
%                                         (  20   :;   0  !>;   0  ?*)
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

thf(cPU_SETR_CTS_pme,conjecture,
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
        ( ? [Xy: a] :
            ( X @ ( cP @ Xy @ Xz ) )
      <=> ? [Xx: a] :
            ( ! [Xx_29: a] :
                ( ! [X0: a > $o] :
                    ( ( ( X0 @ Xx )
                      & ! [Xz0: a] :
                          ( ( X0 @ Xz0 )
                         => ( X0 @ ( cL @ Xz0 ) ) ) )
                   => ? [Xv: a] :
                        ( ( X0 @ Xv )
                        & ( ( cR @ Xv )
                          = Xx_29 ) ) )
               => ( X @ Xx_29 ) )
            & ? [Xy: a] :
              ! [X0: a > $o] :
                ( ( ( X0 @ Xx )
                  & ! [Xz0: a] :
                      ( ( X0 @ Xz0 )
                     => ( X0 @ ( cL @ Xz0 ) ) ) )
               => ? [Xv: a] :
                    ( ( X0 @ Xv )
                    & ( ( cR @ Xv )
                      = ( cP @ Xy @ Xz ) ) ) ) ) ) )).

%------------------------------------------------------------------------------
