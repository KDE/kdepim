#ifndef JOURNAL_H
#define JOURNAL_H
// $Id$
//
// Journal component, representing a VJOURNAL object
//

#include "incidence.h"

namespace KCal {

class Journal : public Incidence
{
  public:
    Journal();
    ~Journal();
};

}

#endif
