#ifndef JOURNAL_H
#define JOURNAL_H
// $Id$
//
// Journal component, representing a VJOURNAL object
//

#include "incidence.h"

namespace KCal {

/**
  This class provides a Journal in the sense of RFC2445.
*/
class Journal : public Incidence
{
  public:
    Journal();
    ~Journal();
};

}

#endif
