#!/opt/local/bin/gawk -f

# Generate a set of classified terms

function genterm(depth,    tmp, res)
{
  tmp = rand();

  if(depth == 0 || tmp > 0.8)
  {
     tmp = rand();
     if(tmp < 0.25)
     {
	res = "a";
     }
     else if(tmp < 0.5)
     {
	res = "b";
     }
     else if(tmp < 0.75)
     {
	res = "c";
     }
     else 
     {
	res = "d";
     }
  }
  else
  {
     tmp = rand();
     if(tmp < 0.25)
     {
	res = "i(" genterm(depth-1) ")";
     }
     else if(tmp < 0.5)
     {
	res = "j(" genterm(depth-1) ")";
     }
     else if(tmp < 0.75)
     {	
	res = "f(" genterm(depth-1) "," genterm(depth-1) ")";
     }
     else
     {
	res = "g(" genterm(depth-1) "," genterm(depth-1) ")";
     }
  }
  return res;
}

function classifyterm(term)
{
   if(index(term, "f(i(")==1 || index(term, "g(i(")==1)
#   if(index(term, "i"))
   {
      return 1;
   }
   return -1;
}

BEGIN{
  if(!ARGV[2])
    {
      print "Usage: classgen.awk <depth> <number>" > "/dev/stderr";
      exit 1;
    }
  depth =  ARGV[1];
  number = ARGV[2];
  ARGV[1] = "";
  ARGV[2] = "";
  
  srand();
  i=0;
  class1=0;
  while(i<number)
  {
     term = genterm(depth);
     if(!(term in terms))
     {
	class = classifyterm(term);
	tmpstring = sprintf("%s: 1: (1, %d).", term, class);
	terms[term] = tmpstring;
	i++;
	if(class == 1)
	{
	   class1++;
	}
     }
  }
  print "# " i " terms, split = " class1/i > "/dev/stderr";
  for(term in terms)
  {
     print terms[term];
  }
}

