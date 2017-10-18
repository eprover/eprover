%------------------------------------------------------------------------------
% File     : SWB008+1 : TPTP v6.4.0. Released v5.2.0.
% Domain   : Semantic Web
% Problem  : Inverse Functional Data Properties
% Version  : [Sch11] axioms.
% English  : 

% Refs     : [Sch11] Schneider, M. (2011), Email to G. Sutcliffe
% Source   : [Sch11]
% Names    : 008_Inverse_Functional_Data_Properties [Sch11]

% Status   : Theorem
% Rating   : 0.37 v6.4.0, 0.35 v6.3.0, 0.33 v6.2.0, 0.48 v6.1.0, 0.60 v6.0.0, 0.61 v5.5.0, 0.63 v5.4.0, 0.64 v5.3.0, 0.70 v5.2.0
% Syntax   : Number of formulae    :  560 ( 197 unit)
%            Number of atoms       : 1777 (  90 equality)
%            Maximal formula depth :   27 (   5 average)
%            Number of connectives : 1353 ( 136   ~;  35   |; 761   &)
%                                         ( 126 <=>; 295  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of predicates  :   13 (   1 propositional; 0-3 arity)
%            Number of functors    :  162 ( 160 constant; 0-2 arity)
%            Number of variables   :  973 (   0 sgn; 911   !;  62   ?)
%            Maximal term depth    :    2 (   1 average)
% SPC      : FOF_THM_RFO_SEQ

% Comments :
%------------------------------------------------------------------------------
%----Include OWL 2 Full axioms
include('Axioms/SWB001+0.ax').
%------------------------------------------------------------------------------
fof(testcase_conclusion_fullish_008_Inverse_Functional_Data_Properties,conjecture,(
    iext(uri_owl_sameAs,uri_ex_bob,uri_ex_robert) )).

fof(testcase_premise_fullish_008_Inverse_Functional_Data_Properties,axiom,
    ( iext(uri_rdf_type,uri_foaf_mbox_sha1sum,uri_owl_DatatypeProperty)
    & iext(uri_rdf_type,uri_foaf_mbox_sha1sum,uri_owl_InverseFunctionalProperty)
    & iext(uri_foaf_mbox_sha1sum,uri_ex_bob,literal_plain(dat_str_xyz))
    & iext(uri_foaf_mbox_sha1sum,uri_ex_robert,literal_plain(dat_str_xyz)) )).

%------------------------------------------------------------------------------
