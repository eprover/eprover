%------------------------------------------------------------------------------
% File     : ALG297^5 : TPTP v7.0.0. Released v4.0.0.
% Domain   : General Algebra (Domain theory)
% Problem  : TPS problem from S-SEQ-THMS
% Version  : Especial.
% English  :

% Refs     : [Bro09] Brown (2009), Email to Geoff Sutcliffe
% Source   : [Bro09]
% Names    : tps_1185 [Bro09]

% Status   : Unknown
% Rating   : 1.00 v4.0.0
% Syntax   : Number of formulae    :    7 (   0 unit;   6 type;   0 defn)
%            Number of atoms       :  105 (   7 equality;  56 variable)
%            Maximal formula depth :   24 (   6 average)
%            Number of connectives :   91 (   1   ~;   0   |;  13   &;  64   @)
%                                         (   1 <=>;  12  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    8 (   8   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    8 (   6   :;   0   =)
%            Number of variables   :   21 (   0 sgn;  19   !;   2   ?;   0   ^)
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

thf(cZ,type,(
    cZ: a )).

thf(cR,type,(
    cR: a > a )).

thf(cL,type,(
    cL: a > a )).

thf(t,type,(
    t: a )).

thf(cPU_LEM7_pme,conjecture,
    ( ( (  cL @ cZ 
        = cZ )
      & (  cR @ cZ 
        = cZ )
      & ! [Xx: a,Xy: a] :
          (  cL @ ( cP @ Xx @ Xy ) 
          = Xx )
      & ! [Xx: a,Xy: a] :
          (  cR @ ( cP @ Xx @ Xy ) 
          = Xy )
      & ! [Xt0: a] :
          ( ( Xt0 != cZ )
        <=> ( Xt0
            =  cP @ ( cL @ Xt0 ) @ ( cR @ Xt0 ) ) ) ) 
   => ! [Xs: a] :
        ( ! [X: a > $o] :
            ( (  ( X @ cZ )
              & ! [Xx: a,Xy: a] :
                  ( (  ( X @ Xx )
                    &  ( X @ Xy )  )
                 =>  X @ ( cP @ Xx @ Xy ) ) ) 
           =>  X @ Xs  )
       => ! [Xb: a] :
            ( ! [X: a > $o] :
                ( (  ( X @ cZ )
                  & ! [Xx: a] :
                      (  ( X @ Xx )
                     => (  ( X @ ( cP @ Xx @ cZ ) )
                        &  ( X @ ( cP @ Xx @ ( cP @ cZ @ cZ ) ) ) ) ) )
               =>  ( X @ Xb )  )
           => ! [Xu: a] :
                ( ? [Xb_9: a,Xu_13: a] :
                    ( (  cP @ Xb @ Xu 
                      =  cP @ Xb_9 @ Xu_13 ) 
                    & ! [X: a > $o] :
                        ( (  ( X @ ( cP @ cZ @ t ) )
                          & ! [Xc: a,Xv: a] :
                              ( ( X @ ( cP @ Xc @ Xv ) )
                             => (  ( X @ ( cP @ ( cP @ Xc @ cZ ) @ ( cL @ Xv ) ) )
                                &  ( X @ ( cP @ ( cP @ Xc @ ( cP @ cZ @ cZ ) ) @ ( cR @ Xv ) ) ) ) ) )
                       =>  ( X @ ( cP @ Xb_9 @ Xu_13 ) ) ) )
               => ! [X: a > $o] :
                    ( ( ( X @ cZ )
                      & ! [Xx: a,Xy: a] :
                          ( (  ( X @ Xx )
                            &  ( X @ Xy ) )
                         =>  ( X @ ( cP @ Xx @ Xy ) ) ) )
                   =>  ( X @ Xu )  ) ) ) ) )).

%------------------------------------------------------------------------------
