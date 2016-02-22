#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/storageservicemanager.desktop/\/org.kde.storageservicemanager.desktop/" $kickoffrcname
fi
