#!/opt/local/bin/gawk -f

# Usage: distribute_eprover.awk <spec_file> [<lower> [<upper>]]
#
# Copyright 2001 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a specification file describing a set of parameters and a list
# of problems, and run a distributed test of the prover over the
# problems. This is fairly specific for our site (with systems named
# systematically), but should be rewritable to a more general case.


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

   assert(tmp, "No result found (get_shell_res(" cmd ")");
   return tmp;
}


# Same thing, but no response is not a bug automatically

function get_shell_res_nocheck(cmd,   tmp)
{
   tmp = "";
   cmd | getline tmp;
   close(cmd);  

   return tmp;
}

# Return the result of a single, simple shell command yieding exactly
# one line, and executed on a remote host. The command should not
# contain single quotes!

function get_remote_shell_res(host, cmd,    tmp)
{
   tmp = get_shell_res("ssh -x " host " '" cmd "'");

   return tmp;
}

# Same thing, but empty response is ok.

function get_remote_shell_res_nocheck(host, cmd,   tmp)
{
   tmp = get_shell_res_nocheck("ssh -x " host " '" cmd "'");

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

function exp_file_exists(file,    test, tmp)
{
   return file_exists(expand_file(file));
}



function has_interactive_user(host,    user)
{
   user = get_remote_shell_res_nocheck(host, "who|grep console");
   return user;
}


# Compute memory in Megabytes from a string containing Bytes (no
# suffixe), Kilobytes (Suffix K), Megabytes (Suffix M) or Gigabytes
# (Suffix G)

function normalize_mem(valstring)
{
   if(valstring ~ /[0-9]+G/)
   {
      return int(valstring*1024);
   }
   if(valstring ~ /[0-9]+M/)
   {
      return int(valstring);
   }
   if(valstring ~ /[0-9]+K/)
   {
      return int(valstring/1024);
   }
   return int(valstring/(1024*1024))
}


# Get the physical memory and free memory of a machine

function get_mem_info(host, result_array,                tmp, tmparr)
{
   tmp = get_remote_shell_res_nocheck(host, "top -d1 | grep ^Memory");
   if(!tmp)
   {
      return 0;
   }
   split(tmp, tmparr);
   result_array[0] = normalize_mem(tmparr[2]);
   result_array[1] = normalize_mem(tmparr[4]);   
   return 1;
}




# ----------% Here starteth the local stuff % ------------- #


# If signal_file exists, sleep in 10 minute intervals until current
# hour is not within [start_block, end_block] 

function delay_processing(start_block, end_block, signal_file,    tmp, day)
{
   delay = exp_file_exists(signal_file);
   tmp = strftime("%H", systime())+0;
   day = strftime("%w", systime())+0;
   while(delay && (day!=0) && (day!=6) && (tmp >= start_block) && (tmp < end_block))
   {
      print "Blockfile exists, sleeping"
      system("sleep 600");
      delay = exp_file_exists(signal_file);
      tmp = strftime("%H", systime())+0;
      day = strftime("%w", systime())+0;
   }
}

function init_machine_ratings()
{
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
   e_mark["ppc7450-1000"]                                = 267.552;
   e_mark["IntelR-PentiumR-4-CPU-1400MHz-1395.787"]      = 300.922;
   e_mark["SUNW-Ultra-60-sparcv9-296"]                   = 100;
   e_mark["NC-AMD-Athlontm-MP-Processor-1600+-1400.058"] = 366.393;
   e_mark["Pentium-II-Deschutes-448.882"]                = 157.919;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2405.486"]      = 496.488;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2405.487"]      = 496.488;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2405.526"]      = 518.546;
   e_mark["IntelR-PentiumR-4-CPU-2.40GHz-2405.502"]      = 521.253;
}


function get_host_type(host,          res)
{
   res = get_shell_res("ssh -x " host " ~/bin/get_system.awk");
   if(!e_mark[res])
   {
      return "";
   }
   print "Host " host " is type " res " with " e_mark[res] " EMark";
   return res;
}

function create_host_list(   i)
{
  for(i=lower_host; i<=upper_host; i++)
  {
     all_hosts[i] = "sunhalle" i;
     host_is_available["sunhalle" i] = 0;
     host_in_use["sunhalle" i] = 0;      
  }
  exclude_hosts["sunhalle83"] = 1; # ssh trouble 
}


function get_cluster_name(   pipe, tmp)
{
   tmp = get_shell_res("hostname");

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
   else if(match(tmp, /lxjessen.*/))
   {
      return "lxjessen";
   }   
   return 0;
}


function print_used_hosts(print_diffs     ,i, count_files,
			  count_array, files, array)
{
   count_files = 0;
   count_array = 0;

   for(i=lower_host; i<= upper_host; i++)
   {
      array = host_in_use[all_hosts[i]];
      files = file_exists(all_hosts[i] "_lock");

      if(array)
      {
	 /* print "In use: " all_hosts[i]; */
	 count_array++;
      }
      if(files)
      {
	 count_files++;
      }
      if(print_diffs)
      {
	 if((array && (!files)))
	 {
	    print all_hosts[i] " in host_in_use[], but no lock file";
	 }
	 if(((!array) && files))
	 {
	    print all_hosts[i] " has lock file, but no host_in_use[] entry";
	 }
      }
   }   
   print "Found " count_array " array entries";
   print "Found " count_files " lock files";
}


function extract_load(string, which,     tmp_array, number)
{
  number = split(string, tmp_array, ",");
  if(number == 0)
    {
      return -1;
    }
  return tmp_array[number-3+which];
}


function extract_host(string)
{
  match(string, /sunhalle[0-9]+/);
  return substr(string, RSTART, RLENGTH);
}


function collect_host_info(    res, count, i, type)
{
  count = 0;
  sum = 0;

  global_check_blockfile_age = 0;
  
  for(i in all_hosts)
  {
     if(exclude_hosts[all_hosts[i]]!=1)
     {
	print "Checking host " all_hosts[i] ".";
	res = check_availablity(all_hosts[i]);
	if(res != 1000000)
	{
           type = get_host_type(all_hosts[i]);
           if(type)
           {
              count++;            
              host_proc_power[all_hosts[i]] = e_mark[type];
              print "..." all_hosts[i] " is ready at " host_proc_power[all_hosts[i]] " EMark";
           }
	}
     }
  }
  print "Found " count " hosts.";
  return count;
}


function update_global_host_count()
{
   global_host_count++;
   if(global_host_count > upper_host)
   {
      global_host_count = lower_host;
   }
}


function ping_host(host,   pipe, tmp)
{
  tmp = get_shell_res_nocheck("ping " host);

  if(!index(tmp, "alive"))
  {     
     return 0;
  } 
  return 1;
}


function check_availablity(host,       pipe, tmp, cmd, memdata)
{
  if(exclude_hosts[host]==1)
  {
     /* Machine unsuitable for reasons known to the user */
      return 1000000;
  }
  if(!ping_host(host))
  {
     # No good response -> dont use machine 
	 return 1000000;
  } 

  mem_data[0] = 0;
  mem_data[1] = 0;
  if(!get_mem_info(host, mem_data))
  {
     # Something is broken
     return 1000000;
  }
  if(mem_data[0] < 256)
  {
     print "# Checking " host ": Not enough memory " mem_data[0];
     exclude_hosts[host] = 1;
     return 1000000; # Not enough memory
  }
  if(has_interactive_user(host))     
  {
     if(mem_data[1] < 160)
     {
	print "# Checking " host ": Greedy interactive user, only " mem_data[1] " free";
	return 1000000;
     }     
  }
  #{
  #   /* No good response -> dont use machine */
  #  return 1000000;
  #}  
  #return extract_load(tmp, 2);
  return 0;
}

  
function check_and_initialize(    tmp, job)
{
   if(first_job)
   {
      if(!executable)
      {
	 print "distribute_eprover.awk: No executable found" > "/dev/stderr";
	 exit 1;
      }
      if(!logfile)
      {
	 print "distribute_eprover.awk: No logfile found" > "/dev/stderr";
	 exit 1;
      }
      if(!problemdir)
      {
	 print "distribute_eprover.awk: No problemdir found" > "/dev/stderr";
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
	    if(match(job, /^[0-9A-Za-z].*\.lop/) ||
               match(job, /^[0-9A-Za-z].*\.p/)||   
               match(job, /^[0-9A-Za-z].*\.e/)  || 
               match(job, /^[0-9A-Za-z].*\.tpt/)  ||
               match(job, /^[0-9A-Za-z].*\.tptp/))
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



function process_result(host     , file, tmp, name, time, org_time, status,\
	     reason, generated, processed,shared_terms,raw_terms,\
	     rewrite_steps, r_matches, e_matches, literals,nu_subs,\
	     nurec_sub, u_subs)
{
   file = host "_complete";
   getline name < file;
   close(file);
   
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

   if(index(name, ".lop") || index(name, ".p") || index(name, ".tptp"))
   {
      file = host "_lock";
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
	    org_time = substr(tmp, 30);
	    time = org_time*host_proc_power[host]/100;
	    # printf("Time %8.3f on host %s corrected to %8.3f\n", org_time, host, time);
	 }
      }
      close(file);
      printf("%-29s " status " %8.3f  %-10s %10d %10d %10d %10d %10d\n", \
	     name, 0+time, reason, generated, processed, \
	     nu_subs, nurec_subs,u_subs) >> logfile;
      open_jobs--;	    
      printf("Open: " open_jobs " %-29s " status " %8.3f  " reason "\n", name, 0+time);
   }
   else
   {
      print "No valid job in " host"_complete: Something strange";
      if(!file_exists(file))
      {
	 print host"_complete does not seem to exists?!?!";
      }
      else
      {
	 system("echo " file " >> buggy_complete");
	 system("cat "  file " >> buggy_complete");
      }
   }
   
   system("rm " cwd "/" host "_lock");
   system("rm " cwd "/" host "_complete");

   host_in_use[host] = 0;
}





function get_host(   i, host, load, tmp_count)
{
   host = "";
   
   global_check_blockfile_age++;
   if(global_check_blockfile_age >= 400)
   {
      delay_processing(8, 18, "~/block_eprover");
      global_check_blockfile_age = 0;
   }
   
   while(!host)
   {
      for(i = lower_host; i<= upper_host; i++)
      {
	 update_global_host_count();
	 if(host_proc_power[all_hosts[global_host_count]])
	 {
	    if((host_in_use[all_hosts[global_host_count]]==0) &&
	       (!file_exists(all_hosts[global_host_count] "_lock")))
	    {
	       load = check_availablity(all_hosts[global_host_count]);
	       
	       if(load != 1000000)
	       {		  
		  host = all_hosts[global_host_count];
		  break;
	       }
	    } 
	 }
      }
      if(!host)
      {
	 print "No host, processing results";
	 if(!process_pending_results())
	 {
	    print "No host available, sleeping";
	    system("sleep 30");
	 }
      }
   }  
   return host;
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


function find_pid_in_protocoll(file,      pid, tmp)
{  
  while ((getline tmp < file) > 0)
    {
      if(match(tmp, /# Pid: [0-9]+/))
	{
	  pid  = substr(tmp, 8);
	  break;
	}
    }	  
  close(file);
  return pid;
}


function kill_job(lockfile,     host, pid, res, shell)
{
   if(match(lockfile, /sunhalle[0-9]+/))
   {
      host = substr(lockfile, RSTART, RLENGTH);   
      if(ping_host(host))
      {
	 pid  = find_pid_in_protocoll(lockfile);
	 system("ssh -x " host " kill " pid "\n");
	 res = 1;
      }
      else
      {
	 print "Host " host " not reachable";
	 res = 0;
      }      
      system("rm " cwd "/" host "_lock");
      system("rm " cwd "/" host "_complete");
      return res;
   }
   return 0;
}


function kill_old_jobs(         pipe, count, tmp)
{
  pipe = "ls " cwd "/sunhalle*_lock";
  count = 0;

  while ((pipe | getline tmp) > 0)
  {
     count+=kill_job(tmp);
     host_in_use[tmp] = 0;
  }	  
  close(pipe);
  
  system("rm " cwd "/sunhalle*_complete");
  system("rm " cwd "/sunhalle*_lock");

  open_jobs = 0;
  print "Killed " count " old job(s)";
}


function check_alive(     current_time, i)
{
   current_time = systime();
   
   for(i in host_in_use)
   {
      if(host_in_use[i] && ((current_time - host_in_use[i]) > time_limit*10))
      {
	 print "Trying to kill dormant job on " i ".";
	 kill_job(i "_lock");
	 open_jobs--;
	 host_in_use[i] = 0;
      }
   }   
}


function number_of_lock_files(        pipe, count)
{
  count = 0;

  pipe = "ls " cwd "/sunhalle*_lock";
  
  while ((pipe | getline tmp) > 0)
    {
      if(match(tmp, /sunhalle[0-9]+_lock/))
	{
	  count++;
	}
    }	  
  close(pipe);
  
  return count;
}


function process_pending_results(     pipe, count, tmp)
{
   print_used_hosts(1);
   count = 0;
   
   pipe = "ls " cwd "/sunhalle*_complete";
   
   while ((pipe | getline tmp) > 0)
   {
      if(match(tmp, /sunhalle[0-9]+/))
      {
	 host = substr(tmp, RSTART, RLENGTH);
	 process_result(host);
	 count++; 
      }
   }	  
   close(pipe);
   print_used_hosts(1);
   return count;
}

function host_cpu_limit_opt(limit, host_rating,    res, host_limit)
{
   host_limit = ((limit*100)/host_rating)+0.5;

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


BEGIN{
  print "Initializing...";
  init_machine_ratings();
  soft_cpu_limit = 0;
  time_limit = 10; /* Default, may be overridden */
  auto_args = "-s --print-pid --resources-info --print-statistics --memory-limit=192";
  first_job = 1;
  cwd = ENVIRON["PWD"];
  print "Working directory is " cwd
  print "Killing old jobs";  
  kill_old_jobs();

  lower_host = 1;
  upper_host = 107;
  if(ARGV[2])
  {
     lower_host = ARGV[2];
     ARGV[2] = "";
  }
  if(ARGV[3])
  {
     upper_host = ARGV[3];
     ARGV[3] = "";
  }
  print "Creating host list";
  create_host_list();
  print "Getting host information";
  collect_host_info();
  global_host_count = lower_host;
  print "...complete";
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
   job = $0;
   check_and_initialize();  
   if(job_is_not_processed(job))
   {
      host_local = get_host();
      outfile1 = cwd "/" host_local "_lock";
      outfile2 = cwd "/" host_local "_complete";
      
      cpu_opt = host_cpu_limit_opt(time_limit, host_proc_power[host_local]);
      
      command = executable " " auto_args " " cpu_opt " " args " " problemdir "/" job " > " outfile1;

      printf "Distributing " job " onto " host_local ". ";
      remote_shell =  "ssh -x -T " host_local " 2>&1 1> /dev/null";
      print  "touch " outfile1 |remote_shell;
      print "(/bin/nice -15 " command "; sync; sleep 3; echo " job ">" \
	 outfile2 ")< /dev/null > & /dev/null &\n" | remote_shell;
      close(remote_shell);
      open_jobs++;	       
      print "Open jobs: " open_jobs;
      host_in_use[host_local] = systime();
   }   
   else
   {
      print "Job " job " already processed";
   }
}
      

END{
  print "Distribution of jobs complete";

  count = 0;
  while((lock_files_pending = number_of_lock_files()) > 0)
    {
      print "Waiting for " lock_files_pending " results... (Open jobs:) " open_jobs;
      system("sleep 30");
      count++;
      if(count%10 == 0)
      {
	 check_alive();
      }
      else
      {
	 process_pending_results();
      }
    }
  print "Sorting Result file";
  srtfile = logfile ".srt";
  system("sort " logfile " | myuniq.awk > " srtfile "; mv " srtfile " " logfile);
  print "Test run complete";
}

