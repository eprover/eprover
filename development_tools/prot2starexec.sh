#!/bin/tcsh -f

# Read a standard E protocol file

touch newbla  newbla2 newbla3
rm newbla newbla2 newbla3

foreach word (`grep '#' $1 | head -1 | sed -e 's/, /,/g' -e 's/\*/./g'`)
   echo $word >> newbla
end

foreach word (`grep -v '#' newbla |grep -v cpu-limit | grep -v delete-bad-limit | grep -v print-pid | grep -v print-statistics | grep -v print-version | grep -v proof-object | grep -v record-gcs |grep -v resources-info |grep -v tptp3-in | grep -v tstp-format | grep -v tstp-in  | grep -v sandbox | grep -v -- '^-s$' | grep -v -- '^-R$' | grep -v definitional-cnf=24 |grep -v eprover `)
echo -n $word ' ' |grep -- "-H" >> newbla2
echo -n $word ' ' |grep -v -- "-H" >> newbla3
end
#echo `cat newbla2`
#echo `cat newbla3`

set name="starexec_run_E---2.0_"`basename -s .csv $1|cut -d_ -f2-`

head -4 /Users/schulz/SOURCES/Projects/E/etc/starexec_run_E---2.0_XXX > $name
echo 'set flags="' `cat newbla3` '"' >> $name
echo 'set heuristic="' ` sed -e "s/'//g" newbla2` '"' >>$name
tail -19 /Users/schulz/SOURCES/Projects/E/etc/starexec_run_E---2.0_XXX >> $name
