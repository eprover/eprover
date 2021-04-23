thf(list_decl, type, list: $tType).
thf(nil_decl, type, nil: list).
thf(cons_decl, type, cons: $i > list > list).
thf(map_decl, type, map: ($i > $i) > list > list).
thf(a_decl, type, a : $i).
thf(b_decl, type, b : $i).
thf(c_decl, type, c : $i).
thf(d_decl, type, d : $i).
thf(f_decl, type, f : $i > $i > $i).
thf(l_decl, type, l : list).
thf(res_decl, type, res : list).

thf(map_nil, axiom, ![F:$i>$i]: ((map @ F @ nil) = nil)).
thf(map_cons, axiom, ![F:$i>$i, X:$i, XS: list]: ((map @ F @ (cons @ X @ XS)) = (cons @ (F @ X) @ (map @ F @ XS)))).

thf(f_def, axiom, ![X:$i]:((f @ a @ X) = d)).

thf(list_a_b_c, axiom, l = (cons @ a @ (cons @ b @ (cons @ c @ nil)))).
thf(list_d_d_d, axiom, res = (cons @ d @ (cons @ d @ (cons @ d @ nil)))).


thf(map_a_b_c, conjecture, (map @ (f @ a) @ l) = res).
