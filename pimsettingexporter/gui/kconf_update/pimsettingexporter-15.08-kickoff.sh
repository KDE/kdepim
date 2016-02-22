#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/pimsettingexporter.desktop/\/org.kde.pimsettingexporter.desktop/" $kickoffrcname
fi
