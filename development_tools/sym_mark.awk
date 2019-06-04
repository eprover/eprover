#!/opt/local/bin/gawk -f

# Usage: sym_mark.awk [file]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
#

BEGIN{
   pos = 0;
   neg = 0;
   toggle=0;
   count=0;
}

function classify_term()
{
   return $7=="s";
}


/^#/{
   next;
}


/[A-Za-z0-9]+/{
   if(classify_term())
   {
      pos++;
      print $1 " : 1:(1, 1).";
   }
   else 
   {
      if(toggle)
      {
	 pos++;
	 print "f2" count "(" $1 "," $1, ") : 1:(1, 1).";
	 count++;
	 if(count == 2)
	 {
	    count = 0;
	 }
      }
      else
      {
	 neg++;
	 print $1 " : 1:(1,-1).";
      }
   }
   toggle = !toggle;
}



END{
   print "# Pos: " pos " Neg: " neg;
}
