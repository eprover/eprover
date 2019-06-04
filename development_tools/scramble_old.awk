#!/opt/local/bin/gawk -f

# Usage: scramble.awk file 
#
# Copyright 1999 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read file line by line and output the lines in random order.
#


function randint(n) 
{
   return int(n * rand());
}

BEGIN{
   i = 0;
}

{
   array[i++] = $0;
}

function pick_nth(count,   i, k)
{
   for(i in array)
   {
      count--;
      if(count==0)
      {
	 break;
      }
   }
   return i;
}   


END{
   while(i--)
   {
      count = randint(i);
      element = pick_nth(count);
      print array[element];
      delete array[element];
   }
}
