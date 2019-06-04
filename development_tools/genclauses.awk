#!/opt/local/bin/gawk -f

# Usage: genclauses.awk <length> <depth> [<seed>]
#
# Copyright 1998 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
#   Generate a random clause of lenght <lenght> and about maximal term
#   depth <depth> 
#

function randint(n) 
{
   return int(n * rand());
}

function genterm(depth,        res, arity)
{
   if(depth == 0)
   {
      return sig0[randint(4)];
   }
   arity = randint(2)+1;
   if(arity==1)
   {
      res = sprintf("f1%d(%s)", randint(3), genterm(depth-1));
   }
   else if(arity == 2)
   {
      res = sprintf("f2%d(%s,%s)", randint(3), 
		    genterm(depth-1),genterm(randint(depth-1)));      
   }
   else
   {
      res = sprintf("f3%d(%s,%s,%s)", randint(3), genterm(randint(depth-1)),
		    genterm(depth-1),genterm(randint(depth-1)));      
   }
   return res;
}

function genliteral(depth)
{
   if(rand()>0.5)
   {
      if(rand()>0.5)
      {
	 return genterm(depth);
      }
      else
      {
	 return "~" genterm(depth);
      }
   }
   if(rand()>0.5)
   {
      return genterm(depth) "!=" genterm(randint(depth+3));
   }
   else
   {
      return genterm(depth) "=" genterm(randint(depth+3));
   }
}

function genclause(len, depth, array)
{
   for(i=0; i<len; i++)
   {
      array[i] = genliteral(depth);
   }
}

function printclause(array,     i, sep)
{
   sep = "";
   for(i=0; array[i]; i++)
   {
      printf sep array[i];
      sep = ";";
   }
   print "<-.";
}

function permute_clause(array, perm, len,    i, picked, store)
{
   print len;
   if(len == 0)
   {      
      printclause(perm);      
   }
   else
   {
      for(i in array)
      {
	 picked = i;
	 store = array[i];
	 perm[len-1] = array[i];
	 delete array[i];
	 permute_clause(array, perm, len-1);
	 array[picked] = store;
	 res = 1;
      }
   }
}


BEGIN{
   if(ARGV[2] = "")
   {
      print "Usage: genclauses.awk <length> <depth> [<seed>]";
      exit 1;
   }
   srand(ARGV[3]);
   sig0[0]="a";
   sig0[1]="b";
   sig0[2]="X";
   sig0[3]="Y";

   genclause(ARGV[1], ARGV[2], array);
   print "Clause:";
   printclause(array);
   permute_clause(array, perm, ARGV[1]);
}






