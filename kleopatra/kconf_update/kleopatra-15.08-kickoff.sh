#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/kleopatra.desktop/\/org.kde.kleopatra.desktop/" $kickoffrcname
fi
