#!/opt/local/bin/gawk -f

# Usage: create_plot.awk max [<file>...<file>]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#

function done_in_time(times, limit,               i, res)
{
   res = 0;

   for(i in times)
   {
      if((0+times[i])<=(0+limit))
      {
	 res++;
      }
   }
   return res;
}

function compute_distrib(times, file,   i)
{
   print "# new file" > file;
   for(i=0; i<=max_time; i+=(max_time/300))
   {
      print done_in_time(times,i) >> file; 
   }
   print "\n\n" >> file;
}


BEGIN{
   if(!ARGV[1] )
   {
      print "Usage: create_plot.awk max [<file>...[<file>]]";
      exit(1);
   }
   max_time = ARGV[1];
   ARGV[1] = "";
   if(!ARGV[2])
   {
      ARGV[2] = "-";
      ARGC++;
   }   
   proc_arg = 0;
   fileno = 0;
   counter = 0;
   plot = "set key bottom\nset term postscript eps monochrome \"Times-Roman\"\nplot ";
   sep = "";
   last_file = "";
   title["protokoll_CW21"] = "1";
   title["protokoll_FFRW21mix"] = "2";
   title["protokoll_TSM36c"] = "3"
   title["protokoll_TSM32"] = "3"
}

# Skipping comments

/^#/{
 next;
}

# Process all non-empty lines

/[A-Za-z0-9]+/{
   if(ARGIND != proc_arg)
   {
      if(fileno)
      {
	 compute_distrib(times, "plotfile" fileno);
	 delete times;
	 first = 0;
	 plot = plot sep "\"plotfile" fileno "\" title \"" title[last_file] "\"";
	 sep = ",";
	 counter = 0;
      }
      proc_arg = ARGIND;
      last_file = ARGV[proc_arg];
      fileno++;
   }
   else if($2!="F")
   {
      times[counter++] = $3;
   }
}


END{
   compute_distrib(times, "plotfile" fileno);
   plot = plot sep "\"plotfile" fileno "\" title \"" title[last_file] "\"";
   print "set data style lines" | "gnuplot";
   print "set yrange [0:]" | "gnuplot";
   print plot | "gnuplot";
   print "Press return to continue" >> "/dev/stderr";
   getline < "/dev/stdin"; 
   close("gnuplot");
   system("rm plotfile*");
}




