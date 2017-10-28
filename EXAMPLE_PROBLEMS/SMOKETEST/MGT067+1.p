%--------------------------------------------------------------------------
% File     : MGT067+1 : TPTP v7.0.0. Released v5.5.0.
% Domain   : Management
% Problem  : Authorization to create requisitions
% Version  : Especial.
% English  : A first-order theory representing the formalization of an 
%            SAP R/3 authorization and process setup together with business 
%            policies for a purchase process.

% Refs     : 
% Source   : [TPTP]
% Names    : 

% Status   : Theorem
% Rating   : 0.25 v7.0.0, 0.29 v6.3.0, 0.31 v6.2.0, 0.36 v6.1.0, 0.28 v6.0.0, 0.50 v5.5.0
% Syntax   : Number of formulae    :  158 ( 127 unit)
%            Number of atoms       :  330 (   0 equality)
%            Maximal formula depth :   27 (   3 average)
%            Number of connectives :  196 (  24   ~;   8   |; 134   &)
%                                         (   0 <=>;  30  =>;   0  <=;   0 <~>)
%                                         (   0  ~|;   0  ~&)
%            Number of predicates  :   15 (   0 propositional; 1-13 arity)
%            Number of functors    :   80 (  73 constant; 0-3 arity)
%            Number of variables   :  181 (   0 sgn; 160   !;  21   ?)
%            Maximal term depth    :    3 (   2 average)
% SPC      : FOF_THM_RFO_NEQ

% Comments : Taken from the SPASS web site
%                http://www.spass-prover.org/prototypes/index.html
%--------------------------------------------------------------------------
fof(ax1,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssS_TCODE,ssTCD,ssME51N))) )).

fof(ax2,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssS_TCODE,ssTCD,ssME52N))) )).

fof(ax3,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssS_TCODE,ssTCD,ssME53N))) )).

fof(ax4,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_WRK,ssACTVT,np__1))) )).

fof(ax5,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_WRK,ssACTVT,np__2))) )).

fof(ax6,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_WRK,ssACTVT,np__3))) )).

fof(ax7,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_WRK,ssWERKS,ssADSU))) )).

fof(ax8,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_BSA,ssACTVT,ssSTAR))) )).

fof(ax9,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_BSA,ssBSART,ssSTAR))) )).

fof(ax10,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_EKG,ssACTVT,ssSTAR))) )).

fof(ax11,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_ADSU_ED,authObj(ssM_BANF_EKG,ssEKGRP,ssI26))) )).

%----ZBANF_WRK_INF_ED
%----Role to create requisitions for plant INF, purchase group I26
fof(ax12,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssS_TCODE,ssTCD,ssME51N))) )).

fof(ax13,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_WRK,ssACTVT,np__1))) )).

fof(ax14,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_WRK,ssACTVT,np__2))) )).

fof(ax15,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_WRK,ssACTVT,np__3))) )).

fof(ax16,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_WRK,ssWERKS,ssINF))) )).

fof(ax17,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_BSA,ssACTVT,ssSTAR))) )).

fof(ax18,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_BSA,ssBSART,ssSTAR))) )).

fof(ax19,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_EKG,ssACTVT,ssSTAR))) )).

fof(ax20,axiom,(
    ssSingleRole(singleRoleEntry(ssZBANF_WRK_INF_ED,authObj(ssM_BANF_EKG,ssEKGRP,ssI26))) )).

%----ZRELEASE_WRK_INF_BASE
%----Permits access to release and view requisitions, but does not specify a 
%----release group or code
fof(ax21,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssS_TCODE,ssTCD,ssME53N))) )).

fof(ax22,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssS_TCODE,ssTCD,ssME54N))) )).

fof(ax23,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_BSA,ssACTVT,np__2))) )).

fof(ax24,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_BSA,ssACTVT,np__3))) )).

fof(ax25,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_BSA,ssACTVT,np__8))) )).

fof(ax26,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_BSA,ssBSART,ssNB))) )).

fof(ax27,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_EKG,ssACTVT,np__2))) )).

fof(ax28,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_EKG,ssACTVT,np__3))) )).

fof(ax29,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_EKG,ssACTVT,np__8))) )).

fof(ax30,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_EKG,ssEKGRP,ssI26))) )).

fof(ax31,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_WRK,ssACTVT,np__2))) )).

fof(ax32,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_WRK,ssACTVT,np__3))) )).

fof(ax33,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_WRK,ssACTVT,np__8))) )).

fof(ax34,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_BASE,authObj(ssM_BANF_WRK,ssWERKS,ssINF))) )).

%----Extension 1 to role RELEASE_INF_BASE:
%----Permits the release of requisitions with release group 01 (=single row 
%----release) and code W1 (=cost center release)
fof(ax35,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_W1,authObj(ssM_EINK_FRG,ssFRGCO,ssW1))) )).

fof(ax36,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_W1,authObj(ssM_EINK_FRG,ssFRGGR,ssRGRP_01))) )).

%----Extension 2 to role RELEASE_INF_BASE:
%----Permits the release of requisitions with release group 01 (=single row 
%----release) and code W2 (=management)
fof(ax37,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_W2,authObj(ssM_EINK_FRG,ssFRGCO,ssW2))) )).

fof(ax38,axiom,(
    ssSingleRole(singleRoleEntry(ssZRELEASE_WRK_INF_W2,authObj(ssM_EINK_FRG,ssFRGGR,ssRGRP_01))) )).

%----ZORDER_WRK_INF_BASE
%----Base role to create/change/view orders
fof(ax39,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssS_TCODE,ssTCD,ssME21N))) )).

fof(ax40,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssS_TCODE,ssTCD,ssME22N))) )).

fof(ax41,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssS_TCODE,ssTCD,ssME23N))) )).

fof(ax42,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKO,ssACTVT,np__1))) )).

fof(ax43,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKO,ssACTVT,np__2))) )).

fof(ax44,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKO,ssACTVT,np__3))) )).

fof(ax45,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKO,ssACTVT,np__8))) )).

fof(ax46,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKO,ssACTVT,np__9))) )).

fof(ax47,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKO,ssEKORG,ssINF))) )).

fof(ax48,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_BSA,ssACTVT,np__1))) )).

fof(ax49,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_BSA,ssACTVT,np__2))) )).

fof(ax50,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_BSA,ssACTVT,np__3))) )).

fof(ax51,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_BSA,ssACTVT,np__8))) )).

fof(ax52,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_BSA,ssACTVT,np__9))) )).

fof(ax53,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BANF_BSA,ssACTVT,np__1))) )).

fof(ax54,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_WRK,ssACTVT,np__1))) )).

fof(ax55,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_WRK,ssACTVT,np__2))) )).

fof(ax56,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_WRK,ssACTVT,np__3))) )).

fof(ax57,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_WRK,ssACTVT,np__8))) )).

fof(ax58,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_WRK,ssACTVT,np__9))) )).

fof(ax59,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_WRK,ssWERKS,ssINF))) )).

fof(ax60,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKG,ssACTVT,np__1))) )).

fof(ax61,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKG,ssACTVT,np__2))) )).

fof(ax62,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKG,ssACTVT,np__3))) )).

fof(ax63,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKG,ssACTVT,np__8))) )).

fof(ax64,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKG,ssACTVT,np__9))) )).

fof(ax65,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_BASE,authObj(ssM_BEST_EKG,ssEKGRP,ssI26))) )).

%----Extension 1:
%---- -> direct order allowed: transaction code ME21NOFF, but
%----   - limit to office material,
%----   - limit the amount of money
fof(ax66,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_OFFICE,authObj(ssS_TCODE,ssTCD,ssME21NOFF))) )).

fof(ax67,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_OFFICE,authObj(ssM_EINK_MKL,ssMATKL,ssB0002))) )).

fof(ax68,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_OFFICE,authObj(ssM_EINK_GWT,ssGSWRT,ssLESS_EQUAL_1000_EUR))) )).

fof(ax69,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_OFFICE,authObj(ssM_BEST_BSA,ssBSART,ssDIRECT))) )).

fof(ax70,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_OFFICE,authObj(ssM_BANF_BSA,ssBSART,ssDIRECT))) )).

%----Extension 2:
%---- - no limits -
fof(ax71,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_ALL,authObj(ssM_EINK_MKL,ssMATKL,ssSTAR))) )).

fof(ax72,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_ALL,authObj(ssM_EINK_GWT,ssGSWRT,ssSTAR))) )).

fof(ax73,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_ALL,authObj(ssM_BEST_BSA,ssBSART,ssSTAR))) )).

fof(ax74,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_INF_TYPE_ALL,authObj(ssM_BANF_BSA,ssBSART,ssSTAR))) )).

%----ZORDER_WRK_ADSU_ED
%----Order role for plant ADSU
fof(ax75,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssS_TCODE,ssTCD,ssME21N))) )).

fof(ax76,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKO,ssACTVT,np__1))) )).

fof(ax77,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKO,ssACTVT,np__2))) )).

fof(ax78,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKO,ssACTVT,np__3))) )).

fof(ax79,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKO,ssACTVT,np__8))) )).

fof(ax80,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKO,ssACTVT,np__9))) )).

fof(ax81,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKO,ssEKORG,ssADSU))) )).

fof(ax82,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_BSA,ssACTVT,np__1))) )).

fof(ax83,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_BSA,ssACTVT,np__2))) )).

fof(ax84,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_BSA,ssACTVT,np__3))) )).

fof(ax85,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_BSA,ssACTVT,np__8))) )).

fof(ax86,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_BSA,ssACTVT,np__9))) )).

fof(ax87,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_BSA,ssBSART,ssNB))) )).

fof(ax88,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BANF_BSA,ssACTVT,np__1))) )).

fof(ax89,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BANF_BSA,ssBSART,ssNB))) )).

fof(ax90,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_WRK,ssACTVT,np__1))) )).

fof(ax91,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_WRK,ssACTVT,np__2))) )).

fof(ax92,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_WRK,ssACTVT,np__3))) )).

fof(ax93,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_WRK,ssACTVT,np__8))) )).

fof(ax94,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_WRK,ssACTVT,np__9))) )).

fof(ax95,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_WRK,ssWERKS,ssADSU))) )).

fof(ax96,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKG,ssACTVT,np__1))) )).

fof(ax97,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKG,ssACTVT,np__2))) )).

fof(ax98,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKG,ssACTVT,np__3))) )).

fof(ax99,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKG,ssACTVT,np__8))) )).

fof(ax100,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKG,ssACTVT,np__9))) )).

fof(ax101,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_BEST_EKG,ssEKGRP,ssI26))) )).

fof(ax102,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_EINK_MKL,ssMATKL,ssSTAR))) )).

fof(ax103,axiom,(
    ssSingleRole(singleRoleEntry(ssZORDER_WRK_ADSU_ED,authObj(ssM_EINK_GWT,ssGSWRT,ssSTAR))) )).

%----Composite Roles
%----Permits releasing of requisitions: plant INF, single position release, 
%----release code W1 (cost center)
fof(ax104,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZRELEASE_WRK_INF_W1_MULTI,ssZRELEASE_WRK_INF_BASE)) )).

fof(ax105,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZRELEASE_WRK_INF_W1_MULTI,ssZRELEASE_WRK_INF_W1)) )).

%----Permits releasing of requisitions: plant INF, single position release, 
%----release code W2 (management)
fof(ax106,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZRELEASE_WRK_INF_W2_MULTI,ssZRELEASE_WRK_INF_BASE)) )).

fof(ax107,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZRELEASE_WRK_INF_W2_MULTI,ssZRELEASE_WRK_INF_W2)) )).

%----Create arbitrary orders
fof(ax108,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZORDER_WRK_INF_ALL,ssZORDER_WRK_INF_BASE)) )).

fof(ax109,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZORDER_WRK_INF_ALL,ssZORDER_WRK_INF_TYPE_ALL)) )).

%----Create orders only for office material with limited amount of money
fof(ax110,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZORDER_WRK_INF_OFFICE,ssZORDER_WRK_INF_BASE)) )).

fof(ax111,axiom,(
    ssCompositeRole(compositeRoleEntry(ssZORDER_WRK_INF_OFFICE,ssZORDER_WRK_INF_TYPE_OFFICE)) )).

%----Assign Roles to Users
fof(ax112,axiom,(
    ssHolds(ssKAISER,ssZRELEASE_WRK_INF_W1_MULTI) )).

fof(ax113,axiom,(
    ssHolds(ssMEIER,ssZRELEASE_WRK_INF_W2_MULTI) )).

%----Create requisitions
fof(ax114,axiom,(
    ssHolds(ssMUELLER,ssZBANF_WRK_INF_ED) )).

%----Create requisitions
fof(ax115,axiom,(
    ssHolds(ssSCHMITT,ssZORDER_WRK_INF_ALL) )).

%----Release requisitions with code W1
fof(ax116,axiom,(
    ssHolds(ssLAMOTTE,ssZORDER_WRK_INF_OFFICE) )).

fof(ax117,axiom,(
    ssDirectPurchase(ssDIRECT) )).

fof(ax118,axiom,(
    ssStandardPurchase(ssNB) )).

fof(ax119,axiom,(
    ! [U] :
      ~ ( ssDirectPurchase(U)
        & ssStandardPurchase(U) ) )).

%----Code Layer
%----Customizing: Release Strategy
%----Release Requirements
fof(ax120,axiom,(
    ssReleaseRequirement(ssKF,ssRGRP_01,ssW1) )).

fof(ax121,axiom,(
    ssReleaseRequirement(ssVF,ssRGRP_01,ssW1) )).

fof(ax122,axiom,(
    ssReleaseRequirement(ssVF,ssRGRP_01,ssW2) )).

%----E.g. ReleaseStrategy(cost center release [KF]/management release [VF], 
%----release group 01, class(FRG_EBAN, property(...)))
fof(ax123,axiom,(
    ssReleaseStrategy(ssKF,ssRGRP_01,class(ssFRG_EBAN,property(ssFRG_CEBAN_EKGRP,ssI26))) )).

fof(ax124,axiom,(
    ssReleaseStrategy(ssKF,ssRGRP_01,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssINF))) )).

fof(ax125,axiom,(
    ssReleaseStrategy(ssKF,ssRGRP_01,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_1000_LESS_10000_EUR))) )).

fof(ax126,axiom,(
    ssReleaseStrategy(ssVF,ssRGRP_01,class(ssFRG_EBAN,property(ssFRG_CEBAN_EKGRP,ssI26))) )).

fof(ax127,axiom,(
    ssReleaseStrategy(ssVF,ssRGRP_01,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssINF))) )).

fof(ax128,axiom,(
    ssReleaseStrategy(ssVF,ssRGRP_01,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_10000_EUR))) )).

fof(ax129,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssINF)))
     => ( ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssADSU)))
        & ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssGRSU))) ) ) )).

fof(ax130,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssGRSU)))
     => ( ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssADSU)))
        & ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssINF))) ) ) )).

fof(ax131,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssADSU)))
     => ( ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssINF)))
        & ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_WERKS,ssGRSU))) ) ) )).

fof(ax132,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_1000_LESS_10000_EUR)))
     => ( ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_10000_EUR)))
        & ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssLESS_EQUAL_1000_EUR))) ) ) )).

fof(ax133,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_10000_EUR)))
     => ( ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_1000_LESS_10000_EUR)))
        & ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssLESS_EQUAL_1000_EUR))) ) ) )).

fof(ax134,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssLESS_EQUAL_1000_EUR)))
     => ( ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_1000_LESS_10000_EUR)))
        & ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_GSWRT,ssGREATER_10000_EUR))) ) ) )).

fof(ax135,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_EKGRP,ssI26)))
     => ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_EKGRP,ssI27))) ) )).

fof(ax136,axiom,(
    ! [U,V] :
      ( ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_EKGRP,ssI27)))
     => ~ ssReleaseStrategy(U,V,class(ssFRG_EBAN,property(ssFRG_CEBAN_EKGRP,ssI26))) ) )).

%----Access()
%----Check of an authorization object with its value, the user profile must 
%----have the same authorization object with an equal value or STAR
fof(ax137,axiom,(
    ! [U,V,W,X] :
      ( ssUserProfile(userProfileEntry(U,authObj(V,W,X)))
     => ssAccess(U,authObj(V,W,X)) ) )).

fof(ax138,axiom,(
    ! [U,V,W,X] :
      ( ssUserProfile(userProfileEntry(U,authObj(V,W,ssSTAR)))
     => ssAccess(U,authObj(V,W,X)) ) )).

fof(ax139,axiom,(
    ! [U,V,W] :
      ( ( ssUserProfile(userProfileEntry(U,authObj(V,W,ssGREATER_1000_LESS_10000_EUR)))
        | ssUserProfile(userProfileEntry(U,authObj(V,W,ssGREATER_10000_EUR))) )
     => ssAccess(U,authObj(V,W,ssLESS_EQUAL_1000_EUR)) ) )).

fof(ax140,axiom,(
    ! [U,V,W] :
      ( ssUserProfile(userProfileEntry(U,authObj(V,W,ssGREATER_10000_EUR)))
     => ssAccess(U,authObj(V,W,ssGREATER_1000_LESS_10000_EUR)) ) )).

%----Profile/(Single,Composite)Role -> Userprofile()
%----A composite role contains other roles. Each authorization in a profile or 
%----single/composite role will be inserted into the user profile.
fof(ax141,axiom,(
    ! [U,V,W,X,Y,Z,X1] :
      ( ( ( ssProfile(profileEntry(V,authObj(Y,Z,X1)))
          & ssHolds(U,V) )
        | ( ssSingleRole(singleRoleEntry(W,authObj(Y,Z,X1)))
          & ssHolds(U,W) )
        | ( ssCompositeRole(compositeRoleEntry(X,W))
          & ssSingleRole(singleRoleEntry(W,authObj(Y,Z,X1)))
          & ssHolds(U,X) ) )
     => ssUserProfile(userProfileEntry(U,authObj(Y,Z,X1))) ) )).

%----Abstraction
%----ME51N - Create Requisitions
fof(ax142,axiom,(
    ! [U,V,W,X] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME51N))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssEKGRP,X)) )
     => ssAccess(U,ssME51N) ) )).

%----ME52N - Change Requisitions
fof(ax143,axiom,(
    ! [U,V,W,X] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME52N))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssEKGRP,X)) )
     => ssAccess(U,ssME52N) ) )).

%----ME53N - View Requisitions
fof(ax144,axiom,(
    ! [U,V,W,X] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME53N))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssEKGRP,X)) )
     => ssAccess(U,ssME53N) ) )).

%----ME54N - Release Requisitions
fof(ax145,axiom,(
    ! [U,V,W,X,Y,Z] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME54N))
        & ssAccess(U,ssME53N)
        & ssAccess(U,authObj(ssM_BANF_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssEKGRP,X))
        & ssAccess(U,authObj(ssM_EINK_FRG,ssFRGGR,Y))
        & ssAccess(U,authObj(ssM_EINK_FRG,ssFRGCO,Z))
        & ssAccess(U,authObj(ssM_BANF_WRK,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssACTVT,np__2)) )
     => ssAccess(U,ssME54N) ) )).

%----ME21N - Create Order
fof(ax146,axiom,(
    ! [U,V,W,X,Y] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME21N))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssEKORG,Y))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssEKGRP,X)) )
     => ssAccess(U,ssME21N) ) )).

%----ME22N - Change Order
fof(ax147,axiom,(
    ! [U,V,W,X,Y] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME22N))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssEKORG,Y))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__2))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssEKGRP,X)) )
     => ssAccess(U,ssME22N) ) )).

%----ME23N - View Order
fof(ax148,axiom,(
    ! [U,V,W,X,Y] :
      ( ( ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME23N))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssEKORG,Y))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssACTVT,np__1))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BEST_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__3))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__8))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssACTVT,np__9))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssEKGRP,X)) )
     => ssAccess(U,ssME23N) ) )).

%----Business process steps
%----Role: Requisition, includes create/change/view requisitions
%----If there is a requisition and an user has access to create the 
%----requisition object, then the requisition object is created by this user
fof(ax149,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3,X4] :
      ( ( ssRequisition(W,Z,X1,V,X,X2,Y,X3,X4)
        & ssStandardPurchase(W)
        & ssAccess(U,ssME51N)
        & ssAccess(U,authObj(ssM_BANF_WRK,ssWERKS,V))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,W))
        & ssAccess(U,authObj(ssM_BANF_EKG,ssEKGRP,X)) )
     => ssRequisitionCreated(U,W,Z,X1,V,X,X2,Y,X3,X4) ) )).

%----Role: Release
fof(ax150,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3,X4,X5,X6,X7,X8,X9] :
      ( ( ssRequisitionCreated(U,X,X2,X3,W,Y,Z,X1,X4,X9)
        & ssReleaseStrategy(X5,X6,class(X8,property(ssFRG_CEBAN_EKGRP,Y)))
        & ssReleaseStrategy(X5,X6,class(X8,property(ssFRG_CEBAN_WERKS,W)))
        & ssReleaseStrategy(X5,X6,class(X8,property(ssFRG_CEBAN_GSWRT,X4)))
        & ssReleaseRequirement(X5,X6,X7)
        & ssAccess(V,authObj(ssM_EINK_FRG,ssFRGGR,X6))
        & ssAccess(V,authObj(ssM_EINK_FRG,ssFRGCO,X7))
        & ssAccess(V,ssME54N)
        & ssAccess(V,authObj(ssM_BANF_WRK,ssWERKS,W))
        & ssAccess(V,authObj(ssM_BANF_BSA,ssBSART,X))
        & ssAccess(V,authObj(ssM_BANF_EKG,ssEKGRP,Y)) )
     => ssRequisitionReleasedStep(V,X6,X5,X7,X,X2,X3,W,Y,Z,X1,X4,X9) ) )).

%----No release strategy matches
fof(ax151,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3,X4,X5,X6] :
      ( ( ssRequisitionCreated(U,W,X1,X2,V,X,Y,Z,X3,X6)
        & ( ~ ssReleaseStrategy(ssKF,X4,class(X5,property(ssFRG_CEBAN_EKGRP,X)))
          | ~ ssReleaseStrategy(ssKF,X4,class(X5,property(ssFRG_CEBAN_WERKS,V)))
          | ~ ssReleaseStrategy(ssKF,X4,class(X5,property(ssFRG_CEBAN_GSWRT,X3))) )
        & ( ~ ssReleaseStrategy(ssVF,X4,class(X5,property(ssFRG_CEBAN_EKGRP,X)))
          | ~ ssReleaseStrategy(ssVF,X4,class(X5,property(ssFRG_CEBAN_WERKS,V)))
          | ~ ssReleaseStrategy(ssVF,X4,class(X5,property(ssFRG_CEBAN_GSWRT,X3))) ) )
     => ssRequisitionReleased(U,W,X1,X2,V,X,Y,Z,X3,X6) ) )).

%----Customizing: Definition of the release codes which are necessary to 
%----release a requisition (only if the strategy matches).
fof(ax152,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3,X4,X5] :
      ( ( ssRequisitionReleasedStep(V,ssRGRP_01,ssKF,ssW1,X,X2,X3,W,Y,Z,X1,X4,X5)
        | ( ssRequisitionReleasedStep(U,ssRGRP_01,ssVF,ssW1,X,X2,X3,W,Y,Z,X1,X4,X5)
          & ssRequisitionReleasedStep(V,ssRGRP_01,ssVF,ssW2,X,X2,X3,W,Y,Z,X1,X4,X5) ) )
     => ssRequisitionReleased(V,X,X2,X3,W,Y,Z,X1,X4,X5) ) )).

%----Role: Order
%----If a requisition has been created and has already been released and the 
%----user has access to create order objects, then the order (object) will be 
%----created by the user
fof(ax153,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3,X4,X5] :
      ( ( ssRequisitionReleased(U,X,X2,X3,W,Y,X1,Z,X4,X5)
        & ssAccess(V,ssME21N)
        & ssAccess(V,ssME22N)
        & ssAccess(V,ssME23N)
        & ssAccess(V,authObj(ssM_BEST_WRK,ssWERKS,W))
        & ssAccess(V,authObj(ssM_BANF_BSA,ssBSART,X))
        & ssAccess(V,authObj(ssM_BEST_BSA,ssBSART,X))
        & ssAccess(V,authObj(ssM_BEST_EKG,ssEKGRP,Y))
        & ssAccess(V,authObj(ssM_BEST_EKO,ssEKORG,X1))
        & ssAccess(V,authObj(ssM_EINK_MKL,ssMATKL,Z))
        & ssAccess(V,authObj(ssM_EINK_GWT,ssGSWRT,X4)) )
     => ssOrderCreated(V,X,X2,X3,W,Y,X1,Z,X4,X5) ) )).

%----Special case: direct order (e.g. office material)
fof(ax154,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3,X4,X5,X6] :
      ( ( ssRequisition(Y,X3,X4,X,Z,X2,X1,X5,X6)
        & ssDirectPurchase(Y)
        & ssAccess(U,authObj(ssM_EINK_MKL,ssMATKL,X1))
        & ssAccess(U,authObj(ssM_EINK_GWT,ssGSWRT,X5))
        & ssAccess(U,authObj(ssS_TCODE,ssTCD,ssME21NOFF))
        & ssAccess(U,ssME21N)
        & ssAccess(U,ssME22N)
        & ssAccess(U,ssME23N)
        & ssAccess(U,authObj(ssM_BEST_WRK,ssWERKS,X))
        & ssAccess(U,authObj(ssM_BANF_BSA,ssBSART,Y))
        & ssAccess(U,authObj(ssM_BEST_BSA,ssBSART,Y))
        & ssAccess(U,authObj(ssM_BEST_EKG,ssEKGRP,Z))
        & ssAccess(U,authObj(ssM_BEST_EKO,ssEKORG,X2)) )
     => ( ssOrderCreated(U,Y,X3,X4,X,Z,X2,X1,X5,X6)
        & ~ ssRequisitionCreated(V,Y,X3,X4,X,Z,X2,X1,X5,X6)
        & ~ ssRequisitionReleased(W,Y,X3,X4,X,Z,X2,X1,X5,X6) ) ) )).

%----Business Policies
%----There exists no user, which is allowed to perform the complete purchase 
%----process:
%     - create a requisition
%     - to release a requisition
%     - to create an order
fof(ax155,axiom,(
    ~ ? [U,V,W,X,Y,Z,X1,X2,X3,X4] :
        ( ssRequisition(Z,X1,X2,V,W,X,Y,X3,X4)
       => ( ssRequisitionCreated(U,Z,X1,X2,V,W,X,Y,X3,X4)
          & ssRequisitionReleased(U,Z,X1,X2,V,W,X,Y,X3,X4)
          & ssOrderCreated(U,Z,X1,X2,V,W,X,Y,X3,X4) ) ) )).

%----It is prohibited to create orders greater than 1000 EUR without a 
%----requisition
fof(ax156,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3] :
    ? [X4] :
      ( ssOrderCreated(U,Z,X1,X2,V,X,Y,W,ssGREATER_1000_LESS_10000_EUR,X3)
     => ssRequisitionCreated(X4,Z,X1,X2,V,X,Y,W,ssGREATER_1000_LESS_10000_EUR,X3) ) )).

fof(ax157,axiom,(
    ! [U,V,W,X,Y,Z,X1,X2,X3] :
    ? [X4] :
      ( ssOrderCreated(U,Z,X1,X2,V,X,Y,W,ssGREATER_10000_EUR,X3)
     => ssRequisitionCreated(X4,Z,X1,X2,V,X,Y,W,ssGREATER_10000_EUR,X3) ) )).

fof(co1,conjecture,(
    ? [U,V,W,X,Y,Z,X1,X2,X3] :
      ( ssRequisition(Y,Z,X1,U,ssINF,W,X,X2,X3)
     => ssRequisitionCreated(ssMUELLER,Y,Z,X1,ssINF,V,W,X,X2,X3) ) )).

%------------------------------------------------------------------------------
