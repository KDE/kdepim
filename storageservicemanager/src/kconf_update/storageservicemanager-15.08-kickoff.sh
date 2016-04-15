#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/storageservicemanager.desktop/\/org.kde.storageservicemanager.desktop/" $kickoffrcname
fi
