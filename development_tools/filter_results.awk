#!/opt/local/bin/gawk -f

# Usage: filter_results.awk <filter_file> [<from_file>]
#
# Copyright 1998, 2016 Stephan Schulz, schulz@eprover.org
#
# Read file of TPTP problem names and a second file, return all lines
# from the second file in which a name from the first file
# occurs. Also pass through comments.
#

BEGIN{
   if(!ARGV[1])
   {
      print "Usage: filter_results.awk <filter_file> [<from_file>]";
      exit 1;
   }
   if(!ARGV[2])
   {
      ARGV[2] = "-";
      ARGC++;
   }   
}

function get_basename(name,     tmp)
{
   if(!match(name, /\.[a-z]*( |$)/))
   {
      print "filter_results.awk: Cannot find problem basename in '" name "'";
   }   
   return substr(name, 0 , RSTART-1);
}


# Skipping comments

/^#/{
   if(ARGIND==2)
   {
      print
   }
   next;
}

# Process all non-empty lines

/[A-Za-z0-9]+/{
   if(ARGIND == 1)
   {
      names[$1] = 1;
   }
   else
   {
      if(names[$1])
      {
	 print;
	 # delete names[get_basename($0)];
      }
   }
}

#END{
#   print "Remaining:";
#   for(i in names)
#   {
#      print i;
#   }
#}
