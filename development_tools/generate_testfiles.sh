#!/bin/sh

if [ "$2" ]; then
    basefile=`basename $2 _0`	
    if test -f "${basefile}_0"; then    
	echo "Base file exists"
    else
	echo "Base file has not the expected name"
	exit 1
    fi
    for line in `sed -e 's/  */,/g' $1` ; do
        appendix=`echo $line|cut -d, -f1`
	argument=`echo $line|cut -d, -f2`
	file=${basefile}_$appendix
	if test -f "$file"; then
	    echo "File $file exists and left unchanged";
	else
	    echo Generating $file
	    sed -e "/Arguments:/s/NoSelection/$argument/" -e"/Logfile:/s/0$/$appendix/" $2 >$file
	fi
    done
else
    echo "Usage: generate_testfiles.sh <key_file> <basefile>"
fi

