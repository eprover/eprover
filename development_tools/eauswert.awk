#!/opt/local/bin/gawk -f

# Usage: eauswert.awk <result_file>
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a protocol file (as e.g. written by distribute_eprover.awk)
# and summarize it, reporting successes, timeouts, out-of
# memory-failures and potential bugs.
#

BEGIN{
  ttime = 0.0;
  if(!ARGV[2])
  {
    proc_limit = int(1000000); /* Large enough */
  }
  else
  {
    proc_limit = ARGV[2];
    sub("0*", "" , proc_limit);
    proc_limit = int(proc_limit);
  }
  
  if(!ARGV[3])
  {
     timelimit = 1000000.0; /* Large enough */
  }
  else
  {
    timelimit = ARGV[3];
    sub("0*", "" , timelimit);
    timelimit = float(timelimit);
  }
  succ = 0;
  total = 0;
  bugs = 0;
  nomem = 0;
  incomplete=0;
  ARGC=2;
  
  if(ARGV[1]=="-")
    {
      ARGV[1]="";
    }

  if(ARGV[1])
    {
      test1 = getline < ARGV[1];      
      close(ARGV[1]);
      if(test1 == -1)
	{
	  if(!index(ARGV[1],"PROTOCOL"))
	    {
	      if(substr(ARGV[1], length(ARGV[1]))!="/")
		{
		  ARGV[1] = ARGV[1] "/";
		}
	      ARGV[1] = ARGV[1] "PROTOCOL";
	    }
	}
    }
}


function float(x)
{
   return 0.0+x+0.0;
}

!(/^#/) && !(/^%/){
   if((total < proc_limit))
   {
      total++;
      if(($2 == "F")||((float($3))>= timelimit))
      {
	 if($4 == "unknown")
	 {
	    bugs++;
	 }
	 if($4 == "maxmem" && ((float($3)) < timelimit))
	 {
	    nomem++;
	 }
	 if($4 == "incomplete" && ((float($3)) < timelimit))
	 {
	    incomplete++;
	 }
      }
      else
      {
	 /* if(($2 == "N")) */
	 {
	    succ++;
	    ttime+=$3;
	 }
      }
   }
}

END{
  printf "Total : %4d S/F/I/N/B: %5d/%5d/%3d/%3d/%3d Time: %f\n", 
         total, succ, total-succ, incomplete, nomem, bugs, ttime;
}
