#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Argument is git commit message."
else
#make clean -- Now I have gitignore... should push it
	git add .
	git commit -m "$1"
	git push
fi


