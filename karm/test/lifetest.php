#!/usr/bin/php
<?

// Description:
// This program starts karm and simulates keypresses to do a real life-test of karm.
// This program returns zero if all tests went ok, else an error code.
// You need a US or DE keyboard to run this.

// for those who do not know php:
// for a tutorial about php, check out www.usegroup.de
// for a reference about php, surf to www.php.net

// TODO
// prepare Windows-port

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
};

function simkey($s)
// This function simulates keypresses that form the string $s, e.g. for $s==hallo, it simulates the keypress of h, then a, then l and so on.
// find a useful list of keycodes under /usr/include/X11/keysymdef.h
{
  for ($i=0; $i<strlen($s); $i++)
  {
    usleep(10000);            # this is heuristic, its need is related to X.org bug #2710
    if ($s[$i]=="/") system("xte 'key KP_Divide'");
    else system("xte 'key ".$s[$i]."'");
    usleep(10000);            # this is heuristic, its need is related to X.org bug #2710
  }
}

function keysim($s)
// remove everything that makes you have to think twice!!
{
  simkey($s);
}

function funkeysim($s, $count=1)
// same as keysim, but interprets $s as function key name to be used by xte and expects a $count to indicate how often key is to be pressed
{
  for ($i=1; $i<=$count; $i++) 
  {
    usleep(100000);            # this is heuristic, its need is related to X.org bug #2710
    $rc=exec("xte 'key $s'");
    usleep(100000);            # this is heuristic, its need is related to X.org bug #2710
  }
  return $rc;
}

// int main()
if ($argv[1]!="--batch")
{
  echo "\nThis is lifetest.php, a program to test karm by starting it and simulating keypresses.\n";
  echo "It is intended for developers who do changes to karm's sourcecode.\n";
  echo "Before publishing these changes, they should\n";
  echo "(a) resolve all conflicts with the latest karm sourcecode (cvs up)\n";
  echo "(b) make sure the code still builds (make)\n";
  echo "(c) run automated test routines like this (make check)\n\n";
  
  echo "This program simulates keypresses, so please leave the keyboard alone during the test. Please use a us or de keyboardlayout (setxkbmap us). This must be run in X environment.\n
  You must have XAutomation installed to run this.";
  system("xte -h 2&>/dev/null",$rc);
  if ($rc==0) echo " You have.\n";
  if ($rc==127) echo " You do not have, please get it from http://hoopajoo.net/projects/xautomation.html .\n";
  echo "This program will test karm by issueing karm, so, make sure, this calls the version you want to test (make install).\n\n";

  echo "This program will now stop unless you give the parameter --batch (confirming that you do not touch the keyboard)\n";   

  $err="";
  $exit=0;
}
else
{
  switch (funkeysim("Alt_L")) 
  {
    case 1: 
      $err.="this must be run in an X environment\n";
    break;
    case 127: 
      $err.="you do not have XAutomation installed, get it from http://hoopajoo.net/projects/xautomation.html\n";
    break;
  }
  unlink ("/tmp/karmtest.ics");
  unlink ("/tmp/example.planner");
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
    
    
    sleep (1);

    
    funkeysim("Alt_L");
    
    funkeysim("Right",3);
    funkeysim("Down",2);
    funkeysim("Return");
    sleep (1);
    funkeysim("Down",2);
    funkeysim("Tab",5);
    simkey("/tmp/karmtest.ics");
    sleep (1);
    funkeysim("Return");
    sleep (1);
    funkeysim("Return");
    sleep (1);

    # 1. add a new task
    funkeysim("Alt_L");
    funkeysim("Right",2);
    funkeysim("Down");
    sleep (1);
    funkeysim("Return");
    sleep (1);
    simkey("example 1");
    funkeysim("Return");
    sleep (1);
    
    echo "\nCreating a planner project file...";
    createplannerexample();
    
    funkeysim("Alt_L");
    funkeysim("Down",5);
    funkeysim("Right");
    funkeysim("Down");
    system("xte 'key Return'");
    sleep (2);
    keysim("/tmp/example.planner");
    sleep (1);
    system("xte 'key Return'");
    sleep (2);
    while ($line=fgetc($pipes[2])) ;
    
    sleep (2);
    echo "\nsending CTRL_Q...\n";
    system ("xte 'keydown Control_L'");
    system ("xte 'key Q'");
    system ("xte 'keyup Control_L'");
    
    $content=file_get_contents("/tmp/karmtest.ics");
    $lines=explode("\n",$content);
    if (!preg_match("/DTSTAMP:[0-9]{1,8}T[0-9]{1,6}Z/", $lines[4])) $err.="iCal file: wrong dtstamp";
    if ($lines[12]<>"SUMMARY:example 1") $err.="iCal file: wrong task example 1";
    if ($lines[16]<>"END:VTODO") $err.="iCal file: wrong end of vtodo";
    if ($lines[27]<>"SUMMARY:task 1") $err.="iCal file: wrong task task 1";
    if (!preg_match("/^UID:libkcal-[0-9]{1,8}.[0-9]{1,3}/", $lines[39])) $err.="iCal file: wrong uid";

  }
}
  echo $err;
  if ($err!="") exit(1);
?>