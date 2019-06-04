#!/opt/local/bin/gawk -f

# Usage: scramble.awk file 
#
# Copyright 1999 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read file line by line and output the lines in random order.
#


function get_pid(     tmp)
{
   tmp = PROCINFO["pid"];
   # getline tmp < "/dev/pid";
   #   close("/dev/pid");
   if(!tmp)
   {
      print "scramble.awk: Cannot get PID ?!?" > "/dev/stderr";
      exit 1;
   }
   return tmp;
}

function get_hostname(   pipe, tmp)
{
   pipe = "hostname";
   pipe | getline tmp;
   close(pipe);
   return tmp;
}

function get_tmpname(   file)
{
   file = "__scramble__" get_hostname() "__" get_pid() "__";
   return file;
}

BEGIN{
   srand(10);
}

{
   key = rand();
   while(array[key])
   {
      key = rand();
   }
   array[key] = $0;
}



END{
   procid = get_pid();
   file = get_tmpname();
   for(i in array)
   {
      printf("%1.12f %s\n", i, array[i]) > file ;
   }
   close(file);
   cmd = "sort " file;
   while ((cmd | getline tmp) > 0)
   {
      print substr(tmp,16);
   }
   system("rm " file);
}
