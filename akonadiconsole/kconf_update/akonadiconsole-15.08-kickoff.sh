#!/bin/sh

sed -i "s/\/akonadiconsole.desktop/\/org.kde.akonadiconsole.desktop/" `kf5-config --path config --locate kickoffrc`
