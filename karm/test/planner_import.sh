#!/bin/bash
# must be run with us or de keyboard
# must be started from the path where it resides
# must have Xautomation to run this

# TODO
# replace the sleep statements by something more elegant
# find out if this program runs in the background, if not, start it there
# decide: english version is <Alt> T for Task, german: <Alt> A for Aufgabe what to do here ?

#echo $PPID
#echo $PID
#echo $!
#echo $*
#echo $$
#echo $@
#echo $0
#echo $1
#echo $@

# 1. greet the user
echo "This is karmtest, a program to test karm."
echo "It is intended for developers who do changes to karm's sourcecode."
echo "Before publishing these changes, they should"
echo "(b) make sure the code still builds"
echo "(a) resolve all conflicts with the latest karm sourcecode"
echo "(c) run this program without receiving any errors"
echo
echo "This program simulates keypresses, so please leave the keyboard alone during the test. Please use a us or de keyboard layout (setxkbmap us). This must be run in X environment."
echo "Start this program solely with ./karmtest.sh"
echo "You must have XAutomation installed to run this."
echo "You must have your code changes to karm installed (make install)."
echo 
echo "Is that okay (y/n)"
read answer
if [ x$answer != x ]; then echo "Test not run because user aborted" >&2; exit 2; fi

# 1. call this program in the background
if [ x$1 != xbackgrounding ]; then ./karmtest.sh backgrounding $@ & exit; fi

# 1. test if Xautomation is useable
xte 'key Return'
rc=$?
if [ $rc = 127 ]; then echo "Test not run because XAutomation not installed" >&2; exit 1;
elif [ $rc = 1 ]; then echo "Test not run because no X display active" >&2; exit 3;
fi

# 1. test if we can use our testing files.
if test -e /tmp/karmtest.ics; then echo "Test not run because /tmp/karmtest.ics already existing" >&2; exit 4; fi
if test -e /tmp/example.planner; then echo "Test not run because /tmp/example.planner already existing" >&2; exit 4; fi

# 1. it gets serious
echo "Now fingers away from the keyboard ... "

killall karm
xte 'str karm'
xte 'key Return'
sleep 5

# 1. change preferences to a file we can use for testing
xte 'key Alt_L'
xte 'key Right'
xte 'key Right'
xte 'key Right'
xte 'key Down'
xte 'key Down'
xte 'key Return'
sleep 1
xte 'key Down'
xte 'key Down'
xte 'key Tab'
xte 'key Tab'
xte 'key Tab'
xte 'key Tab'
xte 'key Tab'
xte 'keydown Shift_L'
xte 'key /'
xte 'keyup Shift_L'
xte 'str tmp'
xte 'keydown Shift_L'
xte 'key /'
xte 'keyup Shift_L'
xte 'str karmtes'
xte 'str t.ics'
sleep 1
xte 'key Return'
xte 'key Return'
sleep 1

# 1. add a new task
# really, this once was impossible!
xte 'key Alt_L'
xte 'key Right'
xte 'key Right'
xte 'key Down'
sleep 1
xte 'key Return'
xte 'str exampl'
xte 'str e 1'
xte 'key Return'
sleep 1

# 1. import a planner file

# 1.1. create a planner file
cat >> /tmp/example.planner << endl
<?xml version="1.0"?>
<project name="" company="" manager="" phase="" project-start="20041101T000000Z" mrproject-version="2" calendar="1">
  <properties>
    <property name="cost" type="cost" owner="resource" label="Cost" description="standard cost for a resource"/>
  </properties>
  <phases/>
  <calendars>
    <day-types>
      <day-type id="0" name="Working" description="Ein Vorgabe-Arbeitstag"/>
      <day-type id="1" name="Nonworking" description="Ein Vorgabetag, an dem nicht gearbeitet wird"/>
      <day-type id="2" name="Basis verwenden" description="Use day from base calendar"/>
    </day-types>
    <calendar id="1" name="Vorgabe">
      <default-week mon="0" tue="0" wed="0" thu="0" fri="0" sat="1" sun="1"/>
      <overridden-day-types>
        <overridden-day-type id="0">
          <interval start="0800" end="1200"/>
          <interval start="1300" end="1700"/>
        </overridden-day-type>
      </overridden-day-types>
      <days/>
    </calendar>
  </calendars>
  <tasks>
    <task id="1" name="task 1" note="" work="28800" start="20041101T000000Z" end="20041101T170000Z" percent-complete="0" priority="0" type="normal" scheduling="fixed-work"/>
    <task id="2" name="task 2" note="" work="28800" start="20041101T000000Z" end="20041101T170000Z" percent-complete="0" priority="0" type="normal" scheduling="fixed-work">
      <task id="3" name="subtask 1-1" note="" work="28800" start="20041101T000000Z" end="20041101T170000Z" percent-complete="0" priority="0" type="normal" scheduling="fixed-work"/>
      <task id="4" name="subtask 1-2" note="" work="28800" start="20041101T000000Z" end="20041101T170000Z" percent-complete="0" priority="0" type="normal" scheduling="fixed-work"/>
    </task>
  </tasks>
  <resource-groups/>
  <resources/>
  <allocations/>
</project>
endl

# 1. quit karm
xte 'keydown Control_L'
xte 'key q'
xte 'keyup Control_L'
sleep 3
grep "SUMMARY:example 1" /tmp/karmtest.ics || echo "iCal file did not contain correct task" >&2
killall karm || echo "karm could be quit with CTRL_Q"

# 1. cleanup
rm /tmp/karmtest.ics
rm /tmp/example.planner
