#!/opt/local/bin/gawk -f

# Usage: class_auswert.awk
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
#

BEGIN{
      ar      = "";
      symb    = "";
      id      = "";
      atop1   = "";
      top2    = "";
      atop2   = "";
      estop3  = "";
      atop4   = "";
      opt     = "";
}

/IndexArity/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   ar  = ar tmp;
}

/IndexSymbol/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   symb = symb tmp;
}

/IndexIdentity/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   id  = id tmp;
}

/IndexAltTop -d1/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   atop1 = atop1 tmp;
}

/IndexTop -d2/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   top2 = top2 tmp;
}

/IndexAltTop -d2/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   atop2 = atop2 tmp;
}

/IndexAltTop -d4/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   atop4 = atop4 tmp;
}

/IndexESTop -d3/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   estop3 = estop3 tmp;
}


/IndexDynamic/{
   tmp = sprintf("& %.1f$\\pm$%.2f ", $3, substr($4,4));
   opt = opt tmp;
}



END{
   print ar "\\\\";
   print symb "\\\\";
   print id "\\\\";
   print atop1 "\\\\";
   print top2 "\\\\";
   print atop2 "\\\\";
   print estop3 "\\\\";
   print atop4 "\\\\";
   print opt "\\\\";
}
