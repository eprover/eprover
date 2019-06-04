#!/opt/local/bin/gawk -f

# Usage: res_host.awk <time>
#
# Copyright 2001 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Reserve a dedicated host for the given user and for a certain time
# by managing a lock file that is checked by long-running
# applications. This is just the front end, the real work is done in
# handle_res.awk. 


function get_username(   pipe, tmp)
{
   pipe = "whoami";
   pipe | getline tmp;
   close(pipe);
   if(!tmp)
   {
      print "res_host.awk: Cannot get user name?!?" > "/dev/stderr";
      exit 1;
   }
   return tmp;
}

BEGIN{
   reserved_hosts["sunjessen56"] = "schulz";
   reserved_hosts["sunjessen48"] = "letz";
   reserved_hosts["sunjessen51"] = "stenzg";
   reserved_hosts["sunjessen57"] = "jobmann";

   if(ARGC!=2)
   {
      duration = 2;
   }
   else
   {
      duration = ARGV[1];
   }
   user     = get_username();
   ARGV[1] = "";
   
   host = "";
   for(i in reserved_hosts)
   {
      if(reserved_hosts[i]==user)
      {
	 host = i;
	 break;
      }
   }
   if(host=="")
   {
      print "res_host: User " user " does not have dedicated host." > "/dev/stderr";
      exit 1;
   }
   print "Will reserve host " host " for user " user " for " duration " hour(s)";
   system("handle_res.awk " user " " host " " duration "&");
}

