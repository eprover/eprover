#!/opt/local/bin/gawk -f
#
# Usage: make_test.awk <file>
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Read a file, split it randomly into 10 subsets and print the 10
# 10/90 files fpr cross-validation.
#

BEGIN{
   i=0;
   file1 = "__tmprand__";
   file2 = "__tmpsort__";
}

/^#/{
  next;
}

{
   i++;
   printf("%5.4f :%s\n", rand(), $0) > file1;
}

END{
   close(file1);
   system("sort " file1 " | cut -d\: -f2- > " file2 ";rm " file1);

   total = i;
   i=0;
   while ((getline tmp < file2) > 0)
   {
      count[i]++;
      set[i] = set[i] tmp"\n";
      i++;
      if(i==10)
      {
	 i=0;
      }
   }
   system("rm " file2);

   for(i=0; i<10; i++)
   {
      if(i<9)
      {
	 file = "crossval0" i+1;
      }
      else
      {
	 file = "crossval10";
      }
      print "# Split " i+1 " - " total-count[i] "/" 0+count[i] " Terms">file; 
      print "Training: \n">file;
      for(j=0; j<10; j++)
      {
	 if(i!=j)
	 {
	    printf set[j]>file;
	 }
      }
      print ".\n">file;
      print "Test: \n">file;
      printf set[i]>file;
      print ".">file;
      close(file);
   }
}
