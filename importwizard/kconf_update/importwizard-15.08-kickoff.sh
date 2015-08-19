#!/bin/sh

sed -i "s/\/importwizard.desktop/\/org.kde.importwizard.desktop/" `kf5-config --path config --locate kickoffrc`
