#!/bin/sh

sed -i "s/\/storageservicemanager.desktop/\/org.kde.storageservicemanager.desktop/" `kf5-config --path config --locate kickoffrc`
