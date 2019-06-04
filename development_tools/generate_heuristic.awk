#!/opt/local/bin/gawk -f

# Usage: generate_heuristic.awk <file>
#
#   Given a protokoll file name, generate an entry for the
#   heuristics-string from it.
#
# Copyright 1999 Stephan Schulz, schulz@informatik.tu-muenchen.de


/\-H/{
   pos = index($0, "-H'(");
   part = substr($0, pos+3);
   pos = index(part, "'");
   part = substr(part, 1, pos-1);
   gsub(" ,", ",", part);
   gsub(", ", ",", part);
   gsub("),", "),\\n\"\n\"            ", part);
   pos = index(FILENAME, "_");
   name = substr(FILENAME, pos+1);
   print "\"" gensub(/-/, "_", "g", name) " = " part "\\n\"";
}
