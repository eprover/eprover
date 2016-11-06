fof(socrates,  axiom, (philosopher(socrates)|philosopher(plato))).
fof(hume,      axiom, (philosopher(hume))).
fof(phil_wise, axiom, (![X]:(philosopher(X) => wise(X)))).
fof(is_there_wisdom, question, (?[X]:wise(X))).

