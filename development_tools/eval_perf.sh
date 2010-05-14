#!/bin/tcsh -f


touch real_perf
rm real_perf
foreach file ( ../CLASS_LISTS_D6/CLASS_* )
   echo $file `filter_results.awk $file $1 | eauswert.awk | gawk '{print $5+0}'` |cut -d/ -f3- >> real_perf
   echo $file
end
grep CLASS $2 | grep prot | cut -c4- | sort > pred_perf
join real_perf pred_perf | gawk '{printf("%-21s:%-53s:%4d %4d:%+4d\n",$1,$4,$2,$5,$2-$5)}' |tee cmp_perf

