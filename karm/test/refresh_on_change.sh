#!/bin/sh


source __lib.sh

TESTFILE="/tmp/testkarm.ics"
set_up

TODO_NAME=$0
TODO_UID=abc-123
TODO_TIME=`date +%Y%m%dT%H%M%SZ`

# file is empty, as karm doesn't write anything till there is data.
cat >> $TESTFILE << endl
BEGIN:VCALENDAR
PRODID:-//K Desktop Environment//NONSGML KArm Test Scripts//EN
VERSION:2.0

BEGIN:VTODO
DTSTAMP:$TODO_TIME
ORGANIZER;CN=Anonymous:MAILTO:nobody@nowhere
CREATED:$TODO_TIME
UID:$TODO_UID
SEQUENCE:0
LAST-MODIFIED:$TODO_TIME
SUMMARY:$TODO_NAME
CLASS:PUBLIC
PRIORITY:5
PERCENT-COMPLETE:0
END:VTODO

END:VCALENDAR
endl

# wait so FAM and KDirWatcher tell karm and karm refreshes view
sleep 2

RVAL=`dcop $DCOPID KarmDCOPIface hastodo $TODO_NAME`
#echo "RVAL = $RVAL"

tear_down

# check that todo was found
if [ "$RVAL" == "$TODO_UID" ]; then 
  echo "PASS $0"
  exit 0;
else 
  echo "FAIL $0: got /$RVAL/, expected /$TODO_UID/"
  exit 1;
fi
