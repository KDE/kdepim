#include "task.h"

#include "karmutility.h"

long addTaskTotalTime( long value, Task* task )
{
  if ( task )
    return value + task->totalTime();
  return value;
}

long addTaskSessionTime( long value, Task* task )
{
  if ( task )
    return value + task->sessionTime();
  return value;
}
