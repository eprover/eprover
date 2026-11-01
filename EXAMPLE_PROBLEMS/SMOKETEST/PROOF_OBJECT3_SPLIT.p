% A check for --proof-object=3 on clause splitting. Called as
%
%   ./eprover --split-clauses=7 --proof-object=3 --tstp-format PROOF_OBJECT3_SPLIT.p
%
% the first clause splits into three variable-disjoint parts. Each
% application of a part's definition gets its own clause, so no line
% contains "apply_def,[status(thm)],[inference(". With --proof-object=1
% the three applications print as one nested apply_def step.

cnf(c1, axiom, p(X) | q(Y) | r(Z)).
cnf(c2, axiom, ~p(a)).
cnf(c3, axiom, ~q(b)).
cnf(c4, axiom, ~r(c)).
