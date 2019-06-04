#!/opt/local/bin/gawk -f

# Usage: find_missing_results.awk <filter_file> [<from_file>]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
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
   if(ARGIND == 1)
   {
      names[get_basename($0)] = 1;
   }
   else
   {
      if(!names[get_basename($0)])
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
