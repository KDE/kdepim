#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/kleopatra.desktop/\/org.kde.kleopatra.desktop/" $kickoffrcname
fi
