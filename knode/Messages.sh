#!/bin/sh

$EXTRACTRC \
    $( find .                                                            \
            -name '*.rc' -a ! -name 'headers.rc' -a ! -name 'filters.rc' \
         -o -name '*.ui'                                                 \
         -o -name '*.kcfg'                                               \
    ) >> rc.cpp || exit 1

$XGETTEXT $(find . -name '*.cpp' -o -name '*.h') -o $podir/knode.pot
