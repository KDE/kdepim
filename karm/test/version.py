'''Check that version is correct.'''

import sys
import __karmutil
import os

goal = "1.5.0"

dcopid = __karmutil.dcopid()
cmd = "dcop %s KarmDCOPIface version" % dcopid

stdin = stdout = None

try:
  ( stdin, stdout ) = os.popen2( cmd )
  version = stdout.readline().strip()
  __karmutil.test( goal, version)
except:
  if stdin: stdin.close()
  if stdout: stdout.close()
  print sys.exc_info()[0]
  sys.exit(1)
stdin.close()
stdout.close()
print 'ok'
sys.exit(0)

