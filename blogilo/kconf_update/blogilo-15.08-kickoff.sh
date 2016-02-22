#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/blogilo.desktop/\/org.kde.blogilo.desktop/" $kickoffrcname
fi
