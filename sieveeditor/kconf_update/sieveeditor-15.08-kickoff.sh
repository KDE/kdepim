#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/sieveeditor.desktop/\/org.kde.sieveeditor.desktop/" $kickoffrcname
fi
