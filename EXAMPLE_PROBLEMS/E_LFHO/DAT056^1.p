%------------------------------------------------------------------------------
% File     : DAT056^1 : TPTP v7.0.0. Released v5.4.0.
% Domain   : Data Structures
% Problem  : List operation requiring induction
% Version  : Especial.
% English  :

% Refs     : [Bla12] Blanchette (2012), Email to Geoff Sutcliffe
% Source   : [Bla12]
% Names    : easy.tptp [Bla12]

% Status   : Theorem
% Rating   : 0.38 v7.0.0, 0.43 v6.4.0, 0.50 v6.3.0, 0.60 v6.2.0, 0.43 v5.5.0, 0.33 v5.4.0
% Syntax   : Number of formulae    :   10 (   0 unit;   6 type;   0 defn)
%            Number of atoms       :   75 (   7 equality;  36 variable)
%            Maximal formula depth :   16 (   5 average)
%            Number of connectives :   57 (   0   ~;   0   |;   0   &;  54   @)
%                                         (   0 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    4 (   4   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    7 (   6   :;   0   =)
%            Number of variables   :   17 (   0 sgn;  17   !;   0   ?;   0   ^)
%                                         (  17   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments : Induction principle is preinstantiated with the conjecture.
%------------------------------------------------------------------------------
%----Should-be-implicit typings (2)
thf(ty_n_tc__Foo__Olst_It__J,type,(
    lst: $tType )).

thf(ty_n_t_,type,(
    a: $tType )).

%----Explicit typings (4)
thf(sy_c_Foo_Oap_001t_,type,(
    ap: lst > lst > lst )).

thf(sy_c_Foo_Olst_OCns_001t_,type,(
    cns: a > lst > lst )).

thf(sy_c_Foo_Olst_ONl_001t_,type,(
    nl: lst )).

thf(sy_v_xs,type,(
    xs: lst )).

%----Relevant facts (3)
thf(fact_0_lst_Oinduct_091where_AP_A_061_A_C_Fxs_O_AALL_Ays_Azs_O_Aap_Axs_A_Iap_Ays_Azs_J_A_061_Aap_A_Iap_Axs_Ays_J_Azs_C_093,axiom,(
    ! [Lst: lst] :
      ( ! [Ys: lst,Zs: lst] :
          ( ( ap @ nl @ ( ap @ Ys @ Zs ) )
          = ( ap @ ( ap @ nl @ Ys ) @ Zs ) )
     => ( ! [A: a,Lst2: lst] :
            ( ! [Ys3: lst,Zs2: lst] :
                ( ( ap @ Lst2 @ ( ap @ Ys3 @ Zs2 ) )
                = ( ap @ ( ap @ Lst2 @ Ys3 ) @ Zs2 ) )
           => ! [Ys: lst,Zs: lst] :
                ( ( ap @ ( cns @ A @ Lst2 ) @ ( ap @ Ys @ Zs ) )
                = ( ap @ ( ap @ ( cns @ A @ Lst2 ) @ Ys ) @ Zs ) ) )
       => ! [Ys3: lst,Zs2: lst] :
            ( ( ap @ Lst @ ( ap @ Ys3 @ Zs2 ) )
            = ( ap @ ( ap @ Lst @ Ys3 ) @ Zs2 ) ) ) ) )).

thf(fact_1p_Osimps_I2_J,axiom,(
    ! [Ys2: lst,Xs: lst,X: a] :
      ( ( ap @ ( cns @ X @ Xs ) @ Ys2 )
      = ( cns @ X @ ( ap @ Xs @ Ys2 ) ) ) )).

thf(fact_2p_Osimps_I1_J,axiom,(
    ! [Ys2: lst] :
      ( ( ap @ nl @ Ys2 )
      = Ys2 ) )).

%----Conjectures (1)
thf(conj_0,conjecture,(
    ! [Ys: lst,Zs: lst] :
      ( ( ap @ xs @ ( ap @ Ys @ Zs ) )
      = ( ap @ ( ap @ xs @ Ys ) @ Zs ) ) )).

%------------------------------------------------------------------------------
