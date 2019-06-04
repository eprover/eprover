#!/opt/local/bin/gawk -f

# Usage: find_missing_results.awk <filter_file> [<from_file>]
#
# Copyright 1998, 2016 Stephan Schulz, schulz@eprover.org 
#
# Read file of TPTP problem names and a second file, return all lines
# from the second file in which no name from the first file
# occurs. This is dual to filter_results.awk.
#

BEGIN{
   if(!ARGV[1])
   {
      print "Usage: find_missing_results.awk <filter_file> [<from_file>]";
      exit 1;
   }
   if(!ARGV[2])
   {
      ARGV[2] = "-";
      ARGC++;
   }   
}


# Skipping comments

/^#/{
   if(ARGIND != 1)
   {
      print;
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
      if(!names[$1])
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
