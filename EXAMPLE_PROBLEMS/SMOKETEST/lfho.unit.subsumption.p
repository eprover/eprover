thf(typedef, type, t: $tType).
thf(fType, type, f: t > t > t).
thf(gType, type, g: t > t > t).
thf(hType, type, h: t > t > t).
thf(aType, type, a: t).
thf(bType, type, b: t).
thf(cType, type, c: t).

%-- Positive eqs
thf(ep1, axiom, ![X:t]: f @ X = g @ X).
thf(ep2, axiom, ![X:t, Y:t]: f @ X @ Y = h @ Y @ X).
thf(ep3, axiom, ![X: t>t]: X @ (X @ b) = g @ (X @ a) @ (X @ b)).

%-- Clauses

thf(c1, axiom, f @ a @ (f @ a @ b) = g @ (f @ a @ a) @ (f @ a @ b) | a = b).
%-- should be subsumbed by ep3
thf(c2, axiom, f @ a @ b = g @ a @ c).
%-- should not be subsumed
thf(c3, axiom, f @ a @ b = g @ a @ b).
%-- should be subsumbed by ep1
thf(c4, axiom, f @ a @ b = h @ b @ a | a = b).
%-- should be subsumbed by ep2

%-- Negative eqs
thf(en1, axiom, ![Y: t>t]: Y @ a != f @ b @ c).
thf(en2, axiom, ![X: t, Y:t]: g @ X @ Y != h @ Y @ X ).

%-- Clauses
thf(c5, axiom, f @ a @ a != f @ b @  c ).
%-- should be subsumbed by en1
thf(c6, axiom, f @ a @ a != f @ b @ b).
%-- should not be subsumed
thf(c7, axiom, g @ a @ a != h @ a @ b | g @ b @ b != h @ a @ a).
%-- should not be subsumed
thf(c8, axiom, h @ a @ a != g @ b @ b | g @ a @ b != h @ b @ a).
%-- subsumed by en2
thf(c9, axiom, ![X:t, Y:t]: h @ X @ Y != g @ Y @ X).
%-- subsumed by en2
