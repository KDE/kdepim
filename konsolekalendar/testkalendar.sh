##########################################################################################
#
#  ***************************************************************************
#  *                                                                         *
#  *   This program is free software; you can redistribute it and/or modify  *
#  *   it under the terms of the GNU General Public License as published by  *
#  *   the Free Software Foundation; either version 2 of the License, or     *
#  *   (at your option) any later version.                                   *
#  *                                                                         *
#  ***************************************************************************
#
##########################################################################################
#
# Purpose of this program is to test konsolekalendar in future this will be more smarter
#
# testkalendar.sh 							0.1
#
##########################################################################################

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

echo "################################ TEXT ##################################"
COMMAND="$KONSOLEKALENDAR --view --time $HOUR:$MINUTE --date $YEAR-$MONTH-$DATE --export-type Text --file=$2"
$COMMAND

echo "################################ HTML ##################################"
COMMAND="$KONSOLEKALENDAR --view --time $HOUR:$MINUTE --date $YEAR-$MONTH-$DATE --export-type HTML --file=$2"
$COMMAND


echo "################################ CSV ##################################"
COMMAND="$KONSOLEKALENDAR --view --time $HOUR:$MINUTE --date $YEAR-$MONTH-$DATE --export-type CSV --file=$2"
$COMMAND

echo "################################ ORG ##################################"
COMMAND="$KONSOLEKALENDAR --view --time $HOUR:$MINUTE --date $YEAR-$MONTH-$DATE --export-type Text-organizer --file=$2"
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


