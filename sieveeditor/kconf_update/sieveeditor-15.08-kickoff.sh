#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/sieveeditor.desktop/\/org.kde.sieveeditor.desktop/" $kickoffrcname
fi
