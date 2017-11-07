thf(typedef, type, t : $tType).
thf(f0Type,  type, f0: t).
thf(f1Type,  type, f1: t > t).
thf(f2Type,  type, f2: t > t > t).
thf(f3Type,  type, f3: t > t > t > t).
thf(g1Type,  type, g1: t > t).
thf(g2Type,  type, g2: t > t > t).
thf(hType,  type,  h: t > t > t).
thf(aType,  type,  a: t).
thf(bType,  type,  b: t).
thf(cType,  type,  c: t).
thf(dType,  type,  d: t).
thf(eType,  type,  e: t).
thf(iType,  type,  i: t).
thf(jType,  type,  j: t).
thf(kType,  type,  k: t).
thf(pType,  type,  p: t > $o).
thf(p2Type,  type, p2: (t > t) > $o).

%-- setting precedence in E --precedence=
%-- h > g1 > g2 > f3 > f2 > f1 > f0 > i > j > k > a > b > c > d 

%-- setting weights in E --order-weights=
%-- f3:1,f2:1,f1:1,f0:1,h:2,g1:1,g2:2,a:1,b:1,c:2,d:2,i:1,j:1,k:2
%-- orient left > right
thf(a1, axiom, g1 @ (f1 @ (f1 @ f0)) = f2 @ (f1 @ f0) @ f0).

%-- up should be bigger than down -- by weights
thf(a2, axiom, p @ (f3 @ a @ b @ c)).
thf(a3, axiom, p2 @ (f3 @ a @ b)).

%-- weight the same -- length-lexicographic wins up < down
thf(a4, axiom, p2 @ (f3 @ c @ d)).
thf(a5, axiom, p @ (f3 @ i @ j @ k)).

%-- app var -- lexicographic wins left > right
thf(a6, axiom, ![X: t>t>t]: X @ a @ b = X @ b @ a).

%-- different app var -- non comparable
thf(a7, axiom, ![X: t>t>t, Y: t>t>t]: X @ a @ b = Y @ a @ b).

%-- not both are app vars -- non comparable
thf(a8, axiom, ![X: t>t>t]: X @ a @ b = f2 @ a @ b).

%-- more complicated case of lexicographic comparison l > r
thf(a9, axiom, ![X: t>t>t]: f3 @ (X @ a @ b) @ c @ d = f3 @ (X @ a @ b) @ d @ c).

%-- another complicated case of lexicographic comparison l < r cause w(arg1 ) = w(arg1) and g2 > f3
thf(a10, axiom, ![X: t, Y:t, Z:t, W:t>t>t]: h @ (f3 @ X @ Y @ Z) @ (W @ c @ d) = h @ (g2 @ X @ c) @ (f3 @ a @ b @ c)).

%-- one where var condition fails -- uncomparable
thf(a11, axiom, ![X: t, Y:t, Z:t]: f2 @ X @ Z = f2 @ Y @ X).
