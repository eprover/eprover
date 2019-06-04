#!/opt/local/bin/gawk -f

# Generate a set of random terms

function get_arity(limit,     res)
{
   res = 0;
   while((rand() < limit) && (res < 3))
   {
      limit = limit/2;
      res++;
   }
   return res;
}


function get_depth_limit_arity(limit, depth)
{
   if(rand() < 2/(depth+1))
   {
      return get_arity(limit);
   }
   return 0;	 
}


function randint(n) 
{
   return int(n * rand());
}


function get_symbol(arity)
{
   return "f" arity randint(4-arity);   
}

function gen_term(limit, depth,    pred, arity, res, i)
{   
   arity = get_depth_limit_arity(limit, depth);
   res = get_symbol(arity);
   if(arity!=0)
   {
      pred = "(";
      for(i=0; i<arity; i++)
      {
	 # print depth ":" i "-" res;
	 res = res pred gen_term(limit, depth+1);
	 pred = ",";
      }
      res = res ")";
   }
   return res;
}


BEGIN{

  if(!ARGV[2])
    {
      print "Usage: termgen.awk <number> <limit>" > "/dev/stderr";
      exit 1;
    }
  limit  = ARGV[2];
  number = ARGV[1];
  ARGV[1] = "";
  ARGV[2] = "";
  
  srand();
  
  for(i=0; i<number; i++)
  {
     print gen_term(limit,0);
  }
}

