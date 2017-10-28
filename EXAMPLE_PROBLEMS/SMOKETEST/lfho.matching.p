thf(typedef, type, t: $tType).
thf(f_typedef, type, f : t > t > t > t).
thf(g_typedef, type, g: t > t > t).
thf(h_typedef, type, h: t > t).
thf(a_typedef, type, a : t).
thf(b_typedef, type, b : t).
thf(c_typedef, type, c : t).
thf(pred_typedef, type, pred : t > $o).
thf(pred_typedef, type, pred2 : (t > t) > $o).

thf(a1, axiom, pred @ (f @ (g @ a @ b) @ b @ (h @ c))).
thf(a2, axiom, ![X : t, Y : t]: pred @ (f @ X @ b @ Y)).
thf(a2, axiom, ![X : t > t > t]: ~pred @ (X @ a @ b)).
thf(a3, axiom, ![X : t > t > t]: X @ a = h).
thf(a4, axiom, ![X : t > t > t > t]: ~ pred @ (X @ a @ a @ b)).
thf(a5, axiom, f @ c @ a @ b = f @ a @ a @ a).
thf(a6, axiom, ![X : t > t]: ~pred @ (X @ (X @ b))).
thf(a7, axiom, f @ a @ b @ (f @ a @ b @ b) = f @ a @ b @ (f @ a @ b @ a)).
thf(a8, axiom, ![X : t > t, Y: t > t > t, Z: t, W: t]: X @ (Y @ b @ b) = X @ (f @ Z @ W @ Z)).
thf(a9, axiom, ~pred @ (f @ a @ b @ (f @ a @ b @ (f @ a @ b @ b)))).
thf(a10, axiom, f @ a @ b @ a = g @ a @ b).
thf(a11, axiom, ![X : t]: pred2 @ (g @ X)).
thf(a12, conjecture, pred2 @ h).