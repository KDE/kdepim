#! /bin/sh
$XGETTEXT text_vcard.cpp rc.cpp -o $podir/kmail_text_vcard_plugin.pot
$XGETTEXT text_calendar.cpp attendeeselector.cpp delegateselector.cpp rc.cpp -o $podir/kmail_text_calendar_plugin.pot
$XGETTEXT text_xdiff.cpp rc.cpp -o $podir/kmail_text_xdiff_plugin.pot
