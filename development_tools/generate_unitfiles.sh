#!/bin/sh

if [ "$2" ]; then
    basefile=`basename $2 _K01_F2`	
    if test -f "${basefile}_K01_F2"; then    
	echo "Base file exists"
    else
	echo "Base file does not have the expected name"
	exit 1
    fi
    for line in `grep -v "#" $1 | sed -e 's/  */,/g'` ; do
        appendix=`echo $line|cut -d, -f1`
	argument=`echo $line|sed -e's/,/ /g'|cut -d' ' -f2-`
	file=${basefile}_$appendix
	if test -f "$file"; then
	    echo "File $file exists and left unchanged";
	else
	    echo Generating $file
	    sed -e "/Arguments: /s/Arguments: /Arguments: $argument /" -e"/Logfile:/s/K01_F2/$appendix/" $2 >$file
	fi
    done
else
    echo "Usage: generate_unitfiles.sh <key_file> <basefile>"
fi

