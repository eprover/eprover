#-------------------------------------------------------------
# Equational specification of a group. This specification does have
# models, i.e. it cannot be refuted (and the prover will note this). 
#
# It uses only unit clauses in infix-equational notation.
#

# There exists a right-neutral element (null).
fof(neutral, axiom, ![X]:f(X,null)=X).

# For each X, there is a right inverse element.
fof(inverse, axiom, ![X]:?[Y]:f(X,Y)=null).

# f is associative.
fof(assoc, axiom, ![X,Y,Z]:f(f(X,Y),Z)=f(X,f(Y,Z))).

# Possible Hypothesis: Right inverse is also left inverse
# fof(rieqli, conjecture, ![X]:?[Y]:(f(X,Y)=null & f(Y,X)=null)).
# fof(rieqli, conjecture, ![X,Y]:(f(X,Y)=null => f(Y,X)=null)).

# A nother possible hypothesis: Multiplication with inverse element is
# commutative. 
# ~f(a,i(a)) = f(i(a),a).

