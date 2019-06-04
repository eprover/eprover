#!/opt/local/bin/gawk -f

# Usage: histogramm.awk <input file>
#
# Read a number of positive integers and print (and plot) a
# distribution.
#


BEGIN{
   minimum  = 2000000000;
   maximum  = 0;
   maxcount = 0;
   compress = 7;
   ignore_low_limit=0;
   all_int_keys = 1;

   for(i=0; i< ARGC; i++)
   {
      if(ARGV[i] == "--ignore-lower-limit")
      {
         ignore_low_limit = ARGV[i+1];
         ARGV[i] = "";
         ARGV[i+1] = "";
      }
   }
}

/^\#/{
   print "Skipping comment";
   next;
}

/[A-Za-z0-9]+/{
   if(0+$1 < ignore_low_limit)
   {
      next;
   }
   if(int($1) != $1)
   {
      all_int_keys = 0;      
   }
   array[$1]++;
   if(array[$1] > maxcount)
   {
      maxcount = array[$1]
   }
   if(0+$1 < minimum)
   { 
      minimum = 0+$1;
   }
   if(0+$1 > maximum)
   { 
      maximum = 0+$1;
   }
}

function line(val,    res,limit,i)
{
   res   = "";
   limit = val *60 / maxcount;
   for(i=0; i<limit; i++)
   {
      res = res "#";
   }
   return res;
}


END{
   sum = 0;

   if(! all_int_keys)
   {
      j = 1;
      for (i in array)
      {
         ind[j] = i;    # index value becomes element value
         j++;
      }
      n = asort(ind);    # index values are now sorted
      for (i = 1; i <= n; i++)
      {
          curr = 0+array[ind[i]];
          printf "%8.4f: %4d  ", ind[i], curr;
          print line(curr);
          sum+=curr;
      }
      l1 = sum/3;
      c1 = 0;
      c2 = 0;
      l2 = sum*2/3;
      sum = 0;
      for(i=minimum; i<=maximum; i++)
      {
         curr = 0+array[i];
         sum += curr;
         if(sum >= l1 &&!c1)
         {
            c1 = i;
         }
         if(sum >= l2 &&!c2)
         {
            c2 = i;
         }
      }      
   }
   else
   {
      for(i=minimum; i<=maximum; i++)
      {
         curr = 0+array[i];
         printf "%4d: %4d  ", i, curr;
         print line(curr);
         sum+=curr;
         if(!curr)
         {
            for(j=i; !array[j]&&(j<maximum); j++);
# print j, i, j-i, compress
            if(j-i >= compress)
            {
               print ".\n.";
               i=j-2;
            }
         }
      }
      l1 = sum/3;
      c1 = 0;
      c2 = 0;
      l2 = sum*2/3;
      sum = 0;
      for(i=minimum; i<=maximum; i++)
      {
         curr = 0+array[i];
         sum += curr;
         if(sum >= l1 &&!c1)
         {
            c1 = i;
         }
         if(sum >= l2 &&!c2)
         {
            c2 = i;
         }
      }
   }
   printf "Suggested partition: %d-%d, %d-%d, %d-%d\n",
      minimum,c1,c1, c2,c2,maximum;
   printf "some_limit=%d, many_limit=%d\n",c1,c2;
}
