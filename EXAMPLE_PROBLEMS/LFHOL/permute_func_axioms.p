thf(f_type, type, f : $i > $i > $i).
thf(f_type, type, g : $i > $i > $i).


thf(g_permutes_f, axiom, ![X:$i, Y:$i]:((f @ X @ Y) = (g @ Y @ X))).
thf(conj, conjecture, ?[G: $i > $i > $i]: (![X:$i, Y:$i]: ( (f @ X @ Y) = (G @ Y @ X)) ) ).
