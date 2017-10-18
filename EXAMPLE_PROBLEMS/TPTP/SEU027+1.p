%------------------------------------------------------------------------------
% File     : SEU027+1 : TPTP v6.4.0. Released v3.2.0.
% Domain   : Set theory
% Problem  : Functions and their basic properties, theorem 60
% Version  : [Urb06] axioms : Especial.
% English  :

% Refs     : [Byl90] Bylinski (1990), Functions and Their Basic Properties
%          : [Urb06] Urban (2006), Email to G. Sutcliffe
% Source   : [Urb06]
% Names    : funct_1__t60_funct_1 [Urb06]

% Status   : Theorem
% Rating   : 0.30 v6.4.0, 0.27 v6.3.0, 0.38 v6.2.0, 0.48 v6.1.0, 0.60 v6.0.0, 0.52 v5.5.0, 0.63 v5.4.0, 0.64 v5.3.0, 0.70 v5.2.0, 0.60 v5.1.0, 0.62 v5.0.0, 0.58 v4.1.0, 0.57 v4.0.0, 0.58 v3.7.0, 0.50 v3.5.0, 0.58 v3.4.0, 0.63 v3.3.0, 0.57 v3.2.0
% Syntax   : Number of formulae    :   37 (   6 unit)
%            Number of atoms       :  119 (  18 equality)
%            Maximal formula depth :   13 (   5 average)
%            Number of connectives :   96 (  14 ~  ;   1  |;  48  &)
%                                         (   5 <=>;  28 =>;   0 <=)
%                                         (   0 <~>;   0 ~|;   0 ~&)
%            Number of predicates  :    9 (   0 propositional; 1-2 arity)
%            Number of functors    :    6 (   1 constant; 0-2 arity)
%            Number of variables   :   59 (   1 singleton;  47 !;  12 ?)
%            Maximal term depth    :    2 (   1 average)
% SPC      : FOF_THM_RFO_SEQ

% Comments : Translated by MPTP 0.2 from the original problem in the Mizar
%            library, www.mizar.org
%------------------------------------------------------------------------------
fof(antisymmetry_r2_hidden,axiom,(
    ! [A,B] :
      ( in(A,B)
     => ~ in(B,A) ) )).

fof(cc1_funct_1,axiom,(
    ! [A] :
      ( empty(A)
     => function(A) ) )).

fof(cc1_relat_1,axiom,(
    ! [A] :
      ( empty(A)
     => relation(A) ) )).

fof(cc2_funct_1,axiom,(
    ! [A] :
      ( ( relation(A)
        & empty(A)
        & function(A) )
     => ( relation(A)
        & function(A)
        & one_to_one(A) ) ) )).

fof(d5_funct_1,axiom,(
    ! [A] :
      ( ( relation(A)
        & function(A) )
     => ! [B] :
          ( B = relation_rng(A)
        <=> ! [C] :
              ( in(C,B)
            <=> ? [D] :
                  ( in(D,relation_dom(A))
                  & C = apply(A,D) ) ) ) ) )).

fof(dt_k2_funct_1,axiom,(
    ! [A] :
      ( ( relation(A)
        & function(A) )
     => ( relation(function_inverse(A))
        & function(function_inverse(A)) ) ) )).

fof(existence_m1_subset_1,axiom,(
    ! [A] :
    ? [B] : element(B,A) )).

fof(fc12_relat_1,axiom,
    ( empty(empty_set)
    & relation(empty_set)
    & relation_empty_yielding(empty_set) )).

fof(fc1_subset_1,axiom,(
    ! [A] : ~ empty(powerset(A)) )).

fof(fc1_xboole_0,axiom,(
    empty(empty_set) )).

fof(fc4_relat_1,axiom,
    ( empty(empty_set)
    & relation(empty_set) )).

fof(fc5_relat_1,axiom,(
    ! [A] :
      ( ( ~ empty(A)
        & relation(A) )
     => ~ empty(relation_dom(A)) ) )).

fof(fc6_relat_1,axiom,(
    ! [A] :
      ( ( ~ empty(A)
        & relation(A) )
     => ~ empty(relation_rng(A)) ) )).

fof(fc7_relat_1,axiom,(
    ! [A] :
      ( empty(A)
     => ( empty(relation_dom(A))
        & relation(relation_dom(A)) ) ) )).

fof(fc8_relat_1,axiom,(
    ! [A] :
      ( empty(A)
     => ( empty(relation_rng(A))
        & relation(relation_rng(A)) ) ) )).

fof(rc1_funct_1,axiom,(
    ? [A] :
      ( relation(A)
      & function(A) ) )).

fof(rc1_relat_1,axiom,(
    ? [A] :
      ( empty(A)
      & relation(A) ) )).

fof(rc1_subset_1,axiom,(
    ! [A] :
      ( ~ empty(A)
     => ? [B] :
          ( element(B,powerset(A))
          & ~ empty(B) ) ) )).

fof(rc1_xboole_0,axiom,(
    ? [A] : empty(A) )).

fof(rc2_funct_1,axiom,(
    ? [A] :
      ( relation(A)
      & empty(A)
      & function(A) ) )).

fof(rc2_relat_1,axiom,(
    ? [A] :
      ( ~ empty(A)
      & relation(A) ) )).

fof(rc2_subset_1,axiom,(
    ! [A] :
    ? [B] :
      ( element(B,powerset(A))
      & empty(B) ) )).

fof(rc2_xboole_0,axiom,(
    ? [A] : ~ empty(A) )).

fof(rc3_funct_1,axiom,(
    ? [A] :
      ( relation(A)
      & function(A)
      & one_to_one(A) ) )).

fof(rc3_relat_1,axiom,(
    ? [A] :
      ( relation(A)
      & relation_empty_yielding(A) ) )).

fof(reflexivity_r1_tarski,axiom,(
    ! [A,B] : subset(A,A) )).

fof(t1_subset,axiom,(
    ! [A,B] :
      ( in(A,B)
     => element(A,B) ) )).

fof(t2_subset,axiom,(
    ! [A,B] :
      ( element(A,B)
     => ( empty(B)
        | in(A,B) ) ) )).

fof(t3_subset,axiom,(
    ! [A,B] :
      ( element(A,powerset(B))
    <=> subset(A,B) ) )).

fof(t4_subset,axiom,(
    ! [A,B,C] :
      ( ( in(A,B)
        & element(B,powerset(C)) )
     => element(A,C) ) )).

fof(t54_funct_1,axiom,(
    ! [A] :
      ( ( relation(A)
        & function(A) )
     => ( one_to_one(A)
       => ! [B] :
            ( ( relation(B)
              & function(B) )
           => ( B = function_inverse(A)
            <=> ( relation_dom(B) = relation_rng(A)
                & ! [C,D] :
                    ( ( ( in(C,relation_rng(A))
                        & D = apply(B,C) )
                     => ( in(D,relation_dom(A))
                        & C = apply(A,D) ) )
                    & ( ( in(D,relation_dom(A))
                        & C = apply(A,D) )
                     => ( in(C,relation_rng(A))
                        & D = apply(B,C) ) ) ) ) ) ) ) ) )).

fof(t5_subset,axiom,(
    ! [A,B,C] :
      ~ ( in(A,B)
        & element(B,powerset(C))
        & empty(C) ) )).

fof(t60_funct_1,conjecture,(
    ! [A] :
      ( ( relation(A)
        & function(A) )
     => ! [B] :
          ( ( relation(B)
            & function(B) )
         => ( ( one_to_one(A)
              & relation_dom(A) = relation_rng(B)
              & relation_rng(A) = relation_dom(B)
              & ! [C,D] :
                  ( ( in(C,relation_dom(A))
                    & in(D,relation_dom(B)) )
                 => ( apply(A,C) = D
                  <=> apply(B,D) = C ) ) )
           => B = function_inverse(A) ) ) ) )).

fof(t6_boole,axiom,(
    ! [A] :
      ( empty(A)
     => A = empty_set ) )).

fof(t7_boole,axiom,(
    ! [A,B] :
      ~ ( in(A,B)
        & empty(B) ) )).

fof(t8_boole,axiom,(
    ! [A,B] :
      ~ ( empty(A)
        & A != B
        & empty(B) ) )).

fof(t9_funct_1,axiom,(
    ! [A] :
      ( ( relation(A)
        & function(A) )
     => ! [B] :
          ( ( relation(B)
            & function(B) )
         => ( ( relation_dom(A) = relation_dom(B)
              & ! [C] :
                  ( in(C,relation_dom(A))
                 => apply(A,C) = apply(B,C) ) )
           => A = B ) ) ) )).
%------------------------------------------------------------------------------
