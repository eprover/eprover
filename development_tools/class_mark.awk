#!/opt/local/bin/gawk -f

# Usage: class_mark.awk [file]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#


BEGIN{
   pos = 0;
   neg = 0;
}

function classify_term(type, param1, param2)
{
   if(type == "size")
   {
      return (($3+0) > param1);
   }
   else if(type == "depth")
   {
      return (($5+0) > param1);
   }
   else if(type == "subterm")
   {
      return index($1, param1);
   }
   else if(type == "disterm")
   {
      return (index($1, param1)||index($1, param2));
   }
   else if(type == "conterm")
   {
      return (index($1, param1)&&index($1, param2));
   }
   else if(type == "toparity")
   {
      return (index($1, "f" param1)==1);
   }
   else if(type == "topstart")
   {
      return (match($1, param1)&&(RSTART==1));
   }
}


/^#/{
  next;
}


/[A-Za-z0-9]+/{
#   if(classify_term("size",10))
#   if(classify_term("depth",5))
#   if(classify_term("subterm", "f01"))
#   if(classify_term("disterm", "f21(f0", "f11"))
#   if(classify_term("conterm", "f1", "f3"))
#   if(classify_term("toparity", 1))
   if(classify_term("topstart", "f[123].[(]f[01]"))
   {
      pos++;
      print $1 " : 1:(1, 1).";
   }
   else
   {
      neg++;
      print $1 " : 1:(1,-1).";
   }
}



END{
   print "# Pos: " pos " Neg: " neg;
}
