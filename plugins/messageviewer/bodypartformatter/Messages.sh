#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT text_vcard.cpp updatecontactjob.cpp rc.cpp -o $podir/messageviewer_text_vcard_plugin.pot
$XGETTEXT text_calendar.cpp attendeeselector.cpp delegateselector.cpp rc.cpp -o $podir/messageviewer_text_calendar_plugin.pot
$XGETTEXT text_xdiff.cpp rc.cpp -o $podir/messageviewer_text_xdiff_plugin.pot
$XGETTEXT application_ms-tnef.cpp -o $podir/messageviewer_application_mstnef_plugin.pot
