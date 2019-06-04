#!/opt/local/bin/gawk -f

function get_MHz(host,   pipe, tmp, res, i)
{
  tmp = "";
  res = "";
  pipe = "ssh -1 " host " psrinfo -v";
  while((pipe | getline tmp )>0)
  {
     if(i = index(tmp, mhz_string))
     {
	print tmp, i
	res = int(substr(tmp, i+mhz_string_l))
	break;
     }
  }
  close(pipe);  

  return res;
}


BEGIN{
   mhz_string = "processor operates at ";
   mhz_string_l = length(mhz_string);
   print get_MHz("sunjessen5")
    }
