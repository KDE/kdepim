'''Refresh when another app changes storage file.'''
import __karmutil

import os
import StringIO
import sys
import time
import traceback

# TODO: make platform independent.
testfile = "/tmp/testkarmstorage.ics"

try:
  
  # open karm
  stdin = stdout = None
  ( dcopid, pid ) = __karmutil.kill_then_start_karm( testfile )

  # add todo to ics file with python
  todo = "test1"
  uid = __karmutil.addtodo( testfile, todo )

  # wait a bit to make sure karm gets time to reload file.
  time.sleep( 2 )

  # make sure karm has new todo
  cmd = "dcop %s KarmDCOPIface hastodo %s" % ( dcopid, todo )
  ( stdin, stdout ) = os.popen2( cmd )
  result = stdout.readline().strip()
  __karmutil.test( uid, result)

  # kill karm
  os.popen2( "killall karm" )

except __karmutil.KarmTestError, e:
  if stdin: stdin.close()
  if stdout: stdout.close()
  print "%s: %s" % ( sys.argv[0], e )
  sys.exit(1)
except:
  if stdin: stdin.close()
  if stdout: stdout.close()
  # print full traceback
  tb = StringIO.StringIO()
  traceback.print_exc( None, tb )
  msg = tb.getvalue()
  tb.close()
  print "%s:\n%s" % ( sys.argv[0], msg )
  sys.exit(1)

stdin.close()
stdout.close()
sys.exit(0)
