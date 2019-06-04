#!/opt/local/bin/gawk -f

# Usage: sequential_eprover.awk <spec_file>
#
# Copyright 1998-2006 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a specification file describing a set of parameters and a list
# of problems, and run a sequential test of the prover over the
# problems. Runs only on known architecture and normalizes times using
# the E-Mark Benchmark.
#

# ----------% Here starteth the generic stuff % ------------- #

# Similar to C assert()

function assert(val, comment)
{
   if(!val)
   {
      print "Assertion fail: " comment > "/dev/stderr";
      exit 1;
   }   
}



# Return the result of a single, simple shell command yieding exactly
# one line

function get_shell_res(cmd,   tmp)
{
   cmd | getline tmp;
   close(cmd);  

   assert(tmp, "No result found (get_shell_res)");
   return tmp;
}


# Minimum...

function min(a,b)
{
  if(a>b)
    {
      return b;
    }
  return a;
}


# Get the host name

function get_hostname(     tmp)
{
   tmp = get_shell_res("hostname");

   return tmp;
}

# Get the Pid

function get_pid(     tmp)
{
   getline tmp < "/dev/pid";
   close("/dev/pid");
   if(!tmp)
   {
      print ARGV[0] ": Cannot get PID ?!?" > "/dev/stderr";
      exit 1;
   }
   return tmp;
}


# Perform globbing

function expand_file(name,            tmp)
{
   tmp = get_shell_res("csh -c \"echo " name "\"");
   if(!tmp)
   {
      print ARGV[0] ": Cannot expand filename" > "/dev/stderr";
      exit 1;
   }
   print name " expands into " tmp;
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


# ----------% Here starteth the local stuff % ------------- #

function get_cluster_name(   pipe, tmp)
{
   tmp = get_hostname();

   if(match(tmp, /sunjessen.*/))
   {
      return "sunjessen";
   }
   else if(match(tmp, /sunhalle.*/))
   {
      return "sunhalle";
   }
   else if(match(tmp, /rayhalle.*/))
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
   else if(match(tmp, /leonardo/))
   {
      return "leonardo";
   }   
   return 0;
}



function check_and_initialize(    tmp, job)
{
   if(first_job)
   {
      if(!executable)
      {
	 print ARGV[0] ": No executable found" > "/dev/stderr";
	 exit 1;
      }
      if(!logfile)
      {
	 print ARGV[0] ": No logfile found" > "/dev/stderr";
	 exit 1;
      }
      if(!problemdir)
      {
	 print ARGV[0] ": No problemdir found" > "/dev/stderr";
	 exit 1;
      }
      if(!file_exists(logfile))
      {
	 print "Creating log file " logfile;
	 print "# " args " " auto_args > logfile;
	 execpath = get_shell_res("which " executable);
	 execdate = get_shell_res("ls -l " execpath);
	 exechost = get_hostname();
	 execversion = get_shell_res(executable " -V");
	 print "# Started with " substr(execdate, 42) " (" execversion ") on " exechost >> logfile;
      }
      else
      {
	 print "Logfile " logfile " exists, using old results";
	 execpath = get_shell_res("which " executable);
	 execdate = get_shell_res("ls -l " execpath);
	 exechost = get_hostname();
	 execversion = get_shell_res(executable " -V");
	 print "# Started with " substr(execdate, 42) " (" execversion ") on " exechost >> logfile;
	 processed_count = 0;
	  while ((getline tmp < logfile) > 0)
	  {
	     if(match(tmp, /^[0-9A-Za-z].*\.lop/)||
		match(tmp, /^[0-9A-Za-z].*\.p/)||
		match(tmp, /^[0-9A-Za-z].*\.e/)||
		match(tmp, /^[0-9A-Za-z].*\.tpt/)||
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

function host_cpu_limit_opt(limit, e_mark,    res, host_limit)
{
   host_limit = ((limit*100)/e_mark)+0.5;
   if(host_limit<1)
   {
      host_limit=1;
   }
   
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
                        raw_terms, rewrite_steps, r_matches, 
                        e_matches, literals, nu_subs, nurec_subs, u_subs)
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
      else if(index(tmp, "# Clause-clause subsumption calls (NU) :"))
      {
         nu_subs = substr(tmp, 42);
      }	 
      else if(index(tmp, "# Rec. Clause-clause subsumption calls :"))
      {
         nurec_subs = substr(tmp, 42);
      }	 
      else if(index(tmp, "# Unit Clause-clause subsumption calls :"))
      {
         u_subs = substr(tmp, 42);
      }	 
      else if(index(tmp, "# Total time"))
      {
         time = substr(tmp, 30)*e_mark_host/100;
      }
   }
   close(file);

   printf("%-29s " status " %8.3f  %-10s %10d %10d %10d %10d %10d\n", \
          name, 0+time, reason, generated, processed, \
          nu_subs, nurec_subs,u_subs) >> logfile;   
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

function host_is_reserved(         pipe, tmp)
{
   if(global_checkreservation)
   {
      return file_exists(global_checkreservation);
   }
   return 0;
}

BEGIN{
   cluster = get_cluster_name();
   if(cluster == "sunjessen")
   {
      mem_limit=128;
   }
   else if(cluster == "sunhalle")
   {
      mem_limit=192;
   }
   else if(cluster == "leonardo")
   {
      mem_limit=192;
   }
   else if(cluster == "athalle")
   {
      mem_limit=96;
   }
   else 
   {
      mem_limit=192;
   }
   print "Cluster " cluster ", memory limited to " mem_limit " MB";

   print "Initializing...";
   time_limit = 10; # Default, may be overridden 
   soft_cpu_limit = 0;
   auto_args = "-s --print-statistics --print-pid --resources-info --memory-limit=" mem_limit;
   first_job = 1;
   cwd = ENVIRON["PWD"];
   print "Working directory is " cwd "."; 
   procid = get_pid();
   global_hostname = get_hostname();

   e_mark["Pentium-III-Coppermine-696.422"]              = 225.642;
   e_mark["Pentium-III-Coppermine-728.454"]              = 196.681;
   e_mark["Pentium-III-Coppermine-860.904"]              = 260.519;
   e_mark["Pentium-III-Coppermine-864.484"]              = 222.518;
   e_mark["Pentium-III-Katmai-498.762"]                  = 143.921;
   e_mark["REFERENCE"]                                   = 100;
   e_mark["SUNW-Sun-Blade-100-sparcv9-502"]              = 131.934;
   e_mark["SUNW-Sun-Blade-1000-sparcv9-750"]             = 250.049;
   e_mark["SUNW-Ultra-2-sparc-200"]                      = 69.2536;
   e_mark["SUNW-Ultra-5_10-sparcv9-300"]                 = 87.2878;
   e_mark["SUNW-Ultra-5_10-sparcv9-440"]                 = 140.591;
   e_mark["SUNW-Sun-Fire-880-sparcv9-900"]               = 285.646;
   e_mark["SUNW-Sun-Fire-880-sparcv9-1200"]              = 380.861;
   e_mark["SUNW-Sun-Fire-sparcv9-750"]                   = 211.921;
   e_mark["ppc7450-1000"]                                = 267.552;
   e_mark["ppc7450-667"]                                 = 267.552; #hack
   e_mark["IntelR-PentiumR-4-CPU-1400MHz-1396"]          = 300.922;
   e_mark["SUNW-Ultra-60-sparcv9-296"]                   = 100;
   e_mark["NC-AMD-Athlontm-MP-Processor-1600+-1400"]     = 366.393;
   e_mark["Pentium-II-Deschutes-448.882"]                = 157.919;
   e_mark["IntelR-PentiumR-4-CPU-1.80GHz-1794"]          = 345.135;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2405"]          = 496.488;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2405"]          = 496.488;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2406"]          = 518.546;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2406"]          = 521.253;
   e_mark["i486-2800"]                                   = 1160.92;

   e_mark_host = e_mark[get_shell_res("get_system.awk")];
   if(!e_mark_host)
   {
      print ARGV[0] ": Unknown system - need to know E-Mark!" > "/dev/stderr";
      exit 1; 
   }
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
      cpu_opt = host_cpu_limit_opt(time_limit, e_mark_host);
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

