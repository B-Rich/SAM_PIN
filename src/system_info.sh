#!/bin/bash

# Get system name and time
un="uname -a"
unval=`eval $un`
sysname="\t<$unval>"

# Get OS name
os_cmd="head -n1 /etc/issue"
os_name=`eval $os_cmd`
os_namexml="\t<$os_name>"

# Get total memery on system
memcmd="grep MemTotal /proc/meminfo"
meminfo=`eval $memcmd`
memxml="\t<$meminfo>"

# CPU Model
cpucmd='grep -m 1 "model name" /proc/cpuinfo'
cpumodel=`eval $cpucmd`
cpumodel=${cpumodel#*: }
cpuxml="\t<$cpumodel>"

echo -e "<System Information>\n$sysname\n$memxml\n$cpuxml\n</System Information>" | cat	- output.xml > temp && mv temp output.xml
