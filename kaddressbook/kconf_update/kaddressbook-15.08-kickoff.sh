#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/kaddressbook.desktop/\/org.kde.kaddressbook.desktop/" $kickoffrcname
fi
