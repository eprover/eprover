tff(hyst_decl_0, type, (z_0 : $int)).
tff(hyst_decl_1, type, (z_3 : $int > $int)).
tff(hyst_decl_2, type, (z_3p1 : $int > $int)).
tff(hyst_decl_3, type, (z_3m1 : $int > $int)).
tff(hyst_decl_4, type, (z_opp : $int > $int)).
tff(hyst_decl_5, type, (z_plus : ($int * $int) > $int)).
tff(hyst_decl_6, type, (z_minus : ($int * $int) > $int)).
tff(hyst_decl_7, type, (z_mult : ($int * $int) > $int)).
tff(hyst_decl_8, type, (b_true : z_bool)).
tff(hyst_decl_9, type, (b_false : z_bool)).
tff(hyst_decl_10, type, (b_istrue : z_bool > $o)).
tff(hyst_decl_11, type, (z_lt : ($int * $int) > z_bool)).
tff(hyst_decl_12, type, (z_leq : ($int * $int) > z_bool)).
tff(hyst_decl_13, type, (z_gt : ($int * $int) > z_bool)).
tff(hyst_decl_14, type, (z_geq : ($int * $int) > z_bool)).
tff(hyst_decl_15, type, (member : (element * set) > $o)).
tff(hyst_decl_16, type, (empty_set : set)).
tff(hyst_decl_17, type, (singleton : element > set)).
tff(hyst_decl_18, type, (subset : (set * set) > $o)).
tff(hyst_decl_19, type, (intersection : (set * set) > set)).
tff(hyst_decl_20, type, (union : (set * set) > set)).
tff(hyst_decl_21, type, (difference : (set * set) > set)).
tff(hyst_decl_22, type, (complement : set > set)).
tff(hyst_decl_23, type, (cardinality : set > $int)).
tff(hyst_decl_24, type, (logtk_sk__0 : set > element)).
tff(hyst_decl_25, type, (logtk_sk__1 : (set * set) > element)).
tff(hyst_decl_26, type, (logtk_sk__2 : element)).
tff(hyst_decl_27, type, (logtk_sk__3 : set)).
tff(hyst_decl_28, type, (logtk_sk__4 : $int)).
tff(hyst_decl_29, type, (z_positive : $int > z_bool)).
tff(hyst_decl_30, type, (z_positiveAux : ($int * z_bool) > z_bool)).
tff(hyst_axiom_0, axiom, (b_istrue(b_true))).
tff(hyst_axiom_1, axiom, (~b_istrue(b_false))).
tff(hyst_axiom_2, axiom, ((X0:z_bool = b_true) | (X0:z_bool = b_false))).
tff(hyst_axiom_3, axiom, (b_true != b_false)).
tff(hyst_axiom_4, axiom, ((z_3(X0:$int) != X0:$int) | (X0:$int = z_0))).
tff(hyst_axiom_5, axiom, (z_3p1(X0:$int) != X0:$int)).
tff(hyst_axiom_6, axiom, (z_3m1(X0:$int) != X0:$int)).
tff(hyst_axiom_7, axiom, (z_plus(X0:$int,z_plus(X1:$int,X2:$int)) = z_plus(z_plus(X0:$int,X1:$int),X2:$int))).
tff(hyst_axiom_8, axiom, (z_plus(X0:$int,X1:$int) = z_plus(X1:$int,X0:$int))).
tff(hyst_axiom_9, axiom, (z_mult(X0:$int,z_mult(X1:$int,X2:$int)) = z_mult(z_mult(X0:$int,X1:$int),X2:$int))).
tff(hyst_axiom_10, axiom, (z_mult(X0:$int,X1:$int) = z_mult(X1:$int,X0:$int))).
tff(hyst_axiom_11, axiom, (z_lt(X0:$int,X1:$int) = z_gt(X1:$int,X0:$int))).
tff(hyst_axiom_12, axiom, (z_leq(X0:$int,X1:$int) = z_geq(X1:$int,X0:$int))).
tff(hyst_axiom_13, axiom, (z_gt(X0:$int,X1:$int) = z_positive(z_plus(z_minus(X0:$int,X1:$int),z_3m1(z_0))))).
tff(hyst_axiom_14, axiom, (z_geq(X0:$int,X1:$int) = z_positive(z_minus(X0:$int,X1:$int)))).
tff(hyst_axiom_15, axiom, (z_positiveAux(z_3m1(X0:$int),X1:z_bool) = z_positiveAux(X0:$int,b_false))).
tff(hyst_axiom_16, axiom, (z_positiveAux(z_3p1(X0:$int),X1:z_bool) = z_positiveAux(X0:$int,b_true))).
tff(hyst_axiom_17, axiom, (z_positiveAux(z_3(X0:$int),X1:z_bool) = z_positiveAux(X0:$int,X1:z_bool))).
tff(hyst_axiom_18, axiom, (z_positiveAux(z_0,X0:z_bool) = X0:z_bool)).
tff(hyst_axiom_19, axiom, (z_positive(X0:$int) = z_positiveAux(X0:$int,b_true))).
tff(hyst_axiom_20, axiom, (z_mult(X0:$int,z_3m1(X1:$int)) = z_plus(z_3(z_mult(X0:$int,X1:$int)),z_opp(X0:$int)))).
tff(hyst_axiom_21, axiom, (z_mult(X0:$int,z_3p1(X1:$int)) = z_plus(X0:$int,z_3(z_mult(X0:$int,X1:$int))))).
tff(hyst_axiom_22, axiom, (z_mult(X0:$int,z_3(X1:$int)) = z_3(z_mult(X0:$int,X1:$int)))).
tff(hyst_axiom_23, axiom, (z_mult(X0:$int,z_0) = z_0)).
tff(hyst_axiom_24, axiom, (z_minus(X0:$int,X1:$int) = z_plus(X0:$int,z_opp(X1:$int)))).
tff(hyst_axiom_25, axiom, (z_opp(z_3m1(X0:$int)) = z_3p1(z_opp(X0:$int)))).
tff(hyst_axiom_26, axiom, (z_opp(z_3p1(X0:$int)) = z_3m1(z_opp(X0:$int)))).
tff(hyst_axiom_27, axiom, (z_opp(z_3(X0:$int)) = z_3(z_opp(X0:$int)))).
tff(hyst_axiom_28, axiom, (z_opp(z_0) = z_0)).
tff(hyst_axiom_29, axiom, (z_plus(z_3m1(X0:$int),z_3m1(X1:$int)) = z_3p1(z_plus(X0:$int,z_plus(X1:$int,z_3m1(z_0)))))).
tff(hyst_axiom_30, axiom, (z_plus(z_3p1(X0:$int),z_3p1(X1:$int)) = z_3m1(z_plus(X0:$int,z_plus(X1:$int,z_3p1(z_0)))))).
tff(hyst_axiom_31, axiom, (z_plus(z_3m1(X0:$int),z_3p1(X1:$int)) = z_3(z_plus(X0:$int,X1:$int)))).
tff(hyst_axiom_32, axiom, (z_plus(z_3p1(X0:$int),z_3m1(X1:$int)) = z_3(z_plus(X0:$int,X1:$int)))).
tff(hyst_axiom_33, axiom, (z_plus(z_3(X0:$int),z_3m1(X1:$int)) = z_3m1(z_plus(X0:$int,X1:$int)))).
tff(hyst_axiom_34, axiom, (z_plus(z_3(X0:$int),z_3p1(X1:$int)) = z_3p1(z_plus(X0:$int,X1:$int)))).
tff(hyst_axiom_35, axiom, (z_plus(z_3(X0:$int),z_3(X1:$int)) = z_3(z_plus(X0:$int,X1:$int)))).
tff(hyst_axiom_36, axiom, (z_plus(X0:$int,z_0) = X0:$int)).
tff(hyst_axiom_37, axiom, (z_plus(z_0,X0:$int) = X0:$int)).
tff(hyst_axiom_38, axiom, (z_3(z_0) = z_0)).
tff(set_type, type, (set: $tType)).
tff(element_type, type, (element: $tType)).
tff(empty_set_type, type, (empty_set : set)).
tff(singleton_type, type, (singleton : element > set)).
tff(member_type, type, (member : (element * set) > $o)).
tff(subset_type, type, (subset : (set * set) > $o)).
tff(intersection_type, type, (intersection : (set * set) > set)).
tff(union_type, type, (union : (set * set) > set)).
tff(difference_type, type, (difference : (set * set) > set)).
tff(complement_type, type, (complement : set > set)).
tff(cardinality_type, type, (cardinality : set > $int)).
cnf(empty_set, axiom, (~member(X3:element,X2:set) | X2:set != empty_set)).
cnf(empty_set, axiom, (X1:set = empty_set | member(logtk_sk__0(X1:set),X1:set))).
cnf(singleton, axiom, (member(X1:element,singleton(X3:element)) | X1:element != X3:element)).
cnf(singleton, axiom, (X1:element = X2:element | ~member(X1:element,singleton(X2:element)))).
cnf(subset, axiom, (subset(X1:set,X4:set) | member(logtk_sk__1(X1:set,X4:set),X1:set))).
cnf(subset, axiom, (subset(X1:set,X4:set) | ~member(logtk_sk__1(X1:set,X4:set),X4:set))).
cnf(subset, axiom, (member(X3:element,X2:set) | ~member(X3:element,X1:set) | ~subset(X1:set,X2:set))).
cnf(intersection, axiom, (member(X1:element,intersection(X2:set,X4:set)) | ~member(X1:element,X4:set) | ~member(X1:element,X2:set))).
cnf(intersection, axiom, (member(X1:element,X2:set) | ~member(X1:element,intersection(X2:set,X3:set)))).
cnf(intersection, axiom, (member(X1:element,X3:set) | ~member(X1:element,intersection(X2:set,X3:set)))).
cnf(union, axiom, (member(X1:element,union(X2:set,X4:set)) | ~member(X1:element,X2:set))).
cnf(union, axiom, (member(X1:element,union(X2:set,X4:set)) | ~member(X1:element,X4:set))).
cnf(union, axiom, (member(X1:element,X3:set) | member(X1:element,X2:set) | ~member(X1:element,union(X2:set,X3:set)))).
cnf(difference, axiom, (member(X1:element,difference(X4:set,X2:set)) | member(X1:element,X2:set) | ~member(X1:element,X4:set))).
cnf(difference, axiom, (member(X1:element,X3:set) | ~member(X1:element,difference(X3:set,X2:set)))).
cnf(difference, axiom, (~member(X1:element,X2:set) | ~member(X1:element,difference(X3:set,X2:set)))).
cnf(complement, axiom, (member(X1:element,X3:set) | member(X1:element,complement(X3:set)))).
cnf(complement, axiom, (~member(X1:element,complement(X2:set)) | ~member(X1:element,X2:set))).
cnf(cardinality_empty_set, axiom, (cardinality(X2:set) = z_0 | X2:set != empty_set)).
cnf(cardinality_empty_set, axiom, (X1:set = empty_set | cardinality(X1:set) != z_0)).
cnf(cardinality_intersection_1, axiom, (intersection(singleton(X1:element),X3:set) = singleton(X1:element) | cardinality(union(singleton(X1:element),X3:set)) != cardinality(X3:set))).
cnf(cardinality_intersection_1, axiom, (cardinality(union(singleton(X1:element),X2:set)) = cardinality(X2:set) | intersection(singleton(X1:element),X2:set) != singleton(X1:element))).
cnf(cardinality_intersection_2, axiom, (intersection(singleton(X1:element),X3:set) = empty_set | cardinality(union(singleton(X1:element),X3:set)) != z_plus(cardinality(X3:set),z_3p1(z_0)))).
cnf(cardinality_intersection_2, axiom, (cardinality(union(singleton(X1:element),X2:set)) = z_plus(cardinality(X2:set),z_3p1(z_0)) | intersection(singleton(X1:element),X2:set) != empty_set)).
cnf(cardinality_intersection_3, axiom, (cardinality(intersection(X1:set,X3:set)) = z_0 | intersection(X1:set,X3:set) != empty_set)).
cnf(cardinality_intersection_3, axiom, (intersection(X1:set,X2:set) = empty_set | cardinality(intersection(X1:set,X2:set)) != z_0)).
cnf(cardinality_union, axiom, (intersection(X1:set,X3:set) = empty_set | cardinality(union(X1:set,X3:set)) != z_plus(cardinality(X1:set),cardinality(X3:set)))).
cnf(cardinality_union, axiom, (cardinality(union(X1:set,X2:set)) = z_plus(cardinality(X1:set),cardinality(X2:set)) | intersection(X1:set,X2:set) != empty_set)).
cnf(vc1, negated_conjecture, (~member(logtk_sk__2,logtk_sk__3))).
cnf(vc1, negated_conjecture, (logtk_sk__3 != empty_set | logtk_sk__4 != z_0)).
cnf(vc1, negated_conjecture, (logtk_sk__3 = empty_set | logtk_sk__4 = z_0)).
cnf(vc1, negated_conjecture, (logtk_sk__4 = cardinality(logtk_sk__3))).
