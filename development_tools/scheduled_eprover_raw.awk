#!/opt/local/bin/gawk -f

# Usage: scheduled_eprover.awk <spec_file>
#
# Copyright 1998--2006 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a specification file describing a set of parameters and a list
# of problems, and run e_scheduler.py over the problems. 


function min(a,b)
{
  if(a>b)
    {
      return b;
    }
  return a;
}

function get_hostname(   pipe, tmp)
{
   pipe = "hostname";
   pipe | getline tmp;
   close(pipe);
   return tmp;
}

function get_pid(     tmp)
{
   getline tmp < "/dev/pid";
   close("/dev/pid");
   if(!tmp)
   {
      print "scheduled_eprover_raw.awk: Cannot get PID ?!?" > "/dev/stderr";
      exit 1;
   }
   return tmp;
}


function expand_file(name,            pipe, tmp)
{
   pipe = "echo " name;
   pipe | getline tmp;
   close(pipe);
   if(!tmp)
   {
      print "scheduled_eprover_raw.awk: Cannot expand filename" > "/dev/stderr";
      exit 1;
   }
   return tmp;
}

  
function file_exists(file,    test, tmp)
{
  test = getline tmp < file;      
  close(file);
  if(test == -1)
    {
      return 0;
    }
  return 1;
}


function check_and_initialize(    tmp, job)
{
  if(first_job)
    {
      if(!executable)
	{
	  print "scheduled_eprover_raw.awk: No executable found" > "/dev/stderr";
	  exit 1;
	}
      if(!logfile)
	{
	  print "scheduled_eprover_raw.awk: No logfile found" > "/dev/stderr";
	  exit 1;
	}
      if(!problemdir)
	{
	  print "scheduled_eprover_raw.awk: No problemdir found" > "/dev/stderr";
	  exit 1;
	}
      if(!file_exists(logfile))
	{
	  print "Creating log file " logfile
	  print "# " args " " auto_args > logfile;
	  tmp = "which " executable;
	  tmp | getline execpath;
	  close(tmp);
	  tmp = "ls -l " execpath;
	  tmp | getline execdate;
	  close(tmp);
	  print "# " substr(execdate, 42) > logfile;
	}
      else
	{
	  print "Logfile " logfile " exists, using old results";
	  
	  processed_count = 0;
	  while ((getline tmp < logfile) > 0)
	  {
	     if(match(tmp, /^[0-9A-Za-z].*\.lop/) ||
                match(tmp, /^[0-9A-Za-z].*\.p/)||   
                match(tmp, /^[0-9A-Za-z].*\.e/)  || 
                match(tmp, /^[0-9A-Za-z].*\.tpt/)  ||
                match(tmp, /^[0-9A-Za-z].*\.tptp/))
	     {
		job = substr(tmp, RSTART, RLENGTH);
		processed_jobs[job] = 1;
		processed_count++;
	     }
	  }
	  close(logfile);
	  print "Found " processed_count " old results";
	}	      
      first_job = 0;	    
    }  
}

function host_cpu_limit_opt(limit)
{
   res = sprintf("--cpu-limit=%d", limit);

   return res;
}



function process_result(job,    file, name, tmp,  time, status,
			reason, generated, processed, shared_terms,
			raw_terms, rewrite_steps, r_matches, e_matches, literals)
{
   file = cwd "/__prvout__" procid "_" global_hostname "__";
   name = job;
   time = time_limit;
   status = "F";
   reason = "unknown";
   generated = 0;
   processed = 0;
   shared_terms = 0;
   raw_terms = 0;
   rewrite_steps = 0;
   r_matches = 0;
   e_matches = 0;
   literals = 0;

   while((getline tmp < file)>0)
   {
      if(index(tmp, "# SZS status: Satisfiable"))
      {
	 status = "N";
	 reason = "success";
      }
      else if(index(tmp, "# SZS status: CounterSatisfiable"))
      {
	 status = "N";
	 reason = "success";
      }
      else if(index(tmp, "# SZS status: Unsatisfiable"))
      {
	 status = "T";
	 reason = "success";
      }
      else if(index(tmp, "# SZS status: Theorem"))
      {
	 status = "T";
	 reason = "success";
      }
      else if(index(tmp, "# SZS status: GaveUp!"))
      {
	 status = "F";
	 reason = "maxtime";
      }
      else if(index(tmp, "# Total time"))
      {
	 time = substr(tmp, 30);
      }
   }
   close(file);

   printf("%-29s " status " %8.3f  %-10s\n",
	  name, 0+time, reason) >> logfile;
   printf("%-29s " status " %8.3f  " reason "\n", name, 0+time);
   system("rm " file);
}

function find_max_index(array,     i)
{
   for(i=1; array[i]; i++)
   {
   }
   return i-1;
}


function shift_num_array(array, position,              limit,i)
{
   limit = find_max_index(array);

   for(i = limit; i>=position; i--)
   {
      array[i+1] = array[i];
   }
   array[position] = "";

   return limit+1;
}



function job_is_not_processed(job    ,tmp)
{
   match(job, /^[0-9A-Za-z].*\.lop/) || \
   match(job, /^[0-9A-Za-z].*\.p/)||    \
   match(job, /^[0-9A-Za-z].*\.e/)  ||  \
   match(job, /^[0-9A-Za-z].*\.tpt/)  ||  \
   match(job, /^[0-9A-Za-z].*\.tptp/);
   tmp = substr(job, RSTART, RLENGTH);
   if(tmp in processed_jobs)
   {
      return 0;
   }
   return 1;
}

BEGIN{
   print "Initializing...";
   soft_cpu_limit = 0;
   time_limit = 10; # Default, may be overridden 
   auto_args = "-s --print-pid --resources-info --print-statistics";
   auto_args = "";
   first_job = 1;
   cwd = ENVIRON["PWD"];
   print "Working directory is " cwd "."; 
   procid = get_pid();
   global_hostname = get_hostname();

   print "PID is " procid " on host " global_hostname ".";
}


/^Executable:/{
   executable = expand_file($2);
   next;
}

/^Logfile:/{
   logfile = expand_file($2);
   next;
}

/^Problemdir:/{
   problemdir = expand_file($2);
   next;
}

/^Arguments:/{
   args = "";
   for(i=2; i<=NF; i++)
   {
      args = args " " $(i);
   }
   next;
}

/^Time limit:/{
   time_limit = $3;
   next;
}

/^Include:/{
   split(expand_file($2), local_files);
   local_count = ARGIND+1;
   for(i=1; local_files[i]; i++)
   {
      print "Adding file " local_files[i];
      shift_num_array(ARGV, local_count);
      ARGV[local_count] = local_files[i];
      local_count++;
      ARGC++;
   }
   next;
}


# Everything else not starting with # and not empty is a job!
/^#/{
  print "Skipping comment";
  next;
}

/[A-Za-z0-9]+/{
   job = $0;
   check_and_initialize();  
   if(job_is_not_processed(job))
   {
      cpu_opt = host_cpu_limit_opt(time_limit);
      outfile = cwd "/__prvout__" procid "_" global_hostname "__";
      command = executable " " auto_args " " args " " problemdir "/" job " > " outfile;
      
      prefix = "/usr/bin/nice -10 ";
      system(prefix command);
      process_result(job);
   }
}
      

END{
   print "Sorting Result file";
   srtfile = logfile ".srt";
   system("sort " logfile " | myuniq.awk > " srtfile "; mv " srtfile " " logfile);
   print "Test run complete";
}

