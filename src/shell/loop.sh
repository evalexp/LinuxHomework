#!/bin/bash

source ./classifyNetwork.sh

RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
RESET=$(tput sgr0)

# redefine stdout
error(){
    echo "$RED$1$RESET"
}

success(){
    echo "$GREEN$1$RESET"
}

warn(){
    echo "$YELLOW$1$RESET"

}

# test ip if is reachable
while read ip
do
    warn ">>>>>>>>>>> $ip <<<<<<<<<<<<<<<"
    try_times=0
    # check ip if is legal and classify
    classifyNetworkType $ip
    if [ $? -eq 1 ]; then
        continue
    fi
    # try four times to reach the host
    for((i=0;i<4;i++));
    do
        ping -c1 -W1 $ip &>/dev/null
        if [ $? -eq 0 ]; then
            success "$ip is reachable."
            echo $result
            break 
        else
            warn "Retry to reach $ip, times: $[ $try_times + 1 ]"
            let try_times++
        fi
    done
    # unreachable
    if [ $try_times -eq 4 ]; then
        error "$ip is unreachable."
        echo $result
    fi
done < ip.txt
