thf(typedef, type, t: $tType).
thf(fType, type, f: t > t > t).
thf(gType, type, g: t > t > t).
thf(hType, type, h: t > t).
thf(iType, type, i: t > t).
thf(aType, type, a: t).
thf(bType, type, b: t).
thf(cType, type, c: t).
thf(pType, type, p: t > $o).
thf(pType, type, q: t > $o).

thf(a1, axiom, ![X: t>t, Y: t>t, Z:t]: (X @ a = Y @ b | f @ a @ Z = g @ b @ Z)).
%-- should be subsumed by a1
thf(a2, axiom, p @ a | h @ a = i @ b | f @ a @ b = g @ b @ b | ![X:t>t>t]: X@a@b = X@b@b | ![Y:t>t>t>t]: Y@a@b@c = Y@b@c@c).
%-- shouldn't be subsumed
thf(a3, axiom, f @ a = g @ b | f @ a @ c = g @ b @ c | ![X:t>t>t]: X @ (X@a@b) @ (X@b@c) = X @ (X @ a @ a) @ (X @ a @ a)).

thf(a4, axiom, ![X:t, Y:t]: (f @ X @ Y = g @ (f @ X @ Y) @ (h @ Y) | h @ X = f @ (h @ X) @ (h @ X))).
%-- should be subsumed by a4
thf(a5, axiom, p @ a | f @ a @ b = g @ (f @ a @ b) @ (h @ b) | h @ a = f @ (h @ a) @ (h @ a)).
%-- shouldn't be subsumed
thf(a6, axiom, p @ a | f @ a @ b = g @ (f @ a @ b) @ (h @ b) | h @ c = f @ (h @ c) @ (h @ c)).

thf(a7, axiom, ![X:t>t, Y:t, Z:t]: X @ (X @ a) = g @ b @ (X @ b) | f @ (i @ Y) @ (i @ Z) = h @ (g @ Y @ Z)).
%-- subsumed by a7
thf(a8, axiom, f @ a @ (f @ a @ a) = g @ b @ (f @ a @ b) | f @ (i @ c) @ (i @ c) = h @ (g @ c @ c) | p @ a).
%-- should not be subsumed
thf(a9, axiom, f @ a @ (f @ a @ a) = g @ b @ (f @ b @ b) | f @ (i @ c) @ (i @ c) = h @ (g @ c @ c) | p @ a).
%-- should not be subsumed
thf(a10, axiom, f @ a @ (f @ a @ a) = g @ b @ (f @ a @ b) | f @ (i @ c) @ (i @ c) = h @ (g @ a @ a) | p @ a).