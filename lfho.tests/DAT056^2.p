%------------------------------------------------------------------------------
% File     : DAT056^2 : TPTP v7.0.0. Released v5.4.0.
% Domain   : Data Structures
% Problem  : List operation requiring induction
% Version  : Especial.
% English  :

% Refs     : [Bla12] Blanchette (2012), Email to Geoff Sutcliffe
% Source   : [Bla12]
% Names    : hard.tptp [Bla12]

% Status   : Theorem
% Rating   : 0.88 v7.0.0, 0.86 v6.4.0, 0.83 v6.3.0, 0.80 v6.2.0, 1.00 v5.4.0
% Syntax   : Number of formulae    :   10 (   0 unit;   6 type;   0 defn)
%            Number of atoms       :   37 (   3 equality;  20 variable)
%            Maximal formula depth :   11 (   5 average)
%            Number of connectives :   27 (   0   ~;   0   |;   0   &;  24   @)
%                                         (   0 <=>;   3  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :    5 (   5   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    8 (   6   :;   0   =)
%            Number of variables   :   10 (   0 sgn;  10   !;   0   ?;   0   ^)
%                                         (  10   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_EQU_NAR

% Comments :
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
thf(fact_0_lst_Oinduct,axiom,(
    ! [Lst: lst,P: lst > $o] :
      ( ( P @ nl )
     => ( ! [A: a,Lst2: lst] :
            ( ( P @ Lst2 )
           => ( P @ ( cns @ A @ Lst2 ) ) )
       => ( P @ Lst ) ) ) )).

thf(fact_1p_Osimps_I2_J,axiom,(
    ! [Ys2: lst,Xs: lst,X: a] :
      (  ap @ ( cns @ X @ Xs ) @ Ys2 
      =  cns @ X @ ( ap @ Xs @ Ys2 ) ) ) ).

thf(fact_2p_Osimps_I1_J,axiom,(
    ! [Ys2: lst] :
      ( ap @ nl @ Ys2 
      = Ys2 ) )).

%----Conjectures (1)
thf(conj_0,conjecture,(
    ! [Ys: lst,Zs: lst] :
      (  ap @ xs @ ( ap @ Ys @ Zs ) 
      =  ap @ ( ap @ xs @ Ys ) @ Zs ) ) ).

%------------------------------------------------------------------------------
