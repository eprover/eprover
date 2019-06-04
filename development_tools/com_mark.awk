#!/opt/local/bin/gawk -f

# Usage: com_mark.awk [file]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
#

BEGIN{
   pos = 0;
   neg = 0;
   toggle=0;
   count2=0;
   count1=0;
}

function classify_term()
{
   return $9=="s";
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
	 print "f2" count2 "(f1" count1 "(" $1 ")," $1, ") : 1:(1, 1).";
	 count1++;
	 if(count1 == 2)
	 {
	    count1 = 0;
	 }
	 count2++;
	 if(count2 == 3)
	 {
	    count2 = 0;
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
