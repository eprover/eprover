thf(typedef, type, t : $tType).

thf(fType, type, f : t > t > t > t).
thf(gType, type, g : t > t).
thf(hType, type, h : t > t > t > t > t).
thf(aType, type, a : t).
thf(bType, type, b : t).
thf(cType, type, c : t).
thf(pType, type, p : t > $o).
thf(pType, type, p2 : (t > t) > $o).

%--- "terms" are arguments to p

%--- those two terms should be unifiable with unifer 
%--- X |-> b, Z |-> Y (or Y |-> Z)
thf(a1, axiom, ![X:t, Y:t]: p @ (f @ X @ a @ Y)).
thf(a2, axiom, ![Z:t]: p @ (f @ b @ a @ Z)).

%--- those two terms are not unifiable.
thf(a3, axiom, ![X:t]: p @ (f @ X @ a @ X)).
thf(a4, axiom, p @ (f @ a @ a @ b)).

%--- those two terms are unifiable with unifier
%--- X |-> a, Y |-> b, Z |-> c
thf(a5, axiom, ![X:t, Y:t, Z:t]: p @ (f @ (f @ X @ Y @ Z) @ X @ Y)).
thf(a6, axiom, p @ (f @ (f @ a @ b @ c) @ a @ b)).

%--- term in a5 and a7 are non unifiable
thf(a7, axiom, ![X:t, Y:t, Z:t]: p @ (f @ (f @ a @ b @ c) @ b @ b)).

%--- terms are unifiable with empty unifier and some remains
thf(a8, axiom, p2 @ (f @ (g @ b) @ (g @ c))).
thf(a9, axiom, p @ (f @ (g @ b) @ (g @ c) @ (g @ a))).

%--- unifiable with unifier {X |-> f a b, Y |-> c}
thf(a10, axiom, ![X: t > t]: p @ (f @ (X @ a) @ (X @ b) @ c)).
thf(a11, axiom, ![Y: t]: p @ (f @ (f @ a @ b @ a) @ (f @ a @ b @ b) @ Y)).

%--- non-unifiable
thf(a12, axiom, p @ (f @ a @ b @ c)).
thf(a13, axiom, p @ (g @ a)).

%-- unifiable with unifier X |-> f a 
thf(a14, axiom, ![X: t > t > t]: p @ (X @ a @ b)).
thf(a15, axiom, p @ (f @ a @ a @ b)).

%-- a14 unifiable with a16 with X |-> Y a
thf(a16, axiom, ![Y: t > t > t > t]: p @ (Y @ a @ a @ b)).

%-- unifiable with unifier X |-> h a b, Y |-> Z, W |-> a, U |-> b
thf(a17, axiom, ![X: t > t > t, Y: t > t]: p @ (X @ (Y @ c) @ (X @ a @ b))).
thf(a18, axiom, ![Z: t > t, W: t, U: t]: p @ (h @ a @ b @ (Z @ c) @ (h @ a @ b @ W @ U))).

%-- unifiable with unifier Z |-> X a, Y |-> c
thf(a19, axiom, ![X: t > t > t > t, Y: t]: p @ (X @ a @ Y @ b)).
thf(a20, axiom, ![Z: t > t > t, W:t]: p @ (Z @ c @ W)).