#!/bin/sh

# Create files relative to current directory if no "/" prefix
# in file name given on command line

TESTFILE="testkarm.ics"
TESTTODO="testtodo"

# If runscripts sees output on stderr, it considers this a test failure
DCOPID=`dcop | grep karm 2>/dev/null`

if [ -n $DCOPID ]; then dcop $DCOPID KarmDCOPIface quit; fi;

if [ -e $TESTFILE ]; then rm $TESTFILE; fi

karm $TESTFILE & 

sleep 2

DCOPID=`dcop | grep karm`

# karm does not write file until data is saved.

echo "dcop $DCOPID KarmDCOPIface addtodo \"$TESTTODO\""
dcop $DCOPID KarmDCOPIface addtodo "$TESTTODO"

RVAL=1
if [ -e $TESTFILE ]; then RVAL=0; fi

# clean up
if [ -n $DCOPID ]; then dcop $DCOPID KarmDCOPIface quit; fi;
if [ -e $TESTFILE ]; then rm $TESTFILE; fi

if [ $RVAL -eq 0 ]
then 
  echo "PASS $0"
  exit 0
else 
  echo "FAIL $0"
  exit 1
fi
