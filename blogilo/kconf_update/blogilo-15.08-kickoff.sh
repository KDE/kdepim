#!/bin/sh

sed -i "s/\/blogilo.desktop/\/org.kde.blogilo.desktop/" `kf5-config --path config --locate kickoffrc`
