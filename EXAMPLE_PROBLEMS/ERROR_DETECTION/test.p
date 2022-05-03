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

fof(weallÜdie,      axiom, ![X]:(human(X) => mortal(X))).
fof(wealldie,      axiom, ![X]:(human(X) => mortal(X))).
fof/wealldie,      axiom, ![X]:(human(X) => mortal(X))).
fof(socrateshuman, axiom, ![X]:(human(socrates))).
fof(socratesdies,  conjecture, mortal(socrates)).