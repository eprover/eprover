#!/opt/local/bin/gawk -f

# Usage: sequential_eprover.awk <spec_file>
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a specification file describing a set of parameters and a list
# of problems, and run a sequential test of the prover over the
# problems. 


function min(a,b)
{
  if(a>b)
    {
      return b;
    }
  return a;
}

function get_cluster_name(   pipe, tmp)
{
   pipe = "hostname";
   pipe | getline tmp;
   close(pipe);
   if(match(tmp, /sunjessen.*/))
   {
      return "sunjessen";
   }
   else if(match(tmp, /sunhalle.*/))
   {
      return "sunhalle";
   }
   else if(match(tmp, /athalle.*/))
   {
      return "athalle";
   }
   else if(match(tmp, /lxjessen.*/))
   {
      return "lxjessen";
   }   
   return 0;
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
      print "sequential_eprover.awk: Cannot get PID ?!?" > "/dev/stderr";
      exit 1;
   }
   return tmp;
}

function get_MHz_local()
{
  tmp = "";
  res = "";
  pipe = "psrinfo -v";
  while((pipe | getline tmp )>0)
  {
     if(i = index(tmp, mhz_string))
     {
        # print tmp, i
        res = int(substr(tmp, i+mhz_string_l))
        break;
     }
  }
  close(pipe);

  return res;
}


function expand_file(name,            pipe, tmp)
{
   pipe = "echo " name;
   pipe | getline tmp;
   close(pipe);
   if(!tmp)
   {
      print "sequential_eprover.awk: Cannot expand filename" > "/dev/stderr";
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
	  print "sequential_eprover.awk: No executable found" > "/dev/stderr";
	  exit 1;
	}
      if(!logfile)
	{
	  print "sequential_eprover.awk: No logfile found" > "/dev/stderr";
	  exit 1;
	}
      if(!problemdir)
	{
	  print "sequential_eprover.awk: No problemdir found" > "/dev/stderr";
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
	     if(match(tmp, /^[A-Za-z].*\.lop/)||
		match(tmp, /^[A-Za-z].*\.p/)||
		match(tmp, /^[A-Za-z].*\.e/)||
		match(tmp, /^[A-Za-z].*\.tptp/))
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

function host_cpu_limit_opt(limit, host_mhz,    res, host_limit)
{
   host_limit = ((limit*norm_mhz)/host_mhz)+0.5;
   
   if(soft_cpu_limit)
   {
      res = sprintf("--soft-cpu-limit=%d --cpu-limit=%d",
                    host_limit,3*host_limit);
   }
   else
   {
      res = sprintf("--cpu-limit=%d",host_limit);
   }
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
      if(index(tmp, "# No proof found!"))
      {
	 status = "N";
	 reason = "success";
      }
      else if(index(tmp, "# Proof found!"))
      {
	 status = "T";
	 reason = "success";
      }
      else if(index(tmp, "# Failure: Out of unprocessed clauses!"))
      {
	 status = "F";
	 reason = "incomplete";
      }
      else if(index(tmp, "# Failure: Resource limit exceeded (memory)"))
      {
	 reason = "maxmem ";
      }
      else if(index(tmp, "# Failure: Resource limit exceeded (time)"))
      {
	 reason = "maxtime ";
      }	 
      else if(index(tmp, "# Failure: User resource limit exceeded"))
      {
	 reason = "maxres";
      }       
      else if(index(tmp, "# Processed clauses                    :"))
      {
	 processed = substr(tmp, 42);
      }       
      else if(index(tmp, "# Generated clauses                    :"))
      {
	 generated = substr(tmp, 42);
      }       
      else if(index(tmp, "# Shared term nodes                    :"))
      {
	 shared_terms = substr(tmp, 42);
      }       
      else if(index(tmp, "# ...corresponding unshared nodes      :"))
      {
	 raw_terms = substr(tmp, 42);
      }       
      else if(index(tmp, "# Shared rewrite steps                 :"))
      {
	 rewrite_steps = substr(tmp, 42);
         }       
      else if(index(tmp, "# Match attempts with oriented units   :"))
      {
	 r_matches = substr(tmp, 42);
      }       
      else if(index(tmp, "# Match attempts with unoriented units :"))
      {
	 e_matches = substr(tmp, 42);
      }       
      else if(index(tmp, "# Total literals in generated clauses  :"))
      {
	 literals = substr(tmp, 42);
      }       
      else if(index(tmp, "# Total time"))
      {
	 time = substr(tmp, 30)*local_mhz/norm_mhz;
      }
   }
   close(file);

   printf("%-29s " status " %8.3f  %-10s %10d %10d %10d %10d %10d %10d %10d %10d\n",
	  name, 0+time, reason, generated, processed, shared_terms,
	  raw_terms, rewrite_steps, r_matches, e_matches, literals) >> logfile;
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
   match(job, /^[A-Za-z]+.*\.lop/) || match(job, /^[A-Za-z]+.*\.p/)|| match(job, /^[A-Za-z]+.*\.e/)  || match(job, /^[A-Za-z].*\.tptp/);
   tmp = substr(job, RSTART, RLENGTH);
   if(tmp in processed_jobs)
   {
      return 0;
   }
   return 1;
}

function host_is_reserved(         pipe, tmp)
{
   if(global_checkreservation)
   {
      return file_exists(global_checkreservation);
   }
   return 0;
}

BEGIN{
   print "Initializing...";
   mhz_string = "processor operates at ";
   mhz_string_l = length(mhz_string);
   norm_mhz = 296;
   soft_cpu_limit = 0;
   time_limit = 10; # Default, may be overridden 
   cluster = get_cluster_name();
   if(cluster == "sunjessen")
   {
      mem_limit=128;
   }
   else if(cluster == "sunhalle")
   {
      mem_limit=192;
   }
   else if(cluster == "athalle")
   {
      mem_limit=96;
   }
   else 
   {
      mem_limit=48;
   }
   local_mhz = get_MHz_local();
   print "Cluster " cluster ", memory limited to " mem_limit " MB, " \
      local_mhz " MHz processor";
   
   time_limit = 10; # Default, may be overridden 
   auto_args = "-s --print-pid --resources-info --print-statistics --memory-limit=" mem_limit;
   first_job = 1;
   cwd = ENVIRON["PWD"];
   print "Working directory is " cwd "."; 
   procid = get_pid();
   global_hostname = get_hostname();

   reserved_hosts["sunjessen56"] = "schulz";
   reserved_hosts["sunjessen48"] = "letz";
   reserved_hosts["sunjessen51"] = "stenzg";
   reserved_hosts["sunjessen57"] = "jobmann";
   if(global_hostname in reserved_hosts)
   {
      global_checkreservation = "/home/" \
	 reserved_hosts[global_hostname] \
	 "/HOST_RESERVED_" global_hostname;
   }
   
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
   if(index(args,"--print-detailed-statistics"))
   {
      print "Switching to soft cpu limit for detailed statistics";
      soft_cpu_limit = 1;
   };
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
   while(host_is_reserved())
   {
      print "Waiting for reservation to expire!";
      system("sleep 600")
   }
   job = $0;
   check_and_initialize();  
   if(job_is_not_processed(job))
   {
      cpu_opt = host_cpu_limit_opt(time_limit, local_mhz);
      outfile = cwd "/__prvout__" procid "_" global_hostname "__";
      command = executable " " auto_args " " cpu_opt " " args " " problemdir "/" job " > " outfile;
      
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

