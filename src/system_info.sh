#!/bin/bash

# Get system name and time
un="uname -a"
unval=`eval $un`
sysname="\t<SystemName>$unval</SystemName>"

# Get OS name
os_cmd="head -n1 /etc/issue"
os_name=`eval $os_cmd`
os_namexml="\t<$os_name>"

# Get total memery on system
memcmd="grep MemTotal /proc/meminfo"
meminfo=`eval $memcmd`
memxml="\t<RAM>$meminfo</RAM>"

# CPU Model
cpucmd='grep -m 1 "model name" /proc/cpuinfo'
cpumodel=`eval $cpucmd`
cpumodel=${cpumodel#*: }
cpuxml="\t<CPUModel>$cpumodel</CPUModel>"

echo -e "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<SAM_PIN_INFO>\n\t<SystemInformation>\n\t\t$sysname\n\t\t$memxml\n\t\t$cpuxml\n\t</SystemInformation>" | cat	- output.xml > temp && mv temp output.xml
echo "</SAM_PIN_INFO>" >> output.xml
