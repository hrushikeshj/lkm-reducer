#! /bin/bash

mod=$2
mod_name=$3

if [ -z $2 ]
then
    mod='reducer.ko'
fi

if [ -z $3 ]
then
    mod_name=reducer
fi

echo $mod, $mod_name

if [[ $1 = "del" ]]
then
    rm /dev/$mod_name
    rmmod $mod
else
    insmod $mod
    dev_no=$(cat /proc/devices  | grep $mod_name | awk '{print $1;}')
    echo $dev_no
    mknod /dev/$mod_name c $dev_no 0
    echo /dev/$mod_name
fi
