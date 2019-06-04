#!/opt/local/bin/gawk -f

# Usage: rand_mark.awk [file]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
#

BEGIN{
   pos = 0;
   neg = 0;
   toggle=0;
}

function classify_term()
{
   return $7=="s";
}


/^#/{
   next;
}


/[A-Za-z0-9]+/{
   if(toggle)
   {
      if($2)
      {
	 for(i=0; i<$1; i++)
	 {
	    print $2 " : 1:(1, 1).";
	    pos++;
	 }
      }
      else
      {
	 print $1 " : 1:(1, 1).";
	 pos++;
      }
   }	
   else
   {
      if($2)
      {
	 for(i=0; i<$1; i++)
	 {
	    print $2 " : 1:(1, -1).";
	    neg++;
	 }
      }
      else
      {
	 print $1 " : 1:(1, -1).";
	 neg++;
      }
   }
   toggle = !toggle;
}



END{
   print "# Pos: " pos " Neg: " neg;
}
