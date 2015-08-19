#!/bin/sh

sed -i "s/\/sieveeditor.desktop/\/org.kde.sieveeditor.desktop/" `kf5-config --path config --locate kickoffrc`
