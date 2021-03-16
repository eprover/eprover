/*-----------------------------------------------------------------------

File  : cte_polymorphictypes.h

Author: Florian Knoch, Lukas Naatz

Contents
	
  Functiondefinitions for dealing with agrument checking for arithmetic functions
  
  
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Mar 16 16:36:12 MET DST 2021
    New

-----------------------------------------------------------------------*/


#include <cte_functypes.h>
#include <cte_simpletypes.h>
#include <cte_typebanks.h>

Type_p TypeCheckArithPred(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithOp(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithNonIntOp(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithIntOp(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithToInt(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithUnaryOp(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithIsDomain(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithToRat(TypeBank_p types, Type_p arg1, Type_p arg2);
Type_p TypeCheckArithToReal(TypeBank_p types, Type_p arg1, Type_p arg2);


