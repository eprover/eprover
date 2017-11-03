tff(typedef, type, t: $tType).
tff(fType, type, f : (t * t) > t).
tff(gType, type, g : (t * t) > t).
tff(hType, type, h : t  > t).

tff(a1, axiom, ![X, Y]: f(X,Y) = g (f(X,Y), h(Y)) | h(X) = f(h(X), h(X))).
tff(c, conjecture, p(h(X))).
