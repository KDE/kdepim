#!/bin/sh

sed -i "s/\/pimsettingexporter.desktop/\/org.kde.pimsettingexporter.desktop/" `kf5-config --path config --locate kickoffrc`
