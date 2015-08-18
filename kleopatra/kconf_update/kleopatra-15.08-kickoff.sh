#!/bin/sh

sed -i "s/\/kleopatra.desktop/\/org.kde.kleopatra.desktop/" `kf5-config --path config --locate kickoffrc`
