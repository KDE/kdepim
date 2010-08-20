#!/bin/bash

CONFIG_H=config-messageviewer.h

#
# Adds #include <$CONFIG_H> to all implementation files,
# right before the first existing preprocessor directive (^#)
#

my_REPLACEMENT_TEXT="#include <$CONFIG_H>

"

find "$@" -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' -o -name '*.c' | \
    xargs grep -LE '# *include *[<"]'"$CONFIG_H"'[>"]' | \
    xargs perl -0777 -pi\~ -e "s/^#/${my_REPLACEMENT_TEXT}#/m;"
