% A check for --proof-object=3, which prints the clause after every
% simplification step. Called as
%
%   ./eprover --proof-object=3 --tstp-format PROOF_OBJECT3.p
%
% the proof has one inference per line, so no line contains
% "[inference(". The unfolding of d, the three rewrites with h(c)=c
% (two in the first arguments and one below g in the third), the
% rewrite with g(c)=c and the final cn each get their own clause.
% With --proof-object=1 the same proof prints the rewrites as one
% nested step cn(rw(rw(rw(rw(...))))).

cnf(def, axiom, d(X) = f(X,X,g(X))).
cnf(gc, axiom, g(c) = c).
cnf(hc, axiom, h(c) = c).
cnf(goal, negated_conjecture, d(h(c)) != f(c,c,c)).
