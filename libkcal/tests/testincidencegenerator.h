#ifndef TESTINCIDENCEGENERATOR
#define TESTINCIDENCEGENERATOR

#include "event.h"
#include "todo.h"
#include "journal.h"
using namespace KCal;

static Event* makeTestEvent() 
{
  Event *event = new Event();
  event->setSummary("Test Event");
  event->recurrence()->setDaily( 2, 3 );
  return event;
}

static Todo* makeTestTodo() 
{
  Todo *todo = new Todo();
  todo->setSummary("Test Todo");
  todo->setPriority( 5 );
  return todo;
}

static Journal* makeTestJournal() 
{
  Journal *journal = new Journal();
  journal->setSummary("Test Journal");
  return journal;
}


#endif