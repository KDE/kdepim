import os
import sys
import time

ICAL_HEADER = '''
BEGIN:VCALENDAR
PRODID:-//K Desktop Environment//NONSGML KArm Test Scripts//EN
VERSION:2.0
'''

TODO_TEMPLATE = '''
BEGIN:VTODO
DTSTAMP:%s
ORGANIZER;CN=Anonymous:MAILTO:nobody@nowhere
CREATED:%s
UID:%s
SEQUENCE:0
LAST-MODIFIED:%s
SUMMARY:%s
CLASS:PUBLIC
PRIORITY:5
PERCENT-COMPLETE:0
END:VTODO
'''
class KarmTestError( Exception ): pass

def dcopid():
  '''Return dcop id of first karm instance found in output of dcop command.'''

  id, n = "", 0
  while not id and n < 1000:
    stdin, stdout = os.popen2( "dcop" )
    l = stdout.readline()
    while l and not id:
      if l.startswith( "karm" ): id = l.strip()
      else: l = stdout.readline()
    stdin.close()
    stdout.close()
    n += 1

  return id

def kill_then_start_karm( filename, delete_file = True ):
  '''Kill any running karm instances, start karm, and return (dcop_id, pid) tuple.'''

  os.popen2( "killall karm" )
  time.sleep( 1 )

  _filename = os.path.expanduser( filename )

  if delete_file and os.path.exists( _filename ): os.remove( _filename )

  pid = os.spawnlp( os.P_NOWAIT, "karm", "karm", _filename )

  id = dcopid()

  if not id:
      raise KarmTestError( "No karm instance found.  Is the DISPLAY env. var set?" )
  else:
    # there is a delay b/f file is created
    f, n = None, 0
    while not f and n < 10:
      time.sleep( 1 )
      try: f = open( _filename, "r" )
      except IOError: pass
      n += 1
    if not f: raise KarmTestError( "%s was not created" % _filename )
    else: f.close()

  # strip trailing newline from dcop id
  return id, pid

def test( goal, actual ):
  '''Raise exception if goal != actual.'''
  if goal != actual:
    path, scriptname = os.path.split( sys.argv[0] )
    raise KarmTestError( "%s: expected '%s', got '%s'" % ( scriptname, goal, actual ) )

def current_time_in_ical_format():
  '''Example: 20041205T162914Z'''
  return time.strftime( "%Y%m%dT%H%M%SZ", time.gmtime() )

def new_uid():
  return "karmtest-%s" % time.time()
