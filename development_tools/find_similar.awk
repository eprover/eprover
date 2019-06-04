#!/opt/local/bin/gawk -f

# Usage: find_similar.awk <prot1> <prot2> <col> [<diff>]
#
# Copyright 2001 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read two extended result files and print a list of all problems
# where both runs differ by less than diff. Also include $col of each
# input file.
#

BEGIN{
   if(!ARGV[3])
   {
      print "Usage: find_similar.awk <prot1> <prot2> <col> [<diff>]";
      exit 1;
   }
   col = int(ARGV[3]);
   ARGV[3] = "";
   diff = 0.1;
   if(ARGV[4])
   {
      diff = ARGV[4];
      ARGV[4] = "";
   }   
}

function abs(num)
{
   if((num+0.0)<0)
   {
      num = -1 * num;
   }
   return num;
}
      
function max(a,b)
{
   if(a>b)
   {
      return a;
   }
   return b;
}


function get_basename(name,     tmp)
{
   if(!match(name, /[A-Z]*[0-9]*[+-][0-9]*(\.[0-9]+)?/))
   {
      print "filter_results.awk: Cannot find problem basename in '" name "'";
   }   
   return substr(name, RSTART , RLENGTH);
}


# Skipping comments

/^#/{
   next;
}

# Process all non-empty lines

/[A-Za-z0-9]+/{
   name = get_basename($0);
   if(ARGIND == 1)
   {
      res1[name] = int($5);
      data1[name] = $col;
   }
   else
   {
      res2[name] = int($5);
      data2[name] = $col;
   }
}

END{
   for(i in res1)
   {
      if(max(res1[i], res2[i]) > 0)
      {
	 d = abs(res1[i] - res2[i])/max(res1[i], res2[i]);
      }
      else
      {
	 d = 0;
      }
      if(d <= diff) 
      {
	 printf("%-20s %-20s %-20s\n", i, data1[i], data2[i]);
      }
   }
}
