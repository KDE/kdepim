#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/blogilo.desktop/\/org.kde.blogilo.desktop/" $kickoffrcname
fi
