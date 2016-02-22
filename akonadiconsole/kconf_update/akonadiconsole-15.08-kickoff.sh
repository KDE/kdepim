#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/akonadiconsole.desktop/\/org.kde.akonadiconsole.desktop/" $kickoffrcname
fi
