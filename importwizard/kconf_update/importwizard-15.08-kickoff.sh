#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/importwizard.desktop/\/org.kde.importwizard.desktop/" $kickoffrcname
fi
