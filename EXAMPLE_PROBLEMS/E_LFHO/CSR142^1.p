%------------------------------------------------------------------------------
% File     : CSR142^1 : TPTP v7.0.0. Released v4.1.0.
% Domain   : Commonsense Reasoning
% Problem  : Who is the husband of Corina?
% Version  : Especial.
% English  : Corina is the wife of Chris. Who is the husband of Corina?

% Refs     : [Ben10] Benzmueller (2010), Email to Geoff Sutcliffe
% Source   : [Ben10]
% Names    : ex_1.tq_SUMO_handselected [Ben10]

% Status   : Theorem
% Rating   : 0.00 v6.1.0, 0.50 v6.0.0, 0.00 v5.3.0, 0.25 v5.2.0, 0.00 v5.1.0, 0.25 v5.0.0, 0.00 v4.1.0
% Syntax   : Number of formulae    :   10 (   0 unit;   6 type;   0 defn)
%            Number of atoms       :   18 (   0 equality;   9 variable)
%            Maximal formula depth :    9 (   4 average)
%            Number of connectives :   14 (   0   ~;   0   |;   0   &;  12   @)
%                                         (   1 <=>;   1  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of type conns  :   14 (  14   >;   0   *;   0   +;   0  <<)
%            Number of symbols     :    9 (   6   :;   0   =)
%            Number of variables   :    5 (   0 sgn;   4   !;   1   ?;   0   ^)
%                                         (   5   :;   0  !>;   0  ?*)
%                                         (   0  @-;   0  @+)
% SPC      : TH0_THM_NEQ_NAR

% Comments : This is a simple test problem for reasoning in/about SUMO.
%            Initally the problem has been hand generated in KIF syntax in
%            SigmaKEE and then automatically translated by Benzmueller's
%            KIF2TH0 translator into THF syntax.
%          : The translation has been applied in three modes: handselected,
%            SInE, and local. The local mode only translates the local
%            assumptions and the query. The SInE mode additionally translates
%            the SInE extract of the loaded knowledge base (usually SUMO). The
%            handselected mode contains a hand-selected relevant axioms.
%          : The examples are selected to illustrate the benefits of
%            higher-order reasoning in ontology reasoning.
%------------------------------------------------------------------------------
%----The extracted signature
thf(numbers,type,(
    num: $tType )).

thf(husband_THFTYPE_IiioI,type,(
    husband_THFTYPE_IiioI: $i > $i > $o )).

thf(lChris_THFTYPE_i,type,(
    lChris_THFTYPE_i: $i )).

thf(lCorina_THFTYPE_i,type,(
    lCorina_THFTYPE_i: $i )).

thf(wife_THFTYPE_IiioI,type,(
    wife_THFTYPE_IiioI: $i > $i > $o )).

%----The handselected axioms from the knowledge base
thf(inverse_THFTYPE_IIiioIIiioIoI,type,(
    inverse_THFTYPE_IIiioIIiioIoI: ( $i > $i > $o ) > ( $i > $i > $o ) > $o )).

thf(ax,axiom,
    ( inverse_THFTYPE_IIiioIIiioIoI @ husband_THFTYPE_IiioI @ wife_THFTYPE_IiioI )).

thf(ax_001,axiom,(
    ! [REL2: $i > $i > $o,REL1: $i > $i > $o] :
      ( ( inverse_THFTYPE_IIiioIIiioIoI @ REL1 @ REL2 )
     => ! [INST1: $i,INST2: $i] :
          ( ( REL1 @ INST1 @ INST2 )
        <=> ( REL2 @ INST2 @ INST1 ) ) ) )).

%----The translated axioms
thf(ax_002,axiom,
    ( wife_THFTYPE_IiioI @ lCorina_THFTYPE_i @ lChris_THFTYPE_i )).

%----The translated conjectures
thf(con,conjecture,(
    ? [X: $i] :
      ( husband_THFTYPE_IiioI @ X @ lCorina_THFTYPE_i ) )).

%------------------------------------------------------------------------------
