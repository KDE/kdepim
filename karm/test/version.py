'''Check that version is correct.'''

import sys
import __karmutil
import os

goal = "1.5.0"

try:
  dcopid = __karmutil.kill_then_start_karm( "~/test.ics" )

  cmd = "dcop %s KarmDCOPIface version" % dcopid

  ( stdin, stdout ) = os.popen2( cmd )

  version = stdout.readline().strip()

  __karmutil.test( goal, version)
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

