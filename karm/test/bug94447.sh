#!/bin/sh

# Create files relative to current directory if no "/" prefix
# in file name given on command line

TESTFILE="testkarm.ics"
TESTTODO="testtodo"

echo "mkb: pwd=`pwd`"

echo "mkb: source __lib.sh"
source __lib.sh 

echo "mkb: call set_up"
set_up

# make karm create the file.
echo "mkb: dcop cmd"
dcop $DCOPID KarmDCOPIface addtodo "$TESTTODO"

RVAL=1
if [ -e $TESTFILE ]; then RVAL=0; fi

echo "mkb: tear_down"
tear_down

if [ $RVAL -eq 0 ]
then 
  echo "PASS $0"
  exit 0
else 
  echo "FAIL $0"
  exit 1
fi
