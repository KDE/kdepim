#!/usr/bin/php
<?

// Description:
// This program starts karm and simulates keypresses to do a real life-test of karm.
// This program returns zero if all tests went ok, else an error code.

// for those who do not know php:
// for a tutorial about php, check out www.usegroup.de
// for a reference about php, surf to www.php.net

// TODO
// Does it only run with us or de keyboard ? Or is the keyboard no matter ?
// What to do if my KDE is set to another lang and Task is not reachable via ALT_T ?
// prepare Windows-port
// how can we simulate a right cursor key via a pipe ? Perhaps with     
// fwrite($pipes[0],chr(27).chr(91).chr(67).chr(0));    

function createplannerexample()
{
$handle=fopen("/tmp/example.planner","w");
fwrite($handle,
'<?xml version="1.0"?>
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
');   
fclose($handle); 
}

echo "\nThis is lifetest.php, a program to test karm by starting it and simulating keypresses.\n";
echo "It is intended for developers who do changes to karm's sourcecode.\n
Before publishing these changes, they should
(a) resolve all conflicts with the latest karm sourcecode (cvs up)
(b) make sure the code still builds (make)
(c) run automated test routines like this (make check)\n\n";

echo "This program simulates keypresses, so please leave the keyboard alone during the test. Please use a us or de keyboardlayout (setxkbmap us). This must be run in X environment.\n
You must have XAutomation installed to run this.";
system("xte -h 2&>/dev/null",$rc);
if ($rc==0) echo " You have.\n";
if ($rc==127) echo " You do not have, please get it from http://hoopajoo.net/projects/xautomation.html .\n";
echo "This program will test karm by issueing karm, so, make sure, this calls the version you want to test (make install).\n\n";

echo "This program will now stop unless you gave the parameter --batch (confirming that you do not touch the keyboard)\n";   

$err="";
$exit=0;
if ($argv[1]=="--batch")
{
  system("xte 'key return' 2&>/dev/null",$rc);
  if ($rc==1) $err+="this must be run in an X environment\n";
  if ($rc==127) $err+="you do not have XAutomation installed, get it from http://hoopajoo.net/projects/xautomation.html\n";
  system ("rm /tmp/karmtest.ics 2>/dev/null");
  system ("rm /tmp/example.planner 2>/dev/null");
  if ($err=="")
  {
    echo "\nCalling karm...";
    
    $descriptorspec = array(
    0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
    1 => array("file", "/tmp/error-output.txt", "a"), 
    2 => array("pipe", "w")
    );
    $process = proc_open("karm", $descriptorspec, $pipes);
     
    while ((strpos($line,"karm: KarmStorage::save : wrote 0 tasks to") === false) and ($line<>"libkcal: ResourceLocal::reload()\n")) $line=fgets($pipes[2]);
    echo "karm is saving, we can start\n";
    
    
    system("sleep 1");

    
    system("xte 'key Alt_L'");
    
    system("xte 'key Right'");
    system("xte 'key Right'");
    system("xte 'key Right'");
    system("xte 'key Down'");
    system("xte 'key Down'");
    system("xte 'key Return'");
    system("sleep 1");
    system("xte 'key Down'");
    system("xte 'key Down'");
    system("xte 'key Tab'");
    system("xte 'key Tab'");
    system("xte 'key Tab'");
    system("xte 'key Tab'");
    system("xte 'key Tab'");
    system("xte 'keydown Shift_L'");
    system("xte 'key /'");
    system("xte 'keyup Shift_L'");
    system("xte 'str tmp'");
    system("xte 'keydown Shift_L'");
    system("xte 'key /'");
    system("xte 'keyup Shift_L'");
    system("xte 'str karm'");
    system("xte 'str te'");
    system("xte 'str st.'");
    system("xte 'str ics'");
    //fwrite($pipes[0],"/tmp/karmtest.ics\n");
    system("sleep 1");
    system("xte 'key Return'");
    system("sleep 1");
    system("xte 'key Return'");
    system("sleep 1");
    # 1. add a new task
    # really, this once was impossible!
    system("xte 'key Alt_L'");
    system("xte 'key Right'");
    system("xte 'key Right'");
    system("xte 'key Down'");
    system("sleep 1");
    system("xte 'key Return'");
    system("sleep 1");
    system("xte 'str exampl'");
    system("xte 'str e 1'");
    system("xte 'key Return'");
    system("sleep 1");
    
    echo "\nCreating a planner project file...";
    createplannerexample();
    
    system("xte 'key Alt_L'");
    system("xte 'key Down'");
    system("xte 'key Down'");
    system("xte 'key Down'");
    system("xte 'key Down'");
    system("xte 'key Down'");
    system("xte 'key Right'");
    system("xte 'key Down'");
    system("xte 'key Return'");
    system("sleep 1");
    system("xte 'keydown Shift_L'");
    system("xte 'key /'");
    system("xte 'keyup Shift_L'");
    system("xte 'str tmp'");
    system("xte 'keydown Shift_L'");
    system("xte 'key /'");
    system("xte 'keyup Shift_L'");
    system("xte 'str exa'");
    system("xte 'str mple'");
    system("xte 'str .pla'");
    system("xte 'str nner'");
    system("sleep 1");
    system("xte 'key Return'");
    system("sleep 1");
    
    echo "\nsending CTRL_Q...\n";
    system ("xte 'keydown Control_L'");
    system ("xte 'key Q'");
    system ("xte 'keyup Control_L'");
    
    $ics=fopen("/tmp/karmtest.ics","r");
    for ($i=1; $i<=13; $i++) $line=fgets($ics); 
    if ($line<>"SUMMARY:example 1\n") $err.="iCal file content was wrong";
    fclose($ics);
    system ("sleep 1");
  }
  echo $err;
  if ($err!="") exit(1);
}
?>