Specification of the PCL 2 inference process description language
-----------------------------------------------------------------


<pcl-listing> ::= <pcl-step>*

<pcl-step> ::= <pcl-id> : <clause> : <just> [:'{'<prover-specific>'}']

<pcl-id> ::= <pos-int> ( .<pos-int> )*

<clause> ::= literal list in TPTP format

<just> ::= 'initial' |
           <pcl-expr> |
           <upcl-expr>

<prover-specific> ::= <ident>|<string>|<pos-int>

<pcl-expr> ::= <pcl-id> |
               <op> '(' <pcl-expr>'('<pos>')'( , <pcl-expr>'('<pos>')')*')'

<upcl-expr> ::= <op> '(' <pcl-expr> ( , <pcl-expr>)*')'

<pos> ::= <pos-int> [. L|R [ .<pos-int> ]*]

Select literal, side, position in term.

Intended semantics: <pcl-expr>s describe a unique inference step and
hence have a unique clause as the result. <upcl-expr>s describe a
class of inferences and the clause dependencies.

<op> ::= 'er' |
         'pm' |
         'ef' |
         'urw' |	 
         'rw' |
         'sr' | 
         'cn' 


SPASS found a proof: SPASS beiseite: Proof found.
