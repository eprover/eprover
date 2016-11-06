# This will make E produce proofs using all inference rules but "ef"
# if called as follows:
# ./eprover --split-clauses=3 --forward-context-sr --proof-object ALL_RULES.lop 
# ./eprover --split-clauses=3 --forward-context-sr
#           --strong-destructive-er --destructive-er-aggressive 
#           --proof-object ALL_RULES.lop 
# ./eprover --condense --proof-object ALL_RULES.lop 
#
# "er"     Equality resolution: x!=a v x=x ==> a=a
# "pm"     Paramodulation
# "ef"     Equality factoring (BG94): x=a v b=c v x=d ==> 
#                                     a!=c v b=c vb=d  
# "split"  Clause splitting a la Vampire (non-deductive, but maintains
#          unsatisfiability)  
# "rw"     Rewriting, can mean repeated application (but only of one
#          equation in one direction
# "sr"     Simplify-reflect: a=b and f(a)!=f(b) => empty clause
# "csr"    Contextual simplify-reflect
# "ar"     AC-resolution: Delete literals that are trivial modulo the
#          AC-theory induced by the named clauses
# "cn"     Clause normalize, delete trivial and repeated literals


cnf(identity,axiom,(i(X1)=i(X2))).
cnf(comm_f,  axiom,(f(X1,X2)=f(X2,X1))).
cnf(comm_g,  axiom,(g(X1,X2)=g(X2,X1))).
cnf(ass_f,   axiom,(f(f(X1,X2),X3)=f(X1,f(X2,X3)))).
cnf(p_holds, axiom,p(X)).

cnf(consts1, axiom,(a=b|c=a|e=a)).
cnf(consts2, axiom,(a=b|c=a|e!=a)).
cnf(split_or_condense, axiom,(c=b|X3!=X4|X1!=X2|d!=c)).
# cnf(guarded_eq, axiom,(d=c|h(i(e))!=h(i(a)))).

fof(guarded_eq, axiom,((d=c|d=c) <=> h(i(e))=h(i(a)))).


fof(conj, conjecture,(?[X1,X2,X3,X4,X5]:
          ((k(a,b)=k(X1,X1)&
          f(f(g(X4,X5),X3),f(X2,X1))=f(f(X1,X2),f(X3,g(X4,X5))))))
          &![X]:p(X)).

