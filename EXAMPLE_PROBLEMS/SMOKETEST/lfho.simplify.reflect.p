thf(typedef, type, t : $tType).
thf(fType, type, f: t > t > t > t).
thf(gType, type, g: t > t > t > t).
thf(aType, type, h: t > t).
thf(aType, type, a: t).
thf(bType, type, b: t).
thf(cType, type, c: t).
thf(dType, type, d: t).

%--- TEST CASES FOR POSITIVE SIMPLIFY REFLECT
thf(a1 , axiom, f @ a @ b = g @ c @ d ).
thf(a2 , axiom, ![X: t > t > t]: X @ c = h).
thf(a3 , axiom, ![X:t]: f @ X @ X = g @ X @ b).
thf(a4 , axiom, c = d).
thf(a5 , axiom, f @ a @ b @ c != g @ c @ d @ c | f @ a @ b @ d != g @ c @ d @ c | f @ a @ a @ c != h @ b).
%--- WHAT SHOULD STAY OF a5:
%--- f @ a @ b @ d = g @ c @ d @ c | f @ a @ a @ c = h @ b

thf(a6 , axiom, ![X:t]: f @ X @ X @ a != g @ X @ b @ a | f @ a @ a @ c != g @ c @ b @ c).
%--- WHAT SHOULD STAY OF a6:
%--- f @ a @ a @ c = g @ c @ b @ c

thf(a7 , axiom, g @ a @ c @ d != h @ d | g @ a @ c @ d != h @ c | g @ a @ a @ a != h @ b).
%--- WHAT SHOULD STAY OF a7:
%--- g @ a @ c @ d = h @ c

thf(a77 , axiom, ![Y: t > t > t > t > t, Z: t > t > t, W: t > t > t]: Y @ a @ b @ c @ d != h @ d | Z @ c != h | W @ c @ a != h @ a).
%--- WHAT SHOULD STAY OF a77:
%--- W @ c @ a = h @ b

%--- TEST CASES FOR NEGATIVE SIMPLIFY REFLECT 
thf(a8 , axiom, ![X: t, Y:t]: f @ X != g @ Y).
thf(a9 , axiom, ![X: t]: h @ X != g @ X @ a @ b).
thf(a10 , axiom, f @ a = g @ b | f @ a @ b @ c = g @ b @ b @ c).
%--- WHAT SHOULD STAY OF a10:
%--- f @ a @ b @ c = g @ b @ b @ c
thf(a11 , axiom, h @ c = g @ c @ a @ b | h @ c = g @ d @ a @ b).
%--- WHAT SHOULD STAY OF 11:
%--- h @ c = g @ d @ a @ b


