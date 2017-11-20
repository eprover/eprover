%------------------------------------------------------------------------------
% File     : ALG291^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from PU-LAMBDA-MODEL-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1197 [Bro09]

% Status   : Unknown
% Rating   : 1.00 v4.0.0
% Syntax   : Number of formulae    :    2 (   0 unit;   1 type;   0 defn)
%            Number of atoms       :  103 (  10 equality;  93 variable)
%            Maximal formula depth :   23 (  12 average)
%            Number of connectives :   83 (   1   ~;   0   |;  17   &;  49   @)
%                                         (   2 <=>;  14  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :   11 (  11   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    3 (   1   :;   0   =)
%            Number of variables   :   33 (   0 sgn;  25   !;   8   ?;   0   ^)
%                                         (  33   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_UNK_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(cPU_X238B_pme,conjecture,(
    ! [Z: a,P: a > a > a,L: a > a,R: a > a,X: a > $o] :
      ( ( (  L @ Z 
          = Z )
        & (  R @ Z 
          = Z )
        & ! [Xx: a,Xy: a] :
            (  L @ ( P @ Xx @ Xy ) 
            = Xx )
        & ! [Xx: a,Xy: a] :
            (  R @ ( P @ Xx @ Xy ) 
            = Xy )
        & ! [Xt: a] :
            ( ( Xt != Z )
          <=> ( (Xt)
              = ( P @ ( L @ Xt ) @ ( R @ Xt ) ) ) )
        & ! [X0: a > $o] :
            ( ? [Xt: a] :
                ( ( X0 @ Xt )
                & ! [Xu: a] :
                    ( ( X0 @ Xu )
                   => ( X0 @ ( L @ Xu ) ) ) )
           => ( X0 @ Z ) ) )
     => ! [X_0: a > $o,Xz: a] :
          ( ? [Xx: a] :
              ( ! [Xx_9: a] :
                  ( ! [X0: a > $o] :
                      ( ( ( X0 @ Xx )
                        & ! [Xz0: a] :
                            ( ( X0 @ Xz0 )
                           => ( X0 @ ( L @ Xz0 ) ) ) )
                     => ? [Xv: a] :
                          ( ( X0 @ Xv )
                          & (  R @ Xv 
                            = Xx_9 ) ) )
                 => ( X @ Xx_9 ) )
              & ( X_0 @ ( P @ Xx @ Xz ) ) )
        <=> ? [Xx: a] :
              ( ! [Xx_10: a] :
                  ( ! [X0: a > $o] :
                      ( ( ( X0 @ Xx )
                        & ! [Xz0: a] :
                            ( ( X0 @ Xz0 )
                           => ( X0 @ ( L @ Xz0 ) ) ) )
                     => ? [Xv: a] :
                          ( ( X0 @ Xv )
                          & (  R @ Xv 
                            = Xx_10 ) ) )
                 => ( X_0 @ Xx_10 ) )
              & ? [Xx_12: a] :
                  ( ! [Xx_11: a] :
                      ( ! [X0: a > $o] :
                          ( ( ( X0 @ Xx_12 )
                            & ! [Xz0: a] :
                                ( ( X0 @ Xz0 )
                               => ( X0 @ ( L @ Xz0 ) ) ) )
                         => ? [Xv: a] :
                              ( ( X0 @ Xv )
                              & (  R @ Xv 
                                = Xx_11 ) ) )
                     => ( X @ Xx_11 ) )
                  & ! [X0: a > $o] :
                      ( ( ( X0 @ Xx )
                        & ! [Xz0: a] :
                            ( ( X0 @ Xz0 )
                           => ( X0 @ ( L @ Xz0 ) ) ) )
                     => ? [Xv: a] :
                          ( ( X0 @ Xv )
                          & ( ( R @ Xv )
                            = ( P @ Xx_12 @ Xz ) ) ) ) ) ) ) ) )).

%------------------------------------------------------------------------------
