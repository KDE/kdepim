#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/pimsettingexporter.desktop/\/org.kde.pimsettingexporter.desktop/" $kickoffrcname
fi
