#!/bin/sh

sed -i "s/\/kaddressbook.desktop/\/org.kde.kaddressbook.desktop/" `kf5-config --path config --locate kickoffrc`
