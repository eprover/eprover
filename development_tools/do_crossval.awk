#!/opt/local/bin/gawk -f

BEGIN{
   args = "";
   for(i=1; ARGV[i]; i++)
   {
      args = args " " ARGV[i];
   }
   print "Args: " args;
}

function success_rate(file    , tmp, succ)
{
   pipe = "tsm_classify " args " " file;
   while ((pipe | getline tmp) > 0)
   {
      if(match(tmp, /successes\,/))
      {
	 succ = substr(tmp, RSTART+RLENGTH);
	 print succ;
      }
   }	 
   close(pipe);

   return 0.0+succ;
}

BEGIN{
   
   sum = 0;
   count = 0;
   for(i=1; i<=10; i++)
   {
      succ[i] = success_rate(sprintf("crossval%02d", i));
      print succ[i];
      count++;
      sum+=succ[i];
   }
   avg = sum/count;   

   var = 0;
   for(i=1; i<=10; i++)
   {
      var += (succ[i]-avg)*(succ[i]-avg);
   }
   var = var/(count-1);
   dev = sqrt(var);
   
   print "Success rate: " avg " +/-" dev;
}
