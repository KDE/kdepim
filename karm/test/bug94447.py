'''Support relative path names in command line parameter.'''
import __karmutil

import os
import StringIO
import sys
import time
import traceback

stdin = stdout = None

try:
  
  os.popen2( "cd /tmp;karm 94447.ics&" )

  dcopid = __karmutil.dcopid()
  time.sleep( 2 )

  # add task (karm does not create file until data is saved.)
  cmd = "dcop %s KarmDCOPIface addtodo %s" % ( dcopid, "'test-todo'" )
  stdin, stdout = os.popen2( cmd )
  time.sleep( 2 )

  if not os.path.exists( "/tmp/94447.ics" ):
    raise "Did not create file relative to dir where karm started."

  os.popen2( "killall karm" )

  if os.path.exists( "/tmp/94447.ics" ): os.popen2( "rm /tmp/94447.ics" )

except __karmutil.KarmTestError, e:
  if stdin: stdin.close()
  if stdout: stdout.close()
  print "%s FAIL: %s" % ( sys.argv[0], e )
  sys.exit(1)

except:
  if stdin: stdin.close()
  if stdout: stdout.close()
  # print full traceback
  tb = StringIO.StringIO()
  traceback.print_exc( None, tb )
  msg = tb.getvalue()
  tb.close()
  print "%s FAIL %s" % ( sys.argv[0], msg )
  sys.exit(1)

stdin.close()
stdout.close()
print "%s: PASS" % sys.argv[0]
sys.exit(0)
