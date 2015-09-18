#!/bin/bash
# watchdog script for monitord
# add this script to your crontab by using `crontab -e`

# "monitord status" returns 0 if it is not running
/etc/init.d/monitord status >> /dev/null


# try to restart
if [ $? -ne 0 ]
then
        echo "not running... trying to restart"
        /etc/init.d/monitord start

        if [ $? -ne 0 ]
        then
                echo "failed!"
        fi
fi

