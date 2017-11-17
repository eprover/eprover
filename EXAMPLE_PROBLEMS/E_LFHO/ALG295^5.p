%------------------------------------------------------------------------------
% File     : ALG295^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from SEQUENTIAL-PU-ALG-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1160 [Bro09]

% Status   : Unknown
% Rating   : 1.00 v4.0.0
% Syntax   : Number of formulae    :    6 (   0 unit;   5 type;   0 defn)
%            Number of atoms       :   87 (   8 equality;  43 variable)
%            Maximal formula depth :   24 (   6 average)
%            Number of connectives :   71 (   1   ~;   0   |;  10   &;  50   @)
%                                         (   2 <=>;   8  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    7 (   7   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   5   :;   0   =)
%            Number of variables   :   18 (   0 sgn;  14   !;   4   ?;   0   ^)
%                                         (  18   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_UNK_EQU_NAR

% Comments : This problem is from the TPS library. Copyright (c) 2009 The TPS
%            project in the Department of Mathematical Sciences at Carnegie
%            Mellon University. Distributed under the Creative Commons copyleft
%            license: http://creativecommons.org/licenses/by-sa/3.0/
%------------------------------------------------------------------------------
thf(a_type,type,(
    a: $tType )).

thf(cZ,type,(
    cZ: a )).

thf(cP,type,(
    cP: a > a > a )).

thf(cR,type,(
    cR: a > a )).

thf(cL,type,(
    cL: a > a )).

thf(cPU_LEM8_pme,conjecture,
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
   => ( ! [X: a > $o] :
          ( ? [Xt: a] :
              ( ( X @ Xt )
              & ! [Xu: a] :
                  ( ( X @ Xu )
                 => ( X @ ( cL @ Xu ) ) ) )
         => ( X @ cZ ) )
    <=> ! [Xt: a] :
        ? [Xn: a] :
          ( ! [X: a > $o] :
              ( ( ( X @ cZ )
                & ! [Xx: a] :
                    ( ( X @ Xx )
                   => ( X @ ( cP @ Xx @ cZ ) ) ) )
             => ( X @ Xn ) )
          & ! [Xu: a] :
              ( ? [Xb: a,Xu_11: a] :
                  ( ( ( cP @ Xn @ Xu )
                    = ( cP @ Xb @ Xu_11 ) )
                  & ! [X: a > $o] :
                      ( ( ( X @ ( cP @ cZ @ Xt ) )
                        & ! [Xc: a,Xv: a] :
                            ( ( X @ ( cP @ Xc @ Xv ) )
                           => ( ( X @ ( cP @ ( cP @ Xc @ cZ ) @ ( cL @ Xv ) ) )
                              & ( X @ ( cP @ ( cP @ Xc @ ( cP @ cZ @ cZ ) ) @ ( cR @ Xv ) ) ) ) ) )
                     => ( X @ ( cP @ Xb @ Xu_11 ) ) ) )
             => ( Xu = cZ ) ) ) ) )).

%------------------------------------------------------------------------------
