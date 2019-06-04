#!/opt/local/bin/gawk -f

# Usage: eliminate_protocolls.awk <input_file>
#
# Copyright 1999 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
#   Read the output of find_optimal.awk and eliminate all protocol
#   lines not matching one of the hard-coded patterns.
#

BEGIN{
   wanted["3b"] = 1;
   wanted["Ba"] = 1;
   wanted["2c"] = 1;
   wanted["7c"] = 1;
   wanted["7a"] = 1;
   wanted["Cb"] = 1;
   wanted["Eb"] = 1;
#   wanted["X"] = 1;
}

/^  protokoll/{
   praekey = substr($0, 18);
   start = match(praekey, /[0-9A-Z].*:/);
   if(start)
   {
      key = substr(praekey, RSTART, RLENGTH-1);
	 if(wanted[key])
	 {
	    print;
	 }
   }
   next;
}

{
   print;
}
