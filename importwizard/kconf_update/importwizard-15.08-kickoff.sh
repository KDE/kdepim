#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/importwizard.desktop/\/org.kde.importwizard.desktop/" $kickoffrcname
fi
