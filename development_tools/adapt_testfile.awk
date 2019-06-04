#!/opt/local/bin/gawk -f

# Usage: adapt_testfile.awk <file>
#
# Copyright 1998-2002 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Take a test run specification file (a "tptp_*" file) and set the
# correct pathes for the current test setup.
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


# Get the host name

function get_hostname(     tmp)
{
   tmp = get_shell_res("hostname");

   return tmp;
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

BEGIN{
   sunhallepath = "/usr/wiss/schulz/home_sun/EPROVER";
   athallepath  = "/home/nfshalle/wiss/schulz/home_at/home_sun/EPROVER";
   otherpath    = "/home/schulz/EPROVER";
   cl = get_cluster_name();
   
   if(cl=="sunhalle")
   {
      path = sunhallepath;
      db_limit="150000000";
   }
   else if(cl=="athalle")
   {
      path = athallepath;
      db_limit="100000000";
   }
   else
   {
      path = otherpath;
      db_limit="100000000";
   }   
}

{
   sub(sunhallepath, path);
   sub(athallepath, path);
   sub(otherpath, path);
   sub(/delete-bad-limit=[^ ]*/,"delete-bad-limit=" db_limit);
   print $0;
}


