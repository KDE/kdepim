#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/kaddressbook.desktop/\/org.kde.kaddressbook.desktop/" $kickoffrcname
fi
