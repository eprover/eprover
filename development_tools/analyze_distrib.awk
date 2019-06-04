#!/opt/local/bin/gawk -f

# Usage: analyze_distrib [<from_file>]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a line of data and generate a distribution for hard-coded
# fields. 
#


# Skipping comments

/^#/{
  next;
}

# Process all non-empty lines

/[A-Za-z0-9]+/{
   res = 0+$5;
   dist_5[res]++;
   res = 0+$6;
   dist_6[res]++;
   res = 0+$7;
   dist_7[res]++;
}

END{
   for(i in dist_5)
   {
      printf "dist_5[%4ld]=" dist_5[i] "\n", i;
   }
   for(i in dist_6)
   {
      printf  "dist_6[%4ld]=" dist_6[i] "\n", i;
   }
   for(i in dist_7)
   {
      printf "dist_7[%4ld]=" dist_7[i] "\n", i;
   }
}
