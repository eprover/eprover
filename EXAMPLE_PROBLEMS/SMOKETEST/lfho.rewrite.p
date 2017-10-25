thf(typedefA, type, tA : $tType).
thf(typeF, type, f: tA > tA > tA).
thf(typeG, type, g: tA > tA > tA).
thf(typeH, type, h: (tA > tA) > tA).
thf(typeG, type, j: tA > tA > tA > tA > tA).
thf(typeH, type, a: tA).
thf(typeH, type, b: tA).
thf(typeH, type, c: tA).
thf(typePred, type, a_predicate: tA > $o).
thf(axiom1, axiom, ![X : tA]: f @ X @ (f @ a @ b) = X).
thf(axiom2, axiom, ![Y : tA > tA]: a_predicate @ (f @ (Y @ c) @ (f @ a @ b))).
thf(axiom3, axiom, ![Z : tA > tA]: Z @ a = f @ (Z @ a) @ (Z @ a)).
thf(axiom4, axiom, a_predicate @ (g @ b @ a)).
thf(axiom5, axiom, ![W : tA > tA]: W @ (W @ b) = h @ W).
thf(axiom6, axiom, ![A : tA > tA > tA]: A @ a = j @ b @ b @ a).
thf(axiom7, axiom, ![A : tA > tA > tA]: a_predicate @ (j @ b @ a @ a @ b)).
thf(conj, conjecture, a_predicate @ (f @ a @ (f @ a @ b))).
