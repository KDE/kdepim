#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/akonadiconsole.desktop/\/org.kde.akonadiconsole.desktop/" $kickoffrcname
fi
