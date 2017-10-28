%------------------------------------------------------------------------------
% File     : COM002_1 : TPTP v7.0.0. Released v5.0.0.
% Domain   : Computing Theory
% Problem  : A program correctness theorem.
% Version  : Especial.
% English  : A computing state space, with eight states - P1 to P8.
%            P1 leads to P3 via P2. There is a branch at P3 such that the
%            following state is either P4 or P6. P6 leads to P8, which has
%            a loop back to P3, while P4 leads to termination. The problem
%            is to show that there is a loop in the computation, passing
%            through P3.

% Refs     : [RR+72] Reboh et al. (1972), Study of automatic theorem provin
% Source   : [TPTP]
% Names    :

% Status   : Theorem
% Rating   : 0.00 v5.1.0, 0.33 v5.0.0
% Syntax   : Number of formulae    :   51 (  15 unit;  32 type)
%            Number of atoms       :   25 (   0 equality)
%            Maximal formula depth :    6 (   2 average)
%            Number of connectives :    6 (   0   ~;   0   |;   2   &)
%                                         (   0 <=>;   4  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :   19 (  10   >;   9   *;   0   +;   0  <<)
%            Number of predicates  :   38 (  34 propositional; 0-2 arity)
%            Number of functors    :   22 (  16 constant; 0-2 arity)
%            Number of variables   :   11 (   0 sgn;  11   !;   0   ?)
%                                         (  11   :;   0  !>;   0  ?*)
%            Maximal term depth    :    3 (   1 average)
% SPC      : TF0_THM_NEQ_NAR

% Comments : I suspect this problem was originally by R.M. Burstall.
%------------------------------------------------------------------------------
tff(state_type,type,(
    state: $tType )).

tff(label_type,type,(
    label: $tType )).

tff(statement_type,type,(
    statement: $tType )).

tff(register_type,type,(
    register: $tType )).

tff(number_type,type,(
    number: $tType )).

tff(boolean_type,type,(
    boolean: $tType )).

tff(p1_type,type,(
    p1: state )).

tff(p2_type,type,(
    p2: state )).

tff(p3_type,type,(
    p3: state )).

tff(p4_type,type,(
    p4: state )).

tff(p5_type,type,(
    p5: state )).

tff(p6_type,type,(
    p6: state )).

tff(p7_type,type,(
    p7: state )).

tff(p8_type,type,(
    p8: state )).

tff(n_type,type,(
    n: number )).

tff(n0_type,type,(
    n0: number )).

tff(n1_type,type,(
    n1: number )).

tff(n2_type,type,(
    n2: number )).

tff(register_j_type,type,(
    register_j: register )).

tff(register_k_type,type,(
    register_k: register )).

tff(out_type,type,(
    out: label )).

tff(loop_type,type,(
    loop: label )).

tff(equal_function_type,type,(
    equal_function: ( register * number ) > boolean )).

tff(assign_type,type,(
    assign: ( register * number ) > statement )).

tff(goto_type,type,(
    goto: label > statement )).

tff(ifthen_type,type,(
    ifthen: ( boolean * state ) > statement )).

tff(plus_type,type,(
    plus: ( register * number ) > number )).

tff(times_type,type,(
    times: ( number * register ) > number )).

tff(follows_type,type,(
    follows: ( state * state ) > $o )).

tff(succeeds_type,type,(
    succeeds: ( state * state ) > $o )).

tff(labels_type,type,(
    labels: ( label * state ) > $o )).

tff(has_type,type,(
    has: ( state * statement ) > $o )).

tff(direct_success,axiom,(
    ! [Start_state: state,Goal_state: state] :
      ( follows(Goal_state,Start_state)
     => succeeds(Goal_state,Start_state) ) )).

tff(transitivity_of_success,axiom,(
    ! [Start_state: state,Intermediate_state: state,Goal_state: state] :
      ( ( succeeds(Goal_state,Intermediate_state)
        & succeeds(Intermediate_state,Start_state) )
     => succeeds(Goal_state,Start_state) ) )).

tff(goto_success,axiom,(
    ! [Goal_state: state,Label: label,Start_state: state] :
      ( ( has(Start_state,goto(Label))
        & labels(Label,Goal_state) )
     => succeeds(Goal_state,Start_state) ) )).

tff(conditional_success,axiom,(
    ! [Goal_state: state,Condition: boolean,Start_state: state] :
      ( has(Start_state,ifthen(Condition,Goal_state))
     => succeeds(Goal_state,Start_state) ) )).

tff(state_1,hypothesis,(
    has(p1,assign(register_j,n0)) )).

tff(transition_1_to_2,hypothesis,(
    follows(p2,p1) )).

tff(state_2,hypothesis,(
    has(p2,assign(register_k,n1)) )).

tff(label_state_3,hypothesis,(
    labels(loop,p3) )).

tff(transition_2_to_3,hypothesis,(
    follows(p3,p2) )).

tff(state_3,hypothesis,(
    has(p3,ifthen(equal_function(register_j,n),p4)) )).

tff(state_4,hypothesis,(
    has(p4,goto(out)) )).

tff(transition_4_to_5,hypothesis,(
    follows(p5,p4) )).

tff(transition_3_to_6,hypothesis,(
    follows(p6,p3) )).

tff(state_6,hypothesis,(
    has(p6,assign(register_k,times(n2,register_k))) )).

tff(transition_6_to_7,hypothesis,(
    follows(p7,p6) )).

tff(state_7,hypothesis,(
    has(p7,assign(register_j,plus(register_j,n1))) )).

tff(transition_7_to_8,hypothesis,(
    follows(p8,p7) )).

tff(state_8,hypothesis,(
    has(p8,goto(loop)) )).

tff(prove_there_is_a_loop_through_p3,conjecture,(
    succeeds(p3,p3) )).

%------------------------------------------------------------------------------
