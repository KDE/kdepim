// $Id$

#include "journal.h"

using namespace KCal;

Journal::Journal()
{
}

Journal::~Journal()
{
}

Journal *Journal::clone()
{
  return new Journal(*this);
}
