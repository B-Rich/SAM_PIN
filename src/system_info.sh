#!/bin/bash

echo "<System Information>" > system.xml

# Get system name and time
un="uname -a"
unval=`eval $un`
sysname="\t<$unval>"
echo -e $sysname >> system.xml

# Get OS name
os_cmd="head -n1 /etc/issue"
os_name=`eval $os_cmd`
os_namexml="\t<$os_name>"
echo -e $os_namexml >> system.xml

# Get total memery on system
memcmd="grep MemTotal /proc/meminfo"
meminfo=`eval $memcmd`
memxml="\t<$meminfo>"
echo -e $memxml >> system.xml

# CPU Model
cpucmd='grep -m 1 "model name" /proc/cpuinfo'
cpumodel=`eval $cpucmd`
cpumodel=${cpumodel#*: }
cpuxml="\t<$cpumodel>"
echo -e $cpuxml >> system.xml

echo "</System Information>" >> system.xml
