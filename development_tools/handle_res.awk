#!/opt/local/bin/gawk -f

# Usage: handle_res.awk <user> <machine> <time>
#
# Copyright 2001 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Generate a file home/<user>/HOST_RESERVED_<machine> and delete it
# again after <time> hours (unless it has been rewritten by some other
# process)
#

function get_hostname(   pipe, tmp)
{
   pipe = "hostname";
   pipe | getline tmp;
   close(pipe);
   if(!tmp)
   {
      print "handle_res.awk: Cannot get hostname?!?" > "/dev/stderr";
      exit 1;
   }
    return tmp;
}

function get_pid(     tmp)
{
   getline tmp < "/dev/pid";
   close("/dev/pid");
   if(!tmp)
   {
      print "handle_res.awk: Cannot get PID ?!?" > "/dev/stderr";
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

# round --- do normal rounding
#
# Arnold Robbins, arnold@gnu.org, August, 1996
# Public Domain

function round(x,   ival, aval, fraction)
{
   ival = int(x)    # integer part, int() truncates

   # see if fractional part
   if (ival == x)   # no fraction
      return x

   if (x < 0) {
      aval = -x     # absolute value
      ival = int(aval)
      fraction = aval - ival
      if (fraction >= .5)
         return int(x) - 1   # -2.5 --> -3
      else
         return int(x)       # -2.3 --> -2
   } else {
      fraction = x - ival
      if (fraction >= .5)
         return ival + 1
      else
         return ival
   }
}     

# End of Arnold Robbins PD code


BEGIN{
   home = "/home"
   global_procid = get_pid();
   global_hostname = get_hostname();

   if(ARGC!=4)
   {
      print "Usage: handle_res.awk <user> <machine> <time>" > "/dev/stderr";
      exit 1;
   }
   user     = ARGV[1];
   machine  = ARGV[2];
   duration = ARGV[3];
   sdur     = round(duration * 3600);
   ARGV[1] = "";
   ARGV[2] = "";
   ARGV[3] = "";

   res_file = home "/" user "/HOST_RESERVED_" machine;

   print "\nhandle_res: PID is " global_procid " on host " global_hostname ".";
   print "handle_res: Reserved host is " machine ".";
   print "handle_res: Reservation file is " res_file ".";
   print "handle_res: Reservation time is " duration " hours (" sdur " seconds).\n";

   fingerprint = global_hostname "-+-" global_procid;
   print fingerprint > res_file;
   close(res_file);

   system("sleep " sdur);

   test = getline tmp < res_file;
   close(res_file);
   # print "\n";
   if(test == -1)
   {
      # print "handle_res: Cannot find file anymore!" > "/dev/stderr";
      exit 1;
   }
   if(tmp == fingerprint)
   {
      # print "handle_res: Deleting my reservation file!" > "/dev/stderr";
      system("rm " res_file);
      exit 0;
   }
   if(match(tmp, /sunjessen.*-\+-[0-9]*/))
   {
      # print "handle_res: Reservation file seems to belong to someone else" > "/dev/stderr";
      exit 0;
   }      
   # print "handle_res: Deleting legacy or malformed reservation file" > "/dev/stderr";
   system("rm " res_file);
}

