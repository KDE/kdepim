#!/bin/sh

QT_DIR=/dvl/kde/branches/work/komo/install/lib

echo $LD_LIBRARY_PATH | grep "$QT_DIR" > /dev/null 2>&1

if [ "$?" -ne "0"  ]; then
	export LD_LIBRARY_PATH=$QT_DIR:$LD_LIBRARY_PATH
fi

/dvl/kde/branches/work/komo/install/bin/kaddressbook-mobile
