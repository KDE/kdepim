import sys
import os

def dcopid():
  '''Get dcop id of karm.  Fail if more than one instance running.'''
  id = stdin = stdout = None
  try:
    ( stdin, stdout ) = os.popen2( "dcop" )
    l = stdout.readline()
    while l:
      if l.startswith( "karm" ):
        if not id: id = l
        else: raise "Only one instance of karm may be running."
      l = stdout.readline()
    if not id:
      raise "No karm instance found.  Try running dcop at command-line to verify it works."
  except:
    if stdin: stdin.close()
    if stdout: stdout.close()
    print sys.exc_info()[0]
    sys.exit(1)
  stdin.close()
  stdout.close()

  # strip trailing newline
  return id.strip()

def test( goal, actual ):
  '''Raise exception if goal != actual.'''
  if goal != actual:
    path, scriptname = os.path.split( sys.argv[0] )
    raise "%s: expected '%s', got '%s'" % ( scriptname, goal, actual )
