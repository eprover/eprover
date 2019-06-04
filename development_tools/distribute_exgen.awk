#!/opt/local/bin/gawk -f

# Usage: distribute_exgen.awk <spec_file>
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a specification file describing a set of parameters and a list
# of problems, run the theorem prover on it and genrate a set of
# examples. Note that this is a lot cruder than the normal
# distribute_eprover.awk script.

function min(a,b)
{
  if(a>b)
    {
      return b;
    }
  return a;
}

function create_host_list(   i)
{
  all_hosts_word = "";
  host_number = 87;
  for(i=1; i<=host_number; i++)
  {
     all_hosts[i] = "sunhalle" i;
     all_hosts_word = all_hosts_word " " all_hosts[i];
     host_is_available["sunhalle" i] = 0;
     host_in_use["sunhalle" i] = 0;      
  }
  exclude_hosts["sunhalle34"] = 1; # Only 128 Megs! 
  exclude_hosts["sunhalle47"] = 1; 
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

   for(i=1; i<= host_number; i++)
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


function collect_host_info(    res, sum, count, i)
{
  count = 0;
  sum = 0;

  global_hostinfoage = 0;
  
  for(i in all_hosts)
  {
     if(exclude_hosts[all_hosts[i]]!=1)
     {
	print "Checking host " all_hosts[i] ".";
	res = get_load(all_hosts[i]);
	if(res != 1000000)
	{	  
	   sum+=res;
	   count++;
	   host_is_available[all_hosts[i]] = 1;
	}
     }
  }
  print "Found " count " hosts, average load is " sum/count ".";
  return sum/count;
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


function file_exists(file,    test)
{
  test = getline < file;      
  close(file);
  if(test == -1)
    {
      return 0;
    }
  return 1;
}

function update_global_host_count()
{
   global_host_count++;
   if(global_host_count > host_number)
   {
      global_host_count = 1;
   }
}


function get_load(host,       pipe, tmp)
{
  tmp = "";
  pipe = "ssh " host " uptime";
  pipe | getline tmp;
  close(pipe);  
  if(!index(tmp, "load average"))
    {
      /* No good response -> dont use machine */
      return 1000000;
    }  
  if(exclude_hosts[host]==1)
  {
     /* Machine unsuitable for reasons known to the user */
      return 1000000;
  }
  return extract_load(tmp, 2);
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
      }
      else
      {
	 print "Logfile " logfile " exists, using old results";
	 
	 processed_count = 0;
	 while ((getline tmp < logfile) > 0)
	 {
	    if(match(tmp, /^[A-Z]+[0-9]+.*\.lop/) || 
	       match(tmp, /^[A-Z]+[0-9]+.*\.p/) ||
	       match(tmp, /^[A-Z]+[0-9]+.*\.tptp/))
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



function process_result(host     , file, tmp, name)
{
   file = host "_complete";
   getline name < file;
   close(file);
   
   if(index(name, ".lop") || index(name, ".p") || index(name, ".tptp"))
   {
      printf("%-29s done\n", name) >> logfile;
      open_jobs--;	    
      printf("Open: " open_jobs " %-29s done\n", name);
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
   system("rm " cwd "/" host "_*");
   host_in_use[host] = 0;
}



function get_host(   i, host, load, tmp_count)
{
   host = "";
   
   global_hostinfoage++;
   if(global_hostinfoage >= 400)
   {
      collect_host_info();
   }
   
   while(!host)
   {
      for(i = 1; i<= host_number; i++)
      {
	 update_global_host_count();
	 if(host_is_available[all_hosts[global_host_count]])
	 {
	    if((host_in_use[all_hosts[global_host_count]]==0) &&
	       (!file_exists(all_hosts[global_host_count] "_lock")))
	    {
	       load = get_load(all_hosts[global_host_count]);
	       
	       if((load <= no_real_load) || 
		  (load <= min(acceptable_load, avg_load+0.5)))
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
   match(job, /^[A-Z]+[0-9]+.*\.lop/) || match(job, /^[A-Z]+[0-9]+.*\.p/) || match(job, /^[A-Z]+[0-9]+.*\.tptp/);
   tmp = substr(job, RSTART, RLENGTH);
   if(tmp in processed_jobs)
   {
      return 0;
   }
   return 1;
}



function kill_job(lockfile,     host, pid)
{
   if(match(lockfile, /sunhalle[0-9]+/))
   {
      host = substr(lockfile, RSTART, RLENGTH);   
      system("ssh " host " kill `top | grep eprover | cut -d' ' -f1`");
      system("rm " cwd "/" host "_*");
      return 1;
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
  }	  
  close(pipe);
  system("rm " cwd "/"  "sunhalle*_complete");
  
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



BEGIN{
  print "Initializing...";
  no_real_load = 1;
  acceptable_load = 4;
  time_limit = 10; /* Default, may be overridden */
  auto_args = " --print-pid --resources-info --memory-limit=192";
  first_job = 1;
  cwd = ENVIRON["PWD"];
  print "Working directory is " cwd
  print "Killing old jobs";  
  kill_old_jobs();
  print "Creating host list";
  create_host_list();
  print "Getting host information";
  avg_load = collect_host_info();
  global_host_count = 0;
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
      outfile3 = "/tmp/ep_" job;
      outfile4 = "/tmp/" job "_ex";
      outfile5 = outfile4 ".gz";
      command = executable " -l4 " auto_args " --soft-cpu-limit=" time_limit " " args " "\
                problemdir "/" job " > " outfile3 ";\
                /usr/wiss/schulz/home_sun/bin/direct_examples -e -n0 -p1 " outfile3 " -o " outfile4 ";\
                rm " outfile3 ";\
                gzip " outfile4 ";\
                mv " outfile5 " /usr/wiss/schulz/home_sun/EXAMPLE_FILES/";
      prefix = "/bin/nice -10 ";
      printf "Distributing " job " onto " host_local ". ";
      if(system("ssh -n " host_local " \"" "(touch " outfile1  "; (" prefix command "; sync; sleep 3; echo " job ">" outfile2 ")&)>&/dev/null\""))
      {
	 print "Warning: " host_local " returned error";
      }
      else
      {
	 open_jobs++;	       
	 print "Open jobs: " open_jobs;
	 host_in_use[host_local] = systime();
      }
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

