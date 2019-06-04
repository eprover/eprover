#!/opt/local/bin/gawk -f

# Usage: get_system.awk [--force]
#
# Copyright 2002 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Determine a string describing the processor type and speed. Option
# --force will force only the first recognized processor to be
# considered.
#


# Similar to C assert()

function assert(val, comment)
{
   if(!val)
   {
      print "Assertion fail: " comment > "/dev/stderr";
      exit 1;
   }   
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


# Return the result of a single, simple shell command yieding exactly
# one line

function get_shell_res(cmd,   tmp)
{
   cmd | getline tmp;
   close(cmd);  

   assert(tmp, "No result found (get_shell_res)");
   return tmp;
}


# Return an OS string sufficient to tell us how to proceed.

function get_OS(             tmp, res, arr, elements)
{

  res = "";

  tmp = get_shell_res("uname -a");
  elements = split(tmp, arr);
  if(!elements)
  {
     print "Cannot get operation system info!" > "/dev/stderr";
     exit 1;
  }

  if((arr[1] == "SunOS")&& (index(arr[3], "5.") == 1))
  {
     res = "SunOS-5";
  }
  else if(arr[1] == "Linux")     
  {
     res = "Linux";
  }
  else if(arr[1] == "Darwin")     
  {
     res = "Darwin";
  }
  else
  {
     print "Warning: Unknown operating system!" > "/dev/stderr";
     res = "unknown";
  }
  return res;
}

function get_platform_info_SunOS5(    pipe, tmp, res, tmpres,  i, arr, elements)
{
   tmp = "";
   res = "";
   pipe = "psrinfo -v";
   while((pipe | getline tmp )>0)
   {
      if(i = index(tmp, "processor operates at"))
      {
	 elements = split(tmp, arr);
	 if(elements!=7)
	 {
	    print "Cannot parse processor information" > "/dev/stderr";
	    exit 1;
	 }
	 tmpres = arr[2] "-" arr[6];
	 if(res && (res!=tmpres))
	 {
	    if(first_proc_only)
	    {
	       print "Two different processors found! Ignoring second"\
		  " one (it's your responsibility now!)" >\
		  "/dev/stderr";
	       tmpres = "NC-" res;
	    }
	    else
	    {
	       print "Two different processors found! Machine"\
		  " is unsuitable for E-MARK evaluation" > "/dev/stderr";
	       exit 1;
	    }
	    exit 1;
	 }
	 else if(res)
	 {
	    print "Two or more identical processors found!" > "/dev/stderr";
	 }
	 res = tmpres;
      }
   }
   close(pipe);
   tmp = get_shell_res("uname -i");
   gsub(",","-",tmp);
   
   return tmp "-" res;
}


function get_platform_info_Linux(    pipe, tmp, res, tmpres,  i, arr, elements)
{
   pipe = "cat /proc/cpuinfo";
   res = "";
   tmpname = "";
   tmpmhz = "";
   while((pipe | getline tmp )>0)
   {
      if(i = index(tmp, "model name")==1)
      {
	 elements = split(tmp, arr, ": ");
	 if(elements!=2)
	 {
	    print "Cannot parse processor information" > "/dev/stderr";
	    exit 1;
	 }
	 tmpname = arr[2];
      }
      if(i = index(tmp, "cpu MHz")==1)
      {
	 elements = split(tmp, arr, ": ");
	 if(elements!=2)
	 {
	    print "Cannot parse processor information" > "/dev/stderr";
	    exit 1;
	 }
	 tmpmhz = round(arr[2]); # PCs have NO reliable frequency!
      }      
      if(i = index(tmp, "bogomips")==1)
      {
	 tmpres = tmpname "-" tmpmhz;
	 if(res && (res!=tmpres))
	 {
	    if(first_proc_only)
	    {
	       print "Two different processors found! Ignoring second"\
		  " one (it's your responsibility now!)" >\
		  "/dev/stderr";
	       tmpres = "NC-" res;
	    }
	    else
	    {
	       print "Two different processors found! Machine"\
		  " is unsuitable for E-MARK evaluation" > "/dev/stderr";
	       exit 1;
	    }
	 }
	 else if(res)
	 {
	    print "Two or more identical processors found!" > "/dev/stderr";
	 }
	 res = tmpres;
	 tmpres = "";
	 tmpname = "";
	 tmpmhz = "";
      }      
   }
   close(pipe);
   gsub(" ","-",res);
   gsub("\\(","",res);
   gsub("\\)","",res);
   
   return res;
}

function get_platform_info_Darwin(     pipe, tmp, i, arr, elements,
				       tmpname, freq)
{
   pipe = "hostinfo";

   while((pipe | getline tmp )>0)
   {
      if(i = index(tmp, "Processor type")==1)
      {
	 elements = split(tmp, arr, ": ");
	 if(elements!=2)
	 {
	    print "Cannot parse processor information" > "/dev/stderr";
	    exit 1;
	 }
	 tmp = arr[2];
	 split(tmp, arr, " ");
	 tmpname = arr[1];
      }
   }
   close(pipe);
   tmp = get_shell_res("sysctl -n hw.cpufrequency");
   freq = tmp/1000000
   return tmpname "-" freq;
}


function get_procinfo(   os, res)
{
   os = get_OS();
   
   if(os == "SunOS-5")
   {
      res = get_platform_info_SunOS5();
   }
   else if(os == "Linux")
   {
      res = get_platform_info_Linux();
   }
   else if(os == "Darwin")
   {
      res = get_platform_info_Darwin()
   }
   else 
   {
      assert((os==unknown), "OS identification failed (get_procinfo())");
      print "Warning: Unknown OS. Trying....";
      
      res = get_procinfo_SunOS5();
      if(!res)
      {
	 res = get_platform_info_Linux();
      }
      if(!res)
      {
	 print "Cannot get system information" > "/dev/stderr";
	 exit 1;
      }      
   }
   return res;
}


BEGIN{
   if(ARGV[1]=="--force")
   {
      first_proc_only = 1;
   }
   print get_procinfo();
}




