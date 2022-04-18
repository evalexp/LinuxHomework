# read -p "Please input a IP address: " ip

classifyNetworkType() {
    # check if is an ip addr
    echo $1 | grep -Eq "[1-2][0-9]{0,2}\.[0-9]{1,3}\.[0-9]{1,3}"
    if [ $? -ne 0 ]; then
        return 1
    fi

    ip=$1
    # get first 8 bits
    header=$(echo $ip | awk -F "." '{print $1}')
    # get second 8 bits
    sub=$(echo $ip | awk -F "." '{print $2}')
    # define the return var
    _RETURN=''
    # Class A network range [1,126]
    if [ $header -ge 1 -a $header -le 126 ]; then
        _RETURN=$_RETURN"$ip is a Class A Network, "
        # internal network 
        if [ $header -eq 10 ]; then
            _RETURN=$_RETURN"it's internal."
        else
            _RETURN=$_RETURN"it's external."
        fi
    # Local Loopback [127]
    elif [ $header -eq 127 ]; then
        _RETURN=$_RETURN"$ip is a Loopback Address."
        result=$_RETURN
        return 0
    # Class B network range[128,191]
    elif [ $header -ge 128 -a $header -le 191 ]; then
        _RETURN=$_RETURN"$ip is a Class B Network, "
        # internal network range 
        if [ $header -eq 172 ]; then
            if [ $sub -ge 16 -a $sub -le 31 ]; then
                _RETURN=$_RETURN"it's internal."
            else
                _RETURN=$_RETURN"it's external."
            fi
        fi
    # Class C network range[192,223]
    elif [ $header -ge 192 -a $header -le 223 ]; then
        _RETURN=$_RETURN"$ip is a Class C Network, "
        # internal network range
        if [ $header -eq 192 ]; then
            if [ $sub -eq 168 ]; then
                _RETURN=$_RETURN"it's internal."
            else
                _RETURN=$_RETURN"it's external."
            fi
        fi
    # Class D and E range
    elif [ $header -ge 223 -a $header -le 239 ]; then
        _RETURN=$_RETURN"$ip is a Class D Network, it's external."
    elif [ $header -ge 240 -a $header -le 255 ]; then
        _RETURN=$_RETURN"$ip is a Class E Network, it's external."
    fi
    # use var result to carry the return message
    result=$_RETURN
    # normal return
    return 0
}

