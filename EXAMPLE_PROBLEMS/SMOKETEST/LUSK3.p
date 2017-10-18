#-----------------------------------------------------------------------------
#   Beispiel:   Lusk 3
#
#   groesseres Beweisbeispiel mit XKBO 
#
#------------------------------------------------------------------------------
#
#   E.L. Lusk & R.A. Overbeek:
#   Reasoning about Equality
#
#   Journal of Automated Reasoning II (1985) 209-228
#
#------------------------------------------------------------------------------
#   Problem 3:  In a ring, if x*x = x for all x in the ring, i
#               then x*y = y*x for all x,y in the ring.
#
#               Funktionen:     f   : Multiplikation *
#                               g   : Inverses
#                               e   : Neutrales Element
#                               a,b : Konstanten
#

cnf(left_identity,  axiom, j(e,X)      = X).                 # e ist a left identity for sum
cnf(right_identity, axiom, j(X,e)      = X).                 # e ist a right identity for sum
cnf(left_inverse,   axiom, j(g(X),X)   = e).                 # there exists a left inverse for sum
cnf(right_inverse,  axiom, j(X,g(X))   = e).                 # there exists a right inverse for sum
cnf(assoc_j,        axiom, j(j(X,Y),Z) = j(X,j(Y,Z))).       # associativity of addition
cnf(commutativ_j,   axiom, j(X,Y)      = j(Y,X)).            # commutativity of addition
cnf(assoc_f,        axiom, f(f(X,Y),Z) = f(X,f(Y,Z))).       # associativity of multiplication
cnf(distrib_1,      axiom, f(X,j(Y,Z)) = j(f (X,Y),f(X,Z))). # distributivity axioms
cnf(distrib_2,      axiom, f(j(X,Y),Z) = j(f (X,Z),f(Y,Z))). #
cnf(xsquaredx,      axiom, f(X,X)      = X).                 # special hypothesis: x*x = x

fof(commutativ_f, conjecture, ![X,Y]:f(X,Y) = f(Y,X)).       # theorem


