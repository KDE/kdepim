#!/bin/sh

function do_test(){

DATE=10
MONTH=10
YEAR=2003

MINUTE=1
HOUR=20

while [  $DATE -lt 29 ];  do


COMMAND="$KONSOLEKALENDAR $1 --time $HOUR:$MINUTE --description $3 --summary $3 --date $YEAR-$MONTH-$DATE --create --file=$2"
echo using command $COMMAND
$COMMAND

let DATE=$DATE+1
let MINUTE=$MINUTE+1

done
}


KONSOLEKALENDAR=./konsolekalendar
ADDING="--add"
REMOVE="--delete"
CHANGE="--change"
VIEW="--view"

COMMAND="";

FILE="./testcaledar.ics"

do_test $ADDING $FILE "ADD"
do_test $CHANGE $FILE "CHANGE"
do_test $REMOVE $FILE "REMOVE"


