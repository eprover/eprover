#! /bin/sh
#
# Loop over a problem set. If
# /usr/wiss/stenzg/home_sun/tmp/SUNHALLE_RESERVED exists, run on
# sunhalle52-107, otherwise use all hosts. 
#

MYHOME=/usr/gast/schulz/home_sun/
STENZHOME=/usr/gast/stenzg/home_sun/

#MYHOME=/home/schulz
#STENZHOME=/home/schulz

touch $MYHOME/dummy;

while test -f $MYHOME/dummy; do
    echo New iteration - checking for jobs
    if test -f $MYHOME/distribute_jobs; then
	for file in `cat $MYHOME/distribute_jobs`; do
	    echo Running on $file
	    if test -f $STENZHOME/tmp/SUNHALLE_RESERVED; then    
		distribute_eprover.awk $file 52 107;
	    else
		distribute_eprover.awk $file;
	    fi;
	done;
        mv $MYHOME/distribute_jobs $MYHOME/distribute_jobs_done
	if test -f $MYHOME/distribute_jobs_queued; then
	    mv $MYHOME/distribute_jobs_queued $MYHOME/distribute_jobs
        fi;
    else
        if test -f $MYHOME/distribute_jobs_queued; then
	    mv $MYHOME/distribute_jobs_queued $MYHOME/distribute_jobs
	else
	    echo Sleeping
	    sleep 300;	
        fi;
    fi;
done;
