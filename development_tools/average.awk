#!/opt/local/bin/gawk -f
#
# Compute average and standard deviation for a list of values.

BEGIN{
   i=0;
}

{
   if($0!="")
   {
      array[i++] = $0;
   }
   else
   {
      print("Ignoring empty line");
   }
}



END{
   total = i;
   sum = 0;
   for(i=0; i<total; i++)
   {
      sum+=array[i];
   }
   avg = sum/total;   

   var = 0;
   for(i=0; i<total; i++)
   {
      var += (array[i]-avg)*(array[i]-avg);
   }
   var = var/(total-1);
   dev = sqrt(var);
   
   printf("Sum over %4d elements: %f\n", total, sum);
   printf("Average               : %f +/- %f\n", avg, dev);
}
