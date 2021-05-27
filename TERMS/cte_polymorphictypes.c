
/*-----------------------------------------------------------------------

  File  : cte_polymorphictypes.c

  Author: Florian Knoch, Lukas Naatz

  Contents: 
			 Checks the argument types of all arithmetic operators and predicates and returns the corresponding type.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Tue Mar 16 13:12:30 MET 2021

  -----------------------------------------------------------------------*/

#include "cte_polymorphictypes.h"

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithPred()
//   
//   Checks if both arguments of an arithmetic predicate are 
//   the same and of type int, rat, real
//   Useful for following predicates:
//   $less/2, $lesseq/2, $greater/2, $greatereq/2
//
//   return: bool_type
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithPred(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
				(arg1== types->rational_type) ||
            (arg1== types->real_type)) &&
            arg1==arg2)
	{
		return types->bool_type;
	}
	return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithOp()
//   
//   Checks if both arguments of an arithmetic operator are 
//   the same and of type int, rat, real
//   Useful for following operators:
//   $sum/2, $difference/2, $product/2
//
//   return: type of arg1
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithOp(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
				(arg1== types->rational_type) ||
            (arg1== types->real_type)) &&
            arg1==arg2)
	{
		return arg1;
	}
	return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithNonIntOp()
//   
//   Checks if both arguments of an arithmetic operator are 
//   the same and of type int, rat, real
//   Useful for following operators:
//   $quotient/2
//
//   return: Type of arg1
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithNonIntOp(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->rational_type) ||
            (arg1 == types->real_type)) &&
            arg1 == arg2)
	{
		return arg1;
	}
	return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithIntOp()
//   
//   Checks if both arguments of an arithmetic operator are 
//   the same and of type int, rat, real
//   Useful for following operators:
//   $quotient_e/2, $quotient_t/2, $quotient_f/2
//   $remainder_e/2, $remainder_t/2, $remainder_f/2
//
//   return: integer_type
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithIntOp(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(arg1 == types->integer_type &&
            arg1 == arg2)
	{
		return types->integer_type;
	}
	return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithUnaryOp()
//   
//   Checks if the argument of an arithmetic operator 
//   is of type int, rat, real
//   Useful for following operators:
//   $uminus/1, $floor/1, $ceiling/1, $truncate/1, $round/1, 
//   
//   return: type of arg1
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithUnaryOp(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
		(arg1== types->rational_type) ||
		(arg1== types->real_type)) &&
		arg2 == NULL)
	{
		return arg1;
	}
	return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithIsDomain()
//   
//   Checks if the argument of an arithmetic predicate 
//   is of type int, rat, real
//   Useful for following predicates:
//   $is_int/1, $is_rat/1
//
//   return: bool_type
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithIsDomain(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
		(arg1== types->rational_type) ||
		(arg1== types->real_type)) &&
		arg2 == NULL)
	{
		return types->bool_type;
	}
	return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithToInt()
//   
//   Checks if the argument of an arithmetic operator
//   is of type int, rat, real
//   Useful for following operators:
//   $to_int/1
//
//   return: integer_type
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithToInt(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
		(arg1== types->rational_type) ||
		(arg1== types->real_type)) &&
		arg2 == NULL)
	{
		return types->integer_type;
	}
	return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithToRat()
//   
//   Checks if the argument of an arithmetic operator 
//   is of type int, rat, real
//   Useful for following operators:
//   $to_rat/1
//
//   return: rational_type
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithToRat(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
		(arg1== types->rational_type) ||
		(arg1== types->real_type)) &&
		arg2 == NULL)
	{
		return types->rational_type;
	}
	return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: TypeCheckArithToReal()
//   
//   Checks if the argument of an arithmetic operator 
//   is of type int, rat, real
//   Useful for following operators:
//   $to_real/1
//
//   return: real_type
//
// Global Variables: -
//
// Side Effects    : memory operations.
//
/----------------------------------------------------------------------*/

Type_p TypeCheckArithToReal(TypeBank_p types, Type_p arg1, Type_p arg2)
{
	if(((arg1 == types->integer_type) ||
		(arg1== types->rational_type) ||
		(arg1== types->real_type)) &&
		arg2 == NULL)
	{
		return types->real_type;
	}
	return NULL;
}
