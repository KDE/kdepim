import sys
import os

class KarmTestError( Exception ): pass

def dcopid():
  '''Get dcop id of karm.  Fail if more than one instance running.'''
  id = stdin = stdout = None
  ( stdin, stdout ) = os.popen2( "dcop" )
  l = stdout.readline()
  while l:
    if l.startswith( "karm" ):
      if not id: id = l
      else: raise KarmTestError( "Only one instance of karm may be running." )
    l = stdout.readline()
  if not id:
    raise KarmTestError( "No karm instance found.  Try running dcop at command-line to verify it works." )
  stdin.close()
  stdout.close()

  # strip trailing newline
  return id.strip()

def test( goal, actual ):
  '''Raise exception if goal != actual.'''
  if goal != actual:
    path, scriptname = os.path.split( sys.argv[0] )
    raise KarmTestError( "%s: expected '%s', got '%s'" % ( scriptname, goal, actual ) )
