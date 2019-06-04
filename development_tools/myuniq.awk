#!/opt/local/bin/gawk -f

# Usage: myuniq.awk file 
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read file line by line, return the first line of each set with the
# same $1. Comments (line with #) are passed on unmodified!
#

BEGIN{
   key = "";
}

/#/{
   print;
   next;
}

{
   if($1!=key)
   {
      print;
      key = $1;
   }
}

