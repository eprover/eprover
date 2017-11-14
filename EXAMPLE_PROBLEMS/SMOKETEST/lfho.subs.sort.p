thf(typedef, type, a: $tType).
thf(cType, type, b: a).
thf(cType, type, c: a).
thf(dType, type, d: a).
thf(fType, type, f: a > a > a > a).
thf(gType, type, g: a > a).
thf(hType, type, h: a > a > a).

thf(a1, axiom, ![X1: a > a, X2: a > a > a]: ( (X1 @ c = X1 @ d) |  (X2 @ c @ d = X2 @ d @ c))).
%-- will not be subsumed. no substitution can cover all the things
thf(a2, axiom, ![X1: a > a, X3: a > a > a > a]: ((X3 @ c @ c @ d = X3 @ c @ d @ c) | (f @ c @ c @ d = f @ c @ d @ c) 
												 | (g @ c = g @ d) | (h @ c @ c = h @ c @ d) | (X1 @ c = X1 @ d))).
%-- subsumed with substitution {X1 <- X1 @ c, X2 <- f @ b}
thf(a3, axiom, ![X1: a > a > a, X2:a, X3: a > a]: ( (X1 @ c @ c = X1 @ c @ d) | (f @ b @ c @ d = f @ b @ d @ c))).
%-- will not be subsumed. no substitution can cover all the things
thf(a4, axiom, ![X1: a > a]: ((X1 @ c = X1 @ d) | (f @ c @ c @ d = f @ c @ d @ c) | (g @ c = g @ d) |  (h @ c @ c = h @ c @ d))).

thf(a5, axiom, ![X1: a > a, X2: a> a> a, X3: a> a> a]: ((X1 @ d != X2 @ c @ d) | (f @ d @ d @ c != g @ c) | (X3 @ d @ d = X3 @ c @ c))).
%-- subsumed with substituition {X1 <- f @ c, X2 <- g, X3 <- g}
thf(a6, axiom, (f @ c @ d @ d!= h @ c @ d) | (f @ d @ d @ c != g @ c) | (h @ d @ d = h @ c @ c)).
%-- no substitution, polarities are wrong
thf(a6Prime, axiom, (f @ c @ d @ d = h @ c @ d) | (f @ d @ d @ c = g @ c) | (h @ d @ d != h @ c @ c)).
%-- subsumed with substitution {X1 <- X1 @ d, X3 <- d}
thf(a7, axiom, ![X1: a > a > a, X2: a> a> a, X3: a> a> a]: ( (X1 @ d @ d != X2 @ c @ d) | (f @ d @ d @ c != g @ c) | (h @ d @ d = h @ c @ c))).
