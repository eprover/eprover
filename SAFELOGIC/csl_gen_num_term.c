

/*-----------------------------------------------------------------------
//
// Function: TermIntRepresentation()
//
//   Generate a term s^i(0) (where s is sig->succc_code and 0 is
//   sig->null_code). This is the elegant recursive version - input is
//   not time-critical anyways.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Term_p TermIntRepresentation(Sig_p sig, long number)
{
   Term_p handle;

   assert(sig);
   assert(number>=0);
   assert(sig->null_code && sig->succ_code);

   if(number==0)
   {
      handle = TermConstCellAlloc(sig->null_code);
   }
   else
   {
      handle = TermDefaultCellAlloc();
      handle->f_code = sig->succ_code;
      handle->arity = 1;
      handle->args = TermArgArrayAlloc(1);
      handle->args[0] = TermIntRepresentation(sig, number-1);
   }
   return handle;
}
