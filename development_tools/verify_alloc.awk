#!/sw/bin/gawk -f
#
# Usage: some_clib_program | verify_alloc.awk
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Interpret the debugging output of a program using the CLIB memory
# management routines (in BASICS/clb_memory.[ch]) and compiled with
# -DCLB_MEMORY_DEBUG2. 
# 
# verify_alloc.awk reports doubly allocated blocks (hopefully never,
# as this would indicate a bug in CLIB), blocks freed that are
# currently not allocated (this catches blocks freed more than once),
# block allocated with a certain size and freed with a different one, 
# and finally blocks that are allocated but never freed.
#
# This works only with memory handled via SizeMalloc()/SizeFree().
#

/Block /{
   if($3=="A:")
   {
      if(mem_array[$2])
      {
	 print $0 " doubly allocated";
	 fail_array[count++] = $0 " DA";
      }
      else
      {
	 mem_array[$2] = $5;
      }
   }
   else
   {
      if(!mem_array[$2])
      {
	 print $0 " freeed but not allocated";
	 fail_array[count++] = $0 " FN";
      }
      else
      {
	 if(mem_array[$2] != $5)
	 {
	    print $0 " freed with wrong size " mem_array[$1];
	    fail_array[count++] = $0 " WS";
	   }
	 delete mem_array[$2];
      }
   }
}

!/Block / && /.+/{
   print $0;
}

END{
   for(i in mem_array)
   {
      print "Remaining: " i " " mem_array[i];      
   }
   print "Failure list:";
   for(i in fail_array)
   {
      print fail_array[i];
   }
}







