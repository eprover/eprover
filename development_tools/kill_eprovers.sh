#!/bin/sh

ps -elf | grep schulz | grep eprover | grep -v grep | gawk '{system ( "kill "$4 ) ; print "Killed "$4}'