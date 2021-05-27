This README lists the major changes made to implement the parsing and normalisation of arithmetic functions:

Type checking is done in the files cte_polymorphictypes.[ch].
Function and AC-normalisation is done in the files ccl_arithnorm.[ch].
The signature was expanded to recognise all arithmetic function symbols and to handle them properly.
Some changes were made in the files cte_termbanks.c and cte_typecheck.c, so arithmetic functions get their proper types assigned.
Many assertions were commented out, to make ad-hoc polymorphism work properly.
The AC-normalisation was added to cco_forward_contraction.c, so new clauses will get normalised.
